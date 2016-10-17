
// ================================================================================================
// -*- C++ -*-
// File: ntb_widgets.hpp
// Author: Guilherme R. Lampert
// Created on: 25/04/16
// Brief: Widgets are the back-end UI elements/components of NTB.
// ================================================================================================

#ifndef NTB_WIDGETS_HPP
#define NTB_WIDGETS_HPP

#include "ntb_utils.hpp"

#if NEO_TWEAK_BAR_DEBUG
    #include <iostream>
#endif // NEO_TWEAK_BAR_DEBUG

namespace ntb
{

// ========================================================
// Helper structures:
// ========================================================

enum class TextAlign
{
    Left,
    Right,
    Center
};

// ========================================================
// class GeometryBatch:
// ========================================================

class GeometryBatch final
{
public:

     GeometryBatch();
    ~GeometryBatch();

    // Not copyable.
    GeometryBatch(const GeometryBatch &) = delete;
    GeometryBatch & operator = (const GeometryBatch &) = delete;

    // Preallocate memory for the draw batches. This is optional since the batches
    // will resize as needed. You can hint the number of lines, quadrilaterals, text
    // glyphs/chars and clipped draw calls needed in each frame.
    void preallocateBatches(int lines, int quads, int textGlyphs, int drawClipped);

    // Draw batch setup. Forwards to RenderInterface::beginDraw/endDraw.
    void beginDraw();
    void endDraw();

    // Filled triangles with clipping (used by the 3D widgets).
    void drawClipped2DTriangles(const VertexPTC * verts, int vertCount,
                                const UInt16 * indexes, int indexCount,
                                const Rectangle & viewport, const Rectangle & clipBox);

    // Filled indexed triangles, without texture:
    void draw2DTriangles(const VertexPTC * verts, int vertCount, const UInt16 * indexes, int indexCount);

    // Lines:
    void drawLine(int xFrom, int yFrom, int xTo, int yTo, Color32 color);
    void drawLine(int xFrom, int yFrom, int xTo, int yTo, Color32 colorFrom, Color32 colorTo);

    // Filled rectangle (possibly translucent):
    void drawRectFilled(const Rectangle & rect, Color32 color);
    void drawRectFilled(const Rectangle & rect, Color32 c0, Color32 c1, Color32 c2, Color32 c3);

    // Rectangle outlines:
    void drawRectOutline(const Rectangle & rect, Color32 color);
    void drawRectOutline(const Rectangle & rect, Color32 c0, Color32 c1, Color32 c2, Color32 c3);

    // Simulates a drop-shadow for the given rectangle by drawing
    // alpha-blended borders to the right and bottom.
    void drawRectShadow(const Rectangle & rect, Color32 shadowColor, Color32 penumbraColor, int shadowOffset);

    // Arrowhead direction: 1=up, -1=down.
    void drawArrowFilled(const Rectangle & rect, Color32 bgColor, Color32 outlineColor, int direction);

    // Handles newlines, spaces and tabs, etc.
    // For efficiency reasons, clipping is done per character, so partly occluded chars will not draw.
    void drawTextConstrained(const char * text, int textLength, Rectangle alignBox, const Rectangle & clipBox,
                             Float32 scaling, Color32 color, TextAlign align);

    // Width in pixels of a text string using the given font. Doesn't actually draw anything.
    static Float32 calcTextWidth(const char * text, int textLength, Float32 scaling);

    // Width/height in pixels of a single character/glyph of the font in use.
    // Note: We always assume a fixed width and height font.
    static Float32 getCharWidth();
    static Float32 getCharHeight();

    // Next Z layer for the frame. Z layers are reset at beginDraw() and incremented
    // every time this method gets called, so no two draw call will have overlapping Z.
    Float32 getNextZ() { return static_cast<Float32>(currentZ++); }

private:

    // Calls in the RenderInterface to allocate the glyph bitmap.
    void createGlyphTexture();

    // Handles newlines, spaces and tabs. String doesn't have to be NUL-terminated, we rely on textLength instead.
    void drawTextImpl(const char * text, int textLength, Float32 x, Float32 y, Float32 scaling, Color32 color);

