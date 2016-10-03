
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
        FlagVisible           = 1 << 0,
        FlagMinimized         = 1 << 1,
        FlagScrolledOutOfView = 1 << 2,
        FlagMouseIntersecting = 1 << 3,
        FlagMouseDragEnabled  = 1 << 4,
        FlagNoRectShadow      = 1 << 5,
        FlagNeedDeleting      = 1 << 6
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
    void init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible = true);

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
    void setTextScaling(Float32 s);
    Float32 getTextScaling() const;
    void setScaling(Float32 s);
    Float32 getScaling() const;
    int uiScaled(int val) const;
    static int uiScaleBy(Float64 val, Float64 scale);

    // State flags:
    bool testFlag(UInt32 mask) const;
    void setFlag(UInt32 mask, int f);

    // Debug printing helpers:
    #if NEO_TWEAK_BAR_DEBUG
    virtual void printHierarchy(std::ostream & out = std::cout, const SmallStr & indent = "") const;
    virtual SmallStr getTypeString() const { return "Widget"; }
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

    enum class Icon
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
    void init(GUI * myGUI, Widget * myParent, const Rectangle & myRect,
              bool visible, Icon myIcon, EventListener * myListener = nullptr);

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
    SmallStr getTypeString() const override { return "ButtonWidget"; }
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    EventListener * eventListener; // Not owned by the button.
    Icon icon;                     // Button type and visuals.
    bool state;                    // Flipped at each click event. Starts as false.
};

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

inline void Widget::setTextScaling(Float32 s)
{
    textScaling = s;
}

inline Float32 Widget::getTextScaling() const
{
    return textScaling;
}

inline void Widget::setScaling(Float32 s)
{
    scaling = s;
}

inline Float32 Widget::getScaling() const
{
    return scaling;
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

} // namespace ntb {}

#endif // NTB_WIDGETS_HPP