    // The glyph bitmap decompressed and copied into a RenderInterface texture object.
    TextureHandle glyphTex;

    // Z layer/index for all 2D elements. Starts at 0 in beginDraw(),
    // incremented for each line/triangle that is added to the batch.
    int currentZ;

    // Current offsets for the 2D/text index buffers.
    UInt16 baseVertex2D;
    UInt16 baseVertexText;
    UInt16 baseVertexClipped;

    // Batch for 2D colored lines.
    PODArray linesBatch;        // [VertexPC]

    // Batch for all untextured 2D triangles (indexed).
    PODArray verts2DBatch;      // [VertexPTC] Miscellaneous 2D elements.
    PODArray tris2DBatch;       // [UInt16]    Triangle indexes for the 2D elements.

    // Batch for all 2D text glyphs (indexed).
    PODArray textVertsBatch;    // [VertexPTC] Vertexes for 2D text glyphs.
    PODArray textTrisBatch;     // [UInt16]    Indexes for the 2D text triangles.

    // Separate batch for the clipped 2D vertexes
    // (normally sent from the 3D widgets).
    PODArray drawClippedInfos;  // [DrawClippedInfo]
    PODArray vertsClippedBatch; // [VertexPTC]
    PODArray trisClippedBatch;  // [UInt16]
};

// ========================================================
// class ValueSlider:
// ========================================================

// Small helper class that draws and manages a horizontal slider bar.
class ValueSlider final
{
public:

    ValueSlider();
    void reset();

    void setRange(Float64 min, Float64 max);
    void setCurrentValue(Float64 v);

    Float64 getCurrentValue() const;
    Rectangle getSliderRect() const;

    // Also updates the current slider rectangle based on the value.
    void drawSelf(GeometryBatch & geoBatch, const Rectangle & displayBox, Color32 borderColor, Color32 fillColor);

private:

    Float64   minVal;
    Float64   maxVal;
    Float64   currentVal;
    Rectangle sliderRect;
};

// ========================================================
// class Widget:
// ========================================================

// Basically an interactive screen/UI element.
// A panel has a widget, but so does a button
// or a tweakable variable. Widgets are drawable
// and can also respond to user input events.
class Widget
{
public:

    enum Flags
    {
        FlagVisible             = 1 << 0,
        FlagMinimized           = 1 << 1,
        FlagScrolledOutOfView   = 1 << 2,
        FlagMouseIntersecting   = 1 << 3,
        FlagMouseDragEnabled    = 1 << 4,
        FlagNoRectShadow        = 1 << 5,
        FlagNeedDeleting        = 1 << 6,
        FlagInvertMouseScroll   = 1 << 7,
        FlagHoldingScrollSlider = 1 << 8
    };

    enum Corner
    {
        TopLeft,
        BottomLeft,
        TopRight,
        BottomRight,

        CornerCount,
        CornerNone = CornerCount
    };

    // Not copyable.
    Widget(const Widget &) = delete;
    Widget & operator = (const Widget &) = delete;

    // Init/shutdown:
    Widget();
    virtual ~Widget();
    void init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible);

    // Input events:
    virtual bool onKeyPressed(KeyCode key, KeyModFlags modifiers);
    virtual bool onMouseButton(MouseButton button, int clicks);
    virtual bool onMouseMotion(int mx, int my);
    virtual bool onMouseScroll(int yScroll);
    virtual void onScrollContentUp();
    virtual void onScrollContentDown();

    // Layout/drawing events:
    virtual void onDraw(GeometryBatch & geoBatch) const;
    virtual void onResize(int displacementX, int displacementY, Corner corner);
    virtual void onMove(int displacementX, int displacementY);
    virtual void onAdjustLayout();
    virtual void onDisableEditing();

    // Miscellaneous gets/sets:
    bool isVisible() const;
    bool isMinimized() const;
    bool isScrolledOutOfView() const;
    bool isMouseIntersecting() const;
    bool isMouseDragEnabled() const;

    virtual void setVisible(bool visible);
    virtual void setMouseIntersecting(bool intersect);
    void setMinimized(bool minimized);
    void setScrolledOutOfView(bool outOfView);
    void setMouseDragEnabled(bool enabled);
    void setGUI(GUI * newGUI);
    void setParent(Widget * newParent);
    void setColors(const ColorScheme * newColors);
    void setRect(const Rectangle & newRect);
    void setNormalColors();
    void setHighlightedColors();

    const ColorScheme & getColors() const;
    const Rectangle & getRect() const;

    const GUI * getGUI() const;
    GUI * getGUI();

    const Widget * getParent() const;
    Widget * getParent();

    // Parenting/hierarchy:
    const Widget * getChild(int index) const;
    Widget * getChild(int index);
    bool isChild(const Widget * widget) const;
    void addChild(Widget * newChild);
    int getChildCount() const;

    // UI/text scaling:
    void setScaling(Float32 s);
    void setTextScaling(Float32 s);
    Float32 getScaling() const;
    Float32 getTextScaling() const;
    int uiScaled(int val) const;
    static int uiScaleBy(Float64 val, Float64 scale);

    // State flags:
    bool testFlag(UInt32 mask) const;
    void setFlag(UInt32 mask, int f);

    // Debug printing helpers:
    #if NEO_TWEAK_BAR_DEBUG
    virtual SmallStr getTypeString() const;
    virtual void printHierarchy(std::ostream & out = std::cout, const SmallStr & indent = "") const;
    #endif // NEO_TWEAK_BAR_DEBUG

protected:

    void drawSelf(GeometryBatch & geoBatch) const;
    void drawChildren(GeometryBatch & geoBatch) const;

    GUI               * gui;          // Direct pointer to the owning UI for things like color-scheme and scaling; never null.
    Widget            * parent;       // Direct parent in the hierarchy (Panel/Window); may be null.
    const ColorScheme * colors;       // A pointer so we can hot swap it, but never null.
    PODArray            children;     // List of pointers to the child widgets, if any.
    Float32             scaling;      // Scaling factor applied to the UI geometry.
    Float32             textScaling;  // Scaling applied to the text only.
    UInt32              flags;        // Miscellaneous state flags (from the Flags enum).
    Rectangle           rect;         // Drawable rectangle.
    Point               lastMousePos; // Saved from last time onMouseMotion() was called.
};

// ========================================================
// class ButtonWidget:
// ========================================================

class ButtonWidget final
    : public Widget
{
public:

    enum class Icon : UInt8
    {
        None,          // No button. Nothing drawn, no events.
        Plus,          // Plus sign [+].
        Minus,         // Minus sign/dash [-].
        LeftArrow,     // Arrowhead pointing left [<].
        RightArrow,    // Arrowhead pointing right [>].
        DblLeftArrow,  // Double arrow pointing left [«].
        DblRightArrow, // Double arrow pointing right [»].
        QuestionMark,  // A question mark [?].
        CheckMark,     // A on|off check mark switch.

        // Number of entries in this enum. Internal use.
        Count
    };

    class EventListener
    {
    public:
        // Callback fired when the button is left-clicked. Should return true if the
        // event was handle. Default implementation is a no-op that always returns false.
        virtual bool onButtonDown(ButtonWidget & button);

    protected:
        // Protected and non-virtual, since this class is meant for use as a mixin.
        ~EventListener();
    };

    ButtonWidget();
    void init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible,
              Icon myIcon, EventListener * myListener = nullptr);

    void onDraw(GeometryBatch & geoBatch) const override;
    bool onMouseButton(MouseButton button, int clicks) override;

    bool getState() const;
    void setState(bool newState);
    bool isCheckBoxButton() const;

    Icon getIcon() const;
    void setIcon(Icon newIcon);

    bool hasEventListener() const;
    EventListener * getEventListener() const;
    void setEventListener(EventListener * newListener);

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const override;
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    EventListener * eventListener; // Not owned by the button.
    Icon            icon;          // Button type and visuals.
    bool            state;         // Flipped at each click event. Starts as false.
};

// ========================================================
// class TitleBarWidget:
// ========================================================

class TitleBarWidget final
    : public Widget, public ButtonWidget::EventListener
{
public:

    TitleBarWidget() = default;
    void init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible,
              const char * myTitle, bool minimizeButton, bool maximizeButton,
              int buttonOffsX, int buttonOffsY, int buttonSize, int buttonSpacing);

    void onDraw(GeometryBatch & geoBatch) const override;
    bool onMouseButton(MouseButton button, int clicks) override;
    void onResize(int displacementX, int displacementY, Corner corner) override;
    bool onButtonDown(ButtonWidget & button) override;

    void setButtonTextScaling(Float32 s);
    void setTitle(const char * newTitle);
    const char * getTitle() const;
    int getBarHeight() const;

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const override;
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    void buttonSetup(bool minimizeButton, bool maximizeButton,
                     int buttonOffsX, int buttonOffsY,
                     int buttonSize,  int buttonSpacing);

    enum ButtonIds
    {
        BtnMinimize,
        BtnMaximize,
        BtnCount
    };

    ButtonWidget buttons[BtnCount];
    SmallStr     titleText;
    int          initialHeight;
};

// ========================================================
// class InfoBarWidget:
// ========================================================

// Bar at the base of a window/panel that displays things like keys pressed.
class InfoBarWidget final
    : public Widget
{
public:

    InfoBarWidget() = default;
    void init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible, const char * myText);

    void onDraw(GeometryBatch & geoBatch) const override;
    void onResize(int displacementX, int displacementY, Corner corner) override;

    void setText(const char * newText);
    const char * getText() const;
    int getBarHeight() const;

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const override;
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    SmallStr infoText;
    int      initialHeight;
};

// ========================================================
// class ScrollBarWidget:
// ========================================================

class ScrollBarWidget final
    : public Widget
{
public:

    ScrollBarWidget();
    void init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible, int buttonSize);

    void onDraw(GeometryBatch & geoBatch) const override;
    void onMove(int displacementX, int displacementY) override;
    void onResize(int displacementX, int displacementY, Corner corner) override;
    void onAdjustLayout() override;

    bool onKeyPressed(KeyCode key, KeyModFlags modifiers) override;
    bool onMouseButton(MouseButton button, int clicks) override;
    bool onMouseMotion(int mx, int my) override;
    bool onMouseScroll(int yScroll) override;

    void updateLineScrollState(int lineCount, int linesOut);
    void setInvertMouseScroll(bool invert);
    bool isMouseScrollInverted() const;
    int getBarWidth() const;

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const override;
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    void doScrollUp();
    void doScrollDown();

    Rectangle makeInnerBarRect() const;
    Rectangle makeUpButtonRect() const;
    Rectangle makeDownButtonRect() const;

    // Scroll slider states:
    int scrollBarOffsetY;      // Current Y start of slider bar.
    int scrollBarDisplacement; // Amount to move each click.
    int scrollBarSizeFactor;   // Slider box scale: [0,100].
    int scrollBarThickness;    // Thickness of slider bar. A fraction of the bar's box.
    int scrollBarButtonSize;
    int initialWidth;

    // Start/end of scroll area (visualized as the line under the slider):
    int scrollStartY;
    int scrollEndY;

    // Button boxes and scroll slider box:
    Rectangle upBtnRect;
    Rectangle downBtnRect;
    Rectangle barSliderRect;

    // Misc mouse and slider helpers:
    Point sliderClickInitialPos;
    int accumulatedScrollSliderDrag;
    int totalLines;
    int linesOutOfView;
    int linesScrolledOut;
};

// ========================================================
// class ListWidget:
// ========================================================

// Vertical list of clickable items. Widget size is adjusted to the longest item text.
class ListWidget final
    : public Widget
{
public:

    ListWidget();
    void init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible);

    void onDraw(GeometryBatch & geoBatch) const override;
    void onMove(int displacementX, int displacementY) override;
    bool onMouseButton(MouseButton button, int clicks) override;
    bool onMouseMotion(int mx, int my) override;

    void allocEntries(int count);
    int getNumOfEntries() const;

    void addEntryText(int index, const char * value);
    SmallStr getEntryText(int index) const;

    int  getSelectedEntry() const;
    bool hasSelectedEntry() const;
    void clearSelectedEntry();

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const override;
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    void addEntryRect(int entryIndex, int entryLengthInChars);
    int findEntryForPoint(int x, int y) const;

    struct Entry
    {
        // Clickable rectangle
        Rectangle rect;

        // Offsets into 'strings'
        int firstChar;
        int lengthInChars;
    };
    PODArray entries; // [Entry]

    // User selection in the list. No selection if < 0.
    static constexpr int None = -1;
    int selectedEntry;
    int hoveredEntry;

    // All strings in the list entries/buttons are
    // packed into this string, no spacing in between.
    SmallStr strings;
};

// ========================================================
// class ColorPickerWidget:
// ========================================================

class ColorPickerWidget final
    : public Widget, public ButtonWidget::EventListener
{
public:

    ColorPickerWidget();
    void init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible,
              int titleBarHeight, int titleBarButtonSize, int scrollBarWidth,
              int scrollBarButtonSize, int clrButtonSize);

    void onDraw(GeometryBatch & geoBatch) const override;
    void onMove(int displacementX, int displacementY) override;

    bool onButtonDown(ButtonWidget & button) override;
    bool onMouseButton(MouseButton button, int clicks) override;
    bool onMouseMotion(int mx, int my) override;
    bool onMouseScroll(int yScroll) override;

    void onScrollContentUp() override;
    void onScrollContentDown() override;
    void setMouseIntersecting(bool intersect) override;
    void setButtonTextScaling(Float32 s);

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const override;
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    using ButtonFunc = bool (ColorPickerWidget::*)(Rectangle, int, GeometryBatch *) const;

    void refreshUsableRect();
    bool drawColorButton(Rectangle colorRect, int colorIndex, GeometryBatch * pGeoBatch) const;
    bool testColorButtonClick(Rectangle colorRect, int colorIndex, GeometryBatch * pUnused) const;
    bool forEachColorButton(ButtonFunc pFunc, GeometryBatch * pGeoBatch) const;

    // Discounts the top/side bars
    Rectangle usableRect;

    static constexpr int None = -1;
    mutable int selectedColorIndex; // Index into the color table. 'None' (-1) for no selection.
    int colorButtonLinesScrolledUp; // # of color buttons scrolled out.
    int colorButtonSize;            // Width & height of each color button.

    TitleBarWidget  titleBar;
    ScrollBarWidget scrollBar;
};

// ========================================================
// class View3DWidget:
// ========================================================

class View3DWidget final
    : public Widget
{
public:

    struct ProjectionParameters
    {
        Rectangle viewport;
        Float32   fovYRadians;
        Float32   aspectRatio;
        Float32   zNear;
        Float32   zFar;
        Mat4x4    viewProjMatrix;
        bool      autoAdjustAspect;
    };

    enum class Object : UInt8
    {
        None,
        Sphere,
        Arrow,
        Box,

        // Number of entries in this enum. Internal use.
        Count
    };

    View3DWidget();
    void init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible,
              const char * myTitle, int titleBarHeight, int titleBarButtonSize,
              int resetAnglesBtnSize, const ProjectionParameters & proj, Object obj);

    void onDraw(GeometryBatch & geoBatch) const override;
    void onMove(int displacementX, int displacementY) override;

    bool onMouseButton(MouseButton button, int clicks) override;
    bool onMouseMotion(int mx, int my) override;
    bool onMouseScroll(int yScroll) override;
    void setMouseIntersecting(bool intersect) override;
    void setButtonTextScaling(Float32 s);

    void setInvertMouseY(bool invert);
    bool isMouseYInverted() const;

    void setMouseSensitivity(Float32 sensitivity);
    Float32 getMouseSensitivity() const;

    void setMaxMouseDelta(int max);
    int getMaxMouseDelta() const;

    void setShowXyzLabels(bool show);
    bool isShowingXyzLabels() const;

    void setInteractive(bool interactive);
    bool isInteractive() const;

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const override;
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    enum ArrowDir
    {
        ArrowDirX,
        ArrowDirY,
        ArrowDirZ
    };

    void clearScreenVertexCaches() const;
    void submitScreenVertexCaches(GeometryBatch & geoBatch) const;
    void addScreenProjectedSphere(const Mat4x4 & modelToWorldMatrix, Float32 scaleXYZ) const;
    void addScreenProjectedArrow(const Mat4x4 & modelToWorldMatrix, Float32 scaleXYZ, Color32 color, ArrowDir dir) const;
    void addScreenProjectedBox(const Mat4x4 & modelToWorldMatrix, Float32 w, Float32 h, Float32 d, Color32 color) const;
    void refreshProjectionViewport();

    // Local mouse states:
    Point mouseDelta;                     // XY deltas to compute the mouse rotations.
    Float32 mouseSensitivity;             // [0,1] range: 0=very low; 1=very high.
    int maxMouseDelta;                    // Any range; default is 20.
    bool invertMouseY;                    // Inverts mouse rotation of the object in the Y-axis.
    bool leftMouseButtonDown;

    // Misc switches:
    bool interactiveControls;             // Allow mouse input and the reset button. Defaults to true.
    bool showXyzLabels;                   // Show the XYZ label at the right corner. Defaults to true.
    Object object;                        // Object geometry that is drawn in the viewport.

    // Rotation angles:
    mutable bool updateScrGeometry;       // Only update the geometry caches when needed (on input/angles changed).
    mutable bool resettingAngles;         // True when "R" clicked. New mouse input cancels it.
    mutable Vec3 rotationDegrees;         // AKA pitch (X), yaw (Y) and roll (Z).
    mutable Int64 prevFrameTimeMs;        // Needed to compute a delta-time for angle reset lerp.
    Rectangle resetAnglesBtnRect;         // Tiny reset button in the corner ("R").

    // Screen projected geometry caches:
    mutable PODArray scrProjectedVerts;   // Cached 3D object vertexes projected to screen, ready for drawing.
    mutable PODArray scrProjectedIndexes; // Index buffer for the above verts, since RenderInterface requires it.
    ProjectionParameters projParams;      // Cached projection/viewport settings.

    // The (optional) title bar:
    TitleBarWidget titleBar;
};

// ========================================================
// Inline methods for the ValueSlider class:
// ========================================================

inline ValueSlider::ValueSlider()
{
    reset();
}

inline void ValueSlider::reset()
{
    minVal     = 0.0;
    maxVal     = 1.0;
    currentVal = 0.0;
    sliderRect.setZero();
}

inline void ValueSlider::setRange(Float64 min, Float64 max)
{
    minVal = min;
    maxVal = max;
}

inline void ValueSlider::setCurrentValue(Float64 v)
{
    currentVal = clamp(v, minVal, maxVal);
}

inline Float64 ValueSlider::getCurrentValue() const
{
    return currentVal;
}

inline Rectangle ValueSlider::getSliderRect() const
{
    return sliderRect;
}

// ========================================================
// Inline methods for the Widget class:
// ========================================================

inline bool Widget::isVisible() const
{
    return testFlag(FlagVisible);
}

inline bool Widget::isMinimized() const
{
    return testFlag(FlagMinimized);
}

inline bool Widget::isScrolledOutOfView() const
{
    return testFlag(FlagScrolledOutOfView);
}

inline bool Widget::isMouseIntersecting() const
{
    return testFlag(FlagMouseIntersecting);
}

inline bool Widget::isMouseDragEnabled() const
{
    return testFlag(FlagMouseDragEnabled);
}

inline void Widget::setVisible(bool visible)
{
    setFlag(FlagVisible, visible);
}

inline void Widget::setMouseIntersecting(bool intersect)
{
    setFlag(FlagMouseIntersecting, intersect);
}

inline void Widget::setMinimized(bool minimized)
{
    setFlag(FlagMinimized, minimized);
}

inline void Widget::setScrolledOutOfView(bool outOfView)
{
    setFlag(FlagScrolledOutOfView, outOfView);
}

inline void Widget::setGUI(GUI * newGUI)
{
    NTB_ASSERT(newGUI != nullptr);
    gui = newGUI;
}

inline void Widget::setParent(Widget * newParent)
{
    parent = newParent;
}

inline void Widget::setColors(const ColorScheme * newColors)
{
    NTB_ASSERT(newColors != nullptr);
    colors = newColors;
}

inline void Widget::setRect(const Rectangle & newRect)
{
    rect = newRect;
}

inline const ColorScheme & Widget::getColors() const
{
    NTB_ASSERT(colors != nullptr);
    return *colors;
}

inline const Rectangle & Widget::getRect() const
{
    return rect;
}

inline const GUI * Widget::getGUI() const
{
    NTB_ASSERT(gui != nullptr);
    return gui;
}

inline GUI * Widget::getGUI()
{
    NTB_ASSERT(gui != nullptr);
    return gui;
}

inline const Widget * Widget::getParent() const
{
    return parent;
}

inline Widget * Widget::getParent()
{
    return parent;
}

inline void Widget::addChild(Widget * newChild)
{
    NTB_ASSERT(newChild != nullptr);
    children.pushBack<Widget *>(newChild);
}

inline const Widget * Widget::getChild(int index) const
{
    return children.get<const Widget *>(index);
}

inline Widget * Widget::getChild(int index)
{
    return children.get<Widget *>(index);
}

inline int Widget::getChildCount() const
{
    return children.getSize();
}

inline void Widget::setScaling(Float32 s)
{
    scaling = s;
    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        getChild(c)->setScaling(s);
    }
}

inline void Widget::setTextScaling(Float32 s)
{
    textScaling = s;
    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        getChild(c)->setTextScaling(s);
    }
}

inline Float32 Widget::getScaling() const
{
    return scaling;
}

inline Float32 Widget::getTextScaling() const
{
    return textScaling;
}

inline int Widget::uiScaled(int val) const
{
    return uiScaleBy(val, scaling);
}

inline int Widget::uiScaleBy(Float64 val, Float64 scale)
{
    return static_cast<int>(val * scale);
}

inline bool Widget::testFlag(UInt32 mask) const
{
    return !!(flags & mask);
}

inline void Widget::setFlag(UInt32 mask, int f)
{
    // Using one of the Standford bit-hacks:
    // http://graphics.stanford.edu/~seander/bithacks.html
    flags = (flags & ~mask) | (-f & mask);
}

#if NEO_TWEAK_BAR_DEBUG
inline SmallStr Widget::getTypeString() const
{
    return "Widget";
}
#endif // NEO_TWEAK_BAR_DEBUG

// ========================================================
// Inline methods for the ButtonWidget class:
// ========================================================

inline bool ButtonWidget::getState() const
{
    return state;
}

inline void ButtonWidget::setState(bool newState)
{
    state = newState;
}

inline bool ButtonWidget::isCheckBoxButton() const
{
    return icon == Icon::CheckMark;
}

inline ButtonWidget::Icon ButtonWidget::getIcon() const
{
    return icon;
}

inline void ButtonWidget::setIcon(Icon newIcon)
{
    icon = newIcon;
}

inline bool ButtonWidget::hasEventListener() const
{
    return eventListener != nullptr;
}

inline ButtonWidget::EventListener * ButtonWidget::getEventListener() const
{
    return eventListener;
}

inline void ButtonWidget::setEventListener(EventListener * newListener)
{
    eventListener = newListener;
}

#if NEO_TWEAK_BAR_DEBUG
inline SmallStr ButtonWidget::getTypeString() const
{
    return "ButtonWidget";
}
#endif // NEO_TWEAK_BAR_DEBUG

// ========================================================
// Inline methods for the TitleBarWidget class:
// ========================================================

inline void TitleBarWidget::setTitle(const char * newTitle)
{
    titleText = newTitle;
}

inline const char * TitleBarWidget::getTitle() const
{
    return titleText.c_str();
}

inline int TitleBarWidget::getBarHeight() const
{
    return initialHeight;
}

#if NEO_TWEAK_BAR_DEBUG
inline SmallStr TitleBarWidget::getTypeString() const
{
    return "TitleBarWidget";
}
#endif // NEO_TWEAK_BAR_DEBUG

// ========================================================
// Inline methods for the InfoBarWidget class:
// ========================================================

inline void InfoBarWidget::setText(const char * newText)
{
    infoText  = " » ";
    infoText += newText;
}

inline const char * InfoBarWidget::getText() const
{
    return infoText.c_str();
}

inline int InfoBarWidget::getBarHeight() const
{
    return initialHeight;
}

#if NEO_TWEAK_BAR_DEBUG
inline SmallStr InfoBarWidget::getTypeString() const
{
    return "InfoBarWidget";
}
#endif // NEO_TWEAK_BAR_DEBUG

// ========================================================
// Inline methods for the ScrollBarWidget class:
// ========================================================

inline void ScrollBarWidget::setInvertMouseScroll(bool invert)
{
    setFlag(FlagInvertMouseScroll, invert);
}

inline bool ScrollBarWidget::isMouseScrollInverted() const
{
    return testFlag(FlagInvertMouseScroll);
}

inline int ScrollBarWidget::getBarWidth() const
{
    return initialWidth;
}

#if NEO_TWEAK_BAR_DEBUG
inline SmallStr ScrollBarWidget::getTypeString() const
{
    return "ScrollBarWidget";
}
#endif // NEO_TWEAK_BAR_DEBUG

// ========================================================
// Inline methods for the ListWidget class:
// ========================================================

inline int ListWidget::getNumOfEntries() const
{
    return entries.getSize();
}

inline SmallStr ListWidget::getEntryText(int index) const
{
    const Entry & entry = entries.get<Entry>(index);
    return { strings.c_str() + entry.firstChar, entry.lengthInChars };
}

inline int ListWidget::getSelectedEntry() const
{
    return selectedEntry;
}

inline bool ListWidget::hasSelectedEntry() const
{
    return selectedEntry != None;
}

inline void ListWidget::clearSelectedEntry()
{
    selectedEntry = None;
}

#if NEO_TWEAK_BAR_DEBUG
inline SmallStr ListWidget::getTypeString() const
{
    return "ListWidget";
}
#endif // NEO_TWEAK_BAR_DEBUG

// ========================================================
// Inline methods for the ColorPickerWidget class:
// ========================================================

inline void ColorPickerWidget::setMouseIntersecting(bool intersect)
{
    Widget::setMouseIntersecting(intersect);

    // We want to highlight the side/top bars when the parent gains focus.
    if (intersect)
    {
        titleBar.setHighlightedColors();
        scrollBar.setHighlightedColors();
    }
}

inline void ColorPickerWidget::setButtonTextScaling(Float32 s)
{
    titleBar.setButtonTextScaling(s);
}

#if NEO_TWEAK_BAR_DEBUG
inline SmallStr ColorPickerWidget::getTypeString() const
{
    return "ColorPickerWidget";
}
#endif // NEO_TWEAK_BAR_DEBUG

// ========================================================
// Inline methods for the View3DWidget class:
// ========================================================

inline void View3DWidget::setButtonTextScaling(Float32 s)
{
    titleBar.setButtonTextScaling(s);
}

inline void View3DWidget::setInvertMouseY(bool invert)
{
    invertMouseY = invert;
}

inline bool View3DWidget::isMouseYInverted() const
{
    return invertMouseY;
}

inline void View3DWidget::setMouseSensitivity(Float32 sensitivity)
{
    mouseSensitivity = sensitivity;
}

inline Float32 View3DWidget::getMouseSensitivity() const
{
    return mouseSensitivity;
}

inline void View3DWidget::setMaxMouseDelta(int max)
{
    maxMouseDelta = max;
}

inline int View3DWidget::getMaxMouseDelta() const
{
    return maxMouseDelta;
}

inline void View3DWidget::setShowXyzLabels(bool show)
{
    showXyzLabels = show;
}

inline bool View3DWidget::isShowingXyzLabels() const
{
    return showXyzLabels;
}

inline void View3DWidget::setInteractive(bool interactive)
{
    interactiveControls = interactive;
}

inline bool View3DWidget::isInteractive() const
{
    return interactiveControls;
}

#if NEO_TWEAK_BAR_DEBUG
inline SmallStr View3DWidget::getTypeString() const
{
    return "View3DWidget";
}
#endif // NEO_TWEAK_BAR_DEBUG

} // namespace ntb {}

#endif // NTB_WIDGETS_HPP
