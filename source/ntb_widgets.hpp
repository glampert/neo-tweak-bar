#pragma once
// ================================================================================================
// -*- C++ -*-
// File: ntb_widgets.hpp
// Author: Guilherme R. Lampert
// Created on: 25/04/16
// Brief: Widgets are the back-end UI elements/components of NTB.
// ================================================================================================

#include "ntb_utils.hpp"

#if NEO_TWEAK_BAR_DEBUG
    #include <iostream>
#endif // NEO_TWEAK_BAR_DEBUG

namespace ntb
{

class Widget;
class WindowWidget;

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
                                const std::uint16_t * indexes, int indexCount,
                                const Rectangle & viewport, const Rectangle & clipBox);

    // Filled indexed triangles, without texture:
    void draw2DTriangles(const VertexPTC * verts, int vertCount, const std::uint16_t * indexes, int indexCount);

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
    std::uint16_t baseVertex2D;
    std::uint16_t baseVertexText;
    std::uint16_t baseVertexClipped;

    // Batch for 2D colored lines.
    PODArray linesBatch;        // [VertexPC]

    // Batch for all untextured 2D triangles (indexed).
    PODArray verts2DBatch;      // [VertexPTC] Miscellaneous 2D elements.
    PODArray tris2DBatch;       // [std::uint16_t] Triangle indexes for the 2D elements.

    // Batch for all 2D text glyphs (indexed).
    PODArray textVertsBatch;    // [VertexPTC] Vertexes for 2D text glyphs.
    PODArray textTrisBatch;     // [std::uint16_t] Indexes for the 2D text triangles.

    // Separate batch for the clipped 2D vertexes
    // (normally sent from the 3D widgets).
    PODArray drawClippedInfos;  // [DrawClippedInfo]
    PODArray vertsClippedBatch; // [VertexPTC]
    PODArray trisClippedBatch;  // [std::uint16_t]
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

    // Also updates the current slider rectangle based on the value.
    void drawSelf(GeometryBatch & geoBatch, const Rectangle & displayBox, Color32 borderColor, Color32 fillColor, Float32 uiScaling);

private:

    Float64 minVal;
    Float64 maxVal;
    Float64 currentVal;
};

// ========================================================
// class EditField:
// ========================================================

class EditField final
    : public ListNode<EditField>
{
public:

    // EditField stores some state related to editing a string of
    // text with a cursor/caret via keyboard or mouse input. It also
    // stores additional state about a selected range inside the text.
    //
    // EditFields are capable of drawing the text string, selected
    // region and the cursor/caret, but it does not keep a pointer
    // to the text. Instead, the text and its containing rectangle
    // must always be passed as parameters.
    //
    // The total length of the text string is saved by drawSelf(),
    // so they should always be kept in sync. When a char is removed
    // by a key command the cursor position and text length are
    // updated so you should update the string to match.
    //
    // handleSpecialKey() will reposition the cursor accordingly
    // and will return an EditCommand for keys that are not handled
    // directly here, like TAB and PAGE-UP/DOWN. ENTER/RETURN and
    // ESCAPE will return the DoneEditing command.
    //
    // Each VarDisplayWidget has an EditField and they are also added to
    // a linked list in the var parent WindowWidget for ease of access.
    //
    // Most fields use a very short integer range, so we ca use Int16s to save some space.
    // Each UI Variable has an EditField, so to scale, we can have a nice gain there.
    //
    std::int64_t  cursorBlinkTimeMs;    // Keeps track of time to switch the cursor draw on and off each frame
    Rectangle     cursorRect;           // Rect of the cursor indicator. Updated by updateCursorPos()
    Rectangle     prevCursorRect;       // Text selection requires one level of cursor history
    Rectangle     selectionRect;        // Rect to draw the current text selection (if any)
    std::int16_t  textLength;           // Updated by drawSelf()
    std::int16_t  selectionStart;       // Char where a text selection starts (zero-based)
    std::int16_t  selectionEnd;         // Where it ends
    std::int16_t  prevSelectionStart;   // Used to estimate the selection direction when selecting via mouse
    std::int16_t  prevSelectionEnd;     // Ditto
    std::int16_t  cursorPos;            // Position within the line for input
    std::int16_t  prevCursorPos;        // Used during text selection navigation
    std::uint16_t selectionDir;         // Set to LeftArrow or RightArrow to indicate selection direction
    std::uint8_t  isActive         : 1; // When active the cursor and selection are drawn
    std::uint8_t  isInInsertMode   : 1; // User hit the [INSERT] key (cursor draws as a full char overlay)
    std::uint8_t  shouldDrawCursor : 1; // "Ping Pong" flag to switch cursor draw between frames
    std::uint8_t  endKeySel        : 1; // Set when [SHIFT]+[END]  is hit to select all the way to the end
    std::uint8_t  homeKeySel       : 1; // Set when [SHIFT]+[HOME] is hit to select all the way to the beginning
    std::uint8_t  isVisisble       : 1;

    // Cursor bar draw once every this many milliseconds.
    static constexpr std::int64_t CursorBlinkIntervalMs = 500;

    // Input commands for the edit field.
    enum class EditCommand
    {
        None,
        DoneEditing,
        InsertChar,
        PushChar,
        EraseChar,
        JumpNextField,
        ScrollWindowUp,
        ScrollWindowDown
    };

    EditField();
    void reset();

    bool hasTextSelection() const;
    void clearSelection();

    void setActive(bool trueIfActive);
    void setDrawCursor(bool trueIfShouldDraw);

    void drawSelf(GeometryBatch & geoBatch, Rectangle displayBox, const char * inText, int inTextLength,
                  Color32 textColor, Color32 selectionColor, Color32 cursorColor, Color32 bgColor, Float32 textScaling, Float32 uiScaling);

    EditCommand handleSpecialKey(const Rectangle & displayBox, KeyCode key, KeyModFlags modifiers, Float32 textScaling, Float32 uiScaling);

    void updateCursorPos(const Rectangle & displayBox, Point pos, Float32 textScaling, Float32 uiScaling);
    void updateSelection(const Rectangle & displayBox, Point pos, Float32 textScaling, Float32 uiScaling);
    void charInserted(const Rectangle & displayBox, Float32 textScaling, Float32 uiScaling);

    void saveCursorPos();
    void restoreCursorPos(Float32 textScaling, Float32 uiScaling);

    void moveCursorRight(const Rectangle & displayBox, Float32 textScaling, Float32 uiScaling);
    void moveCursorLeft(const Rectangle & displayBox, Float32 textScaling, Float32 uiScaling);
    void moveCursorHome(const Rectangle & displayBox, Float32 textScaling, Float32 uiScaling);
    void moveCursorEnd(const Rectangle & displayBox, Float32 textScaling, Float32 uiScaling);
    Rectangle moveCursor(const Rectangle & displayBox, Float32 newPos, Float32 textScaling, Float32 uiScaling);
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

    enum Flags : std::uint32_t
    {
        // Common to all widgets:
        Flag_Visible                 = 1 << 0,
        Flag_Minimized               = 1 << 1,
        Flag_ScrolledOutOfView       = 1 << 2,
        Flag_MouseIntersecting       = 1 << 3,
        Flag_MouseDragEnabled        = 1 << 4,
        Flag_NoRectShadow            = 1 << 5,
        Flag_NoRectBackground        = 1 << 6,
        Flag_NoRectOutline           = 1 << 7,

        // Scroll bars only:
        Flag_InvertMouseScroll       = 1 << 8,
        Flag_HoldingScrollSlider     = 1 << 9,

        // Var display widgets:
        Flag_WithValueEditButtons    = 1 << 10,
        Flag_WithEditPopupButton     = 1 << 11,
        Flag_WithCheckboxButton      = 1 << 12,
        Flag_ColorDisplayVar         = 1 << 13,

        // Windows:
        Flag_NoResizing              = 1 << 14,
        Flag_NoInfoBar               = 1 << 15,
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
    void orphanAllChildren();

    // UI/text scaling:
    void setScaling(Float32 s);
    void setTextScaling(Float32 s);
    Float32 getScaling() const;
    Float32 getTextScaling() const;
    int uiScaled(int val) const;
    static int uiScaleBy(Float64 val, Float64 scale);

    // State flags:
    bool testFlag(std::uint32_t mask) const;
    void setFlag(std::uint32_t mask, int f);

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
    std::uint32_t       flags;        // Miscellaneous state flags (from the Flags enum).
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

    enum class Icon : std::uint8_t
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
              Icon myIcon, EventListener * myListener = nullptr, bool isValueEditButton = false);

    void onDraw(GeometryBatch & geoBatch) const override;
    bool onMouseButton(MouseButton button, int clicks) override;
    void onResize(int displacementX, int displacementY, Corner corner) override;

    bool getState() const;
    void setState(bool newState);
    bool isCheckboxButton() const;

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
    bool            valueEditButton;
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
              int buttonOffsX, int buttonOffsY, int buttonSize, int buttonSpacing,
              ButtonWidget::EventListener * fwdBtnListener = nullptr);

    void onDraw(GeometryBatch & geoBatch) const override;
    bool onMouseButton(MouseButton button, int clicks) override;
    void onResize(int displacementX, int displacementY, Corner corner) override;
    void onMove(int displacementX, int displacementY) override;
    bool onButtonDown(ButtonWidget & button) override;

    void setButtonTextScaling(Float32 s);
    void setTitle(const char * newTitle);
    const char * getTitle() const;
    int getBarHeight() const;

    ButtonWidget & getMinimizeButton() { return buttons[BtnMinimize]; }
    ButtonWidget & getMaximizeButton() { return buttons[BtnMaximize]; }

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
    int          initialHeight{ 0 };

    // If set, forwards the minimize/maximize button events to this listener.
    ButtonWidget::EventListener * forwadBtnListener{ nullptr };
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
    void updateLineScrollState(int lineCount, int linesOut, int barPosition);
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

    // Callback signature:
    // - void onEntrySelected(void * userData, const ListWidget * listWidget, int selectedEntry)
    using OnEntrySelectedDelegate = Delegate<void *, void, const ListWidget *, int>;

    ListWidget();
    void init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible, OnEntrySelectedDelegate onEntrySelected = {});

    void onDraw(GeometryBatch & geoBatch) const override;
    void onMove(int displacementX, int displacementY) override;
    void onResize(int displacementX, int displacementY, Corner corner) override;
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

    // Invoked when user clicks on an entry.
    OnEntrySelectedDelegate onEntrySelectedDelegate;

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

    // Callback signature:
    // - void onColorSelected(void * userData, const ColorPickerWidget * colorPicker, Color32 selectedColor)
    using OnColorSelectedDelegate = Delegate<void *, void, const ColorPickerWidget *, Color32>;

    // Callback signature:
    // - void onClosed(void * userData, const ColorPickerWidget * colorPicker)
    using OnClosedDelegate = Delegate<void *, void, const ColorPickerWidget *>;

    ColorPickerWidget();
    void init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible,
              int titleBarHeight, int titleBarButtonSize, int scrollBarWidth,
              int scrollBarButtonSize, int clrButtonSize,
              OnColorSelectedDelegate onColorSelected = {},
              OnClosedDelegate onClosed = {});

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

    OnColorSelectedDelegate onColorSelectedDelegate;
    OnClosedDelegate        onClosedDelegate;
};

// ========================================================
// class View3DWidget:
// ========================================================

class View3DWidget final
    : public Widget, public ButtonWidget::EventListener
{
public:

    // Callback signature:
    // - void onAnglesChanged(void * userData, const View3DWidget * view3d, const Vec3 & rotationDegrees)
    using OnAnglesChangedDelegate = Delegate<void *, void, const View3DWidget *, const Vec3 &>;

    // Callback signature:
    // - void onClosed(void * userData, const View3DWidget * colorPicker)
    using OnClosedDelegate = Delegate<void *, void, const View3DWidget *>;

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

    enum class ObjectType : std::uint8_t
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
              int resetAnglesBtnSize, const ProjectionParameters & proj, ObjectType obj,
              OnAnglesChangedDelegate onAnglesChanged = {}, OnClosedDelegate onClosed = {});

    void onDraw(GeometryBatch & geoBatch) const override;
    void onMove(int displacementX, int displacementY) override;

    bool onButtonDown(ButtonWidget & button) override;
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

    const Vec3 & getRotationDegrees() const;
    void setRotationDegrees(const Vec3 & degrees);

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
    ObjectType object;                    // Object/shape geometry that is drawn in the viewport.

    // Rotation angles:
    mutable bool updateScrGeometry;       // Only update the geometry caches when needed (on input/angles changed).
    mutable bool resettingAngles;         // True when "R" clicked. New mouse input cancels it.
    mutable Vec3 rotationDegrees;         // AKA pitch (X), yaw (Y) and roll (Z).
    mutable std::int64_t prevFrameTimeMs; // Needed to compute a delta-time for angle reset lerp.
    Rectangle resetAnglesBtnRect;         // Tiny reset button in the corner ("R").

    // Screen projected geometry caches:
    mutable PODArray scrProjectedVerts;   // Cached 3D object vertexes projected to screen, ready for drawing.
    mutable PODArray scrProjectedIndexes; // Index buffer for the above verts, since RenderInterface requires it.
    ProjectionParameters projParams;      // Cached projection/viewport settings.

    // The (optional) title bar:
    TitleBarWidget titleBar;

    // User callbacks:
    OnAnglesChangedDelegate onAnglesChangedDelegate;
    OnClosedDelegate        onClosedDelegate;
};

// ========================================================
// class MultiEditFieldWidget:
// ========================================================

// Widget with multiple stacked edit fields for displaying the elements of a vector/color, e.g.:
// X: [1.1______]
// Y: [2.5______]
// Z: [3.4______]
class MultiEditFieldWidget final
    : public Widget, public ButtonWidget::EventListener
{
public:

    // Callback signature:
    // - void onGetFieldValueTextDelegate(void * userData, const MultiEditFieldWidget * widget, int fieldIndex, SmallStr * outValueText)
    using OnGetFieldValueTextDelegate = Delegate<void *, void, const MultiEditFieldWidget *, int, SmallStr *>;

    // Callback signature:
    // - void onClosed(void * userData, const MultiEditFieldWidget * widget)
    using OnClosedDelegate = Delegate<void *, void, const MultiEditFieldWidget *>;

    MultiEditFieldWidget();
    void init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible,
              const char * myTitle, int titleBarHeight, int titleBarButtonSize,
              OnGetFieldValueTextDelegate onGetFieldValueText = {}, OnClosedDelegate onClosed = {});

    void onDraw(GeometryBatch & geoBatch) const override;
    void onMove(int displacementX, int displacementY) override;
    bool onButtonDown(ButtonWidget & button) override;

    void allocFields(int count);
    void addFieldLabel(int index, const char * label);

private:

    struct Field
    {
        EditField    editField;
        int          labelTextLen;
        int          labelWidth;
        const char * labelText;
    };

    mutable PODArray fields; // [Field]

    // The (optional) title bar:
    TitleBarWidget titleBar;

    // User callbacks:
    OnGetFieldValueTextDelegate onGetFieldValueTextDelegate;
    OnClosedDelegate            onClosedDelegate;
};

// ========================================================
// class FloatValueSliderWidget:
// ========================================================

class FloatValueSliderWidget final
    : public Widget, public ButtonWidget::EventListener
{
public:

    // Callback signature:
    // - Float64 onGetFloatValue(void * userData, const FloatValueSliderWidget * widget)
    using OnGetFloatValueDelegate = Delegate<void *, Float64, const FloatValueSliderWidget *>;

    // Callback signature:
    // - void onClosed(void * userData, const FloatValueSliderWidget * widget)
    using OnClosedDelegate = Delegate<void *, void, const FloatValueSliderWidget *>;

    FloatValueSliderWidget();
    void init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible,
              const char * myTitle, int titleBarHeight, int titleBarButtonSize,
              OnGetFloatValueDelegate onGetFloatValue = {}, OnClosedDelegate onClosed = {});

    void onDraw(GeometryBatch & geoBatch) const override;
    void onMove(int displacementX, int displacementY) override;
    bool onButtonDown(ButtonWidget & button) override;

    void setRange(Float64 min, Float64 max) { slider.setRange(min, max); }

private:

    mutable ValueSlider slider;

    // The (optional) title bar:
    TitleBarWidget titleBar;

    // User callbacks:
    OnGetFloatValueDelegate onGetFloatValueDelegate;
    OnClosedDelegate        onClosedDelegate;
};

// ========================================================
// class VarDisplayWidget:
// ========================================================

class VarDisplayWidget
    : public Widget, public ButtonWidget::EventListener
{
public:

    VarDisplayWidget();
    virtual ~VarDisplayWidget();

    void init(GUI * myGUI, VarDisplayWidget * myParent, const Rectangle & myRect,
              bool visible, WindowWidget * myWindow, const char * name,
              std::uint32_t varWidgetFlags = 0, bool checkboxInitialState = false);

    void onDraw(GeometryBatch & geoBatch) const override final;
    void onMove(int displacementX, int displacementY) override final;
    void onResize(int displacementX, int displacementY, Corner corner) override final;

    void onAdjustLayout() override final;
    void onDisableEditing() override final;
    void setVisible(bool visible) override final;

    bool onMouseButton(MouseButton button, int clicks) override final;
    bool onMouseMotion(int mx, int my) override final;
    bool onMouseScroll(int yScroll) override final;
    bool onKeyPressed(KeyCode key, KeyModFlags modifiers) override final;
    bool onButtonDown(ButtonWidget & button) override final;

    void setButtonTextScaling(Float32 s);
    int getExpandCollapseButtonSize() const;
    Rectangle getExpandCollapseButtonRect() const;
    void addExpandCollapseButton();
    bool hasExpandCollapseButton() const;
    bool isHierarchyCollapsed() const;

    const Rectangle & getDataDisplayRect() const;
    void setDataDisplayRect(const Rectangle & newRect);

    const WindowWidget * getParentWindow() const;
    WindowWidget * getParentWindow();

    const SmallStr & getVarName() const;
    void setVarName(const SmallStr & name);
    void setVarName(const char * name);

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const override final;
    #endif // NEO_TWEAK_BAR_DEBUG

protected:

    virtual bool onGetVarValueText(SmallStr &) const { return false; }
    virtual void onSetVarValueText(const SmallStr &) {}

    virtual void onIncrementButton() {}
    virtual void onDecrementButton() {}
    virtual void onEditPopupButton(bool) {}
    virtual void onCheckboxButton(bool)  {}

    void setExpandCollapseState(bool expanded);
    void setEditFieldBackground(Color32 bg) { editFieldBackground = bg; }
    ButtonWidget & getEditPopupButton() { return editPopupButton; }

private:

    struct ButtonRects
    {
        Rectangle incrButtonRect;
        Rectangle decrButtonRect;
        Rectangle editPopupButtonRect;
        Rectangle checkboxButtonRect;
    };

    int getMinDataDisplayRectWidth() const;
    Rectangle makeDataDisplayAndButtonRects(ButtonRects & outBtnRects, bool withValueEditButtons, bool withEditPopupButton, bool withCheckboxButton) const;
    void setHierarchyVisibility(VarDisplayWidget* child, bool visible) const;

    void drawVarName(GeometryBatch& geoBatch) const;
    void drawVarValue(GeometryBatch& geoBatch) const;
    void drawValueEditButtons(GeometryBatch& geoBatch) const;

private:

    // Need the extra reference to the parent window because the
    // 'parent' field of a VarDisplayWidget might be another
    // VarDisplayWidget for nested var instances.
    WindowWidget * parentWindow;

    // Cached data:
    Rectangle dataDisplayRect;
    int initialHeight;
    int titleWidth;

    // Button state true if hierarchy open, false if collapsed.
    // Note that the button will be made a child of the parent widget (Window), so the
    // VarDisplayWidget::children list only containers nested child VarDisplayWidgets.
    mutable ButtonWidget expandCollapseButton;

    // [+][-] edit buttons for numbers.
    mutable ButtonWidget incrButton;
    mutable ButtonWidget decrButton;

    // Open edit popup window for colors/vectors/enums.
    mutable ButtonWidget editPopupButton;

    // Boolean variables get a checkbox button.
    mutable ButtonWidget checkboxButton;

    // Mutable because EditField::drawSelf() needs to update some internal state.
    mutable EditField editField;
    Color32 editFieldBackground;

    // Last value queried form the user variable as text. Updated by drawVarValue().
    mutable SmallStr cachedValueText;

    // Name displayed in the UI.
    // Can contain any ASCII character, including whitespace.
    SmallStr varName;
};

// ========================================================
// class WindowWidget:
// ========================================================

class WindowWidget
    : public Widget
{
public:

    WindowWidget();
    virtual ~WindowWidget();

    void init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible, bool resizeable,
              const char * title, int titleBarH, int titleBarBtnSize, int scrollBarW, int scrollBarBtnSize);

    virtual void onDraw(GeometryBatch & geoBatch) const override;
    virtual void onMove(int displacementX, int displacementY) override;
    virtual void onAdjustLayout() override;
    virtual void onDisableEditing() override;
    virtual void setMouseIntersecting(bool intersect) override;

    virtual bool onMouseButton(MouseButton button, int clicks) override;
    virtual bool onMouseMotion(int mx, int my) override;
    virtual bool onMouseScroll(int yScroll) override;
    virtual bool onKeyPressed(KeyCode key, KeyModFlags modifiers) override;

    const Rectangle & getUsableRect() const;
    void setUsableRect(const Rectangle & newRect);

    const char * getTitle() const;
    void setTitle(const char * newTitle);

    ScrollBarWidget & getScrollBar();
    IntrusiveList<EditField> & getEditFieldList();

    void addEditField(EditField * editField);
    void removeEditField(EditField * editField);

    // NOTE: Window takes ownership of the Widget and will deleted when itself is destroyed.
    void setPopupWidget(Widget * popup);
    Widget * getPopupWidget() const;
    void destroyPopupWidget();

    int getMinWindowWidth() const;
    int getMinWindowHeight() const;
    void setMinWindowWidth(int w);
    void setMinWindowHeight(int h);

    void setButtonTextScaling(Float32 s);
    void setResizeable(bool resizeable);
    bool isResizeable() const;

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const override;
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    void drawResizeHandles(GeometryBatch & geoBatch) const;
    void resizeWithMin(Corner corner, int & x, int & y, int offsetX, int offsetY);
    void refreshBarRects(const char * newTitle, const char * newInfoString);
    void refreshUsableRect();

    Rectangle usableRect;     // Size discounting the top/side bars.
    Corner    resizingCorner; // Which corner being resized by user input.
    Widget *  popupWidget;    // Each window can have one open popup at a time. Popups are always window children.

    // Global list of EditFields attached to this window for easy access.
    IntrusiveList<EditField> editFieldsList;

    // These are always present in a window, so we can avoid a mem alloc and declare them inline.
    ScrollBarWidget scrollBar;
    TitleBarWidget  titleBar;
    InfoBarWidget   infoBar;

    // No need for a full 32-bits for these; save the space.
    std::int16_t titleBarButtonSize;
    std::int16_t titleBarHeight;
    std::int16_t scrollBarButtonSize;
    std::int16_t scrollBarWidth;
    std::int16_t minWindowWidth;
    std::int16_t minWindowHeight;
};

// ========================================================
// class ConsoleWindowWidget:
// ========================================================

class ConsoleWindowWidget final
    : public WindowWidget
{
public:

    ConsoleWindowWidget();
    virtual ~ConsoleWindowWidget();

    void init(GUI * myGUI, Widget * myParent, const Rectangle & myRect, bool visible, bool resizeable,
              const char * title, int titleBarH, int titleBarBtnSize, int scrollBarW, int scrollBarBtnSize,
              int maxLineCount, int maxBufferSize);

    void onDraw(GeometryBatch & geoBatch) const override;
    void onScrollContentUp() override;
    void onScrollContentDown() override;
    void onAdjustLayout() override;

    void pushLine(const char * text);
    void pushLine(const char * text, int length);

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const override;
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    struct Line
    {
        EditField    edit;  // Edit Field to allow selecting and copying the console lines.
        std::int32_t start; // Line text start in the console buffer.
        std::int32_t end;   // Line text end in the console buffer.
    };

    const char * getTextForLine(const Line & line) const;
    int getTextLengthForLine(const Line & line) const;

    std::int32_t linesUsed;
    std::int32_t maxLines;
    std::int32_t firstLineDrawn;
    std::int32_t bufferUsed;
    std::int32_t bufferSize;
    Line *       lines;
    char *       buffer;
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

// ========================================================
// Inline methods for the Widget class:
// ========================================================

inline bool Widget::isVisible() const
{
    return testFlag(Flag_Visible);
}

inline bool Widget::isMinimized() const
{
    return testFlag(Flag_Minimized);
}

inline bool Widget::isScrolledOutOfView() const
{
    return testFlag(Flag_ScrolledOutOfView);
}

inline bool Widget::isMouseIntersecting() const
{
    return testFlag(Flag_MouseIntersecting);
}

inline bool Widget::isMouseDragEnabled() const
{
    return testFlag(Flag_MouseDragEnabled);
}

inline void Widget::setVisible(bool visible)
{
    setFlag(Flag_Visible, visible);
}

inline void Widget::setMouseIntersecting(bool intersect)
{
    setFlag(Flag_MouseIntersecting, intersect);
}

inline void Widget::setMinimized(bool minimized)
{
    setFlag(Flag_Minimized, minimized);
}

inline void Widget::setScrolledOutOfView(bool outOfView)
{
    setFlag(Flag_ScrolledOutOfView, outOfView);
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
    onAdjustLayout();
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

inline void Widget::orphanAllChildren()
{
    children.clear();
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

inline bool Widget::testFlag(std::uint32_t mask) const
{
    return !!(flags & mask);
}

inline void Widget::setFlag(std::uint32_t mask, int f)
{
    // Using one of the Standford bit-hacks:
    // (Conditionally set or clear bit without branching)
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

inline bool ButtonWidget::isCheckboxButton() const
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
    setFlag(Flag_InvertMouseScroll, invert);
}

inline bool ScrollBarWidget::isMouseScrollInverted() const
{
    return testFlag(Flag_InvertMouseScroll);
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

inline const Vec3 & View3DWidget::getRotationDegrees() const
{
    return rotationDegrees;
}

inline void View3DWidget::setRotationDegrees(const Vec3 & degrees)
{
    rotationDegrees = degrees;
}

#if NEO_TWEAK_BAR_DEBUG
inline SmallStr View3DWidget::getTypeString() const
{
    return "View3DWidget";
}
#endif // NEO_TWEAK_BAR_DEBUG

// ========================================================
// Inline methods for the VarDisplayWidget class:
// ========================================================

inline void VarDisplayWidget::setButtonTextScaling(Float32 s)
{
    expandCollapseButton.setTextScaling(s);
}

inline int VarDisplayWidget::getExpandCollapseButtonSize() const
{
    return Widget::uiScaleBy(initialHeight, 0.8);
}

inline bool VarDisplayWidget::hasExpandCollapseButton() const
{
    return expandCollapseButton.getIcon() != ButtonWidget::Icon::None;
}

inline bool VarDisplayWidget::isHierarchyCollapsed() const
{
    return hasExpandCollapseButton() && (expandCollapseButton.getState() == false);
}

inline const Rectangle & VarDisplayWidget::getDataDisplayRect() const
{
    return dataDisplayRect;
}

inline void VarDisplayWidget::setDataDisplayRect(const Rectangle & newRect)
{
    dataDisplayRect = newRect;
}

inline const WindowWidget * VarDisplayWidget::getParentWindow() const
{
    return parentWindow;
}

inline WindowWidget * VarDisplayWidget::getParentWindow()
{
    return parentWindow;
}

inline const SmallStr & VarDisplayWidget::getVarName() const
{
    return varName;
}

inline void VarDisplayWidget::setVarName(const SmallStr & name)
{
    varName = name;
    titleWidth = (int)GeometryBatch::calcTextWidth(varName.c_str(), varName.getLength(), gui->getGlobalTextScaling());
}

inline void VarDisplayWidget::setVarName(const char * name)
{
    varName = name;
}

// ========================================================
// Inline methods for the WindowWidget class:
// ========================================================

inline const Rectangle & WindowWidget::getUsableRect() const
{
    return usableRect;
}

inline void WindowWidget::setUsableRect(const Rectangle & newRect)
{
    usableRect = newRect;
}

inline const char * WindowWidget::getTitle() const
{
    return titleBar.getTitle();
}

inline void WindowWidget::setTitle(const char * newTitle)
{
    titleBar.setTitle(newTitle);
}

inline ScrollBarWidget & WindowWidget::getScrollBar()
{
    return scrollBar;
}

inline IntrusiveList<EditField> & WindowWidget::getEditFieldList()
{
    return editFieldsList;
}

inline void WindowWidget::addEditField(EditField * editField)
{
    editFieldsList.pushBack(editField);
}

inline void WindowWidget::removeEditField(EditField * editField)
{
    editFieldsList.unlink(editField);
}

inline void WindowWidget::setPopupWidget(Widget * popup)
{
    NTB_ASSERT(popupWidget == nullptr); // should call destroyPopupWidget first.
    popupWidget = popup;
}

inline Widget * WindowWidget::getPopupWidget() const
{
    return popupWidget;
}

inline int WindowWidget::getMinWindowWidth() const
{
    return minWindowWidth;
}

inline int WindowWidget::getMinWindowHeight() const
{
    return minWindowHeight;
}

inline void WindowWidget::setMinWindowWidth(int w)
{
    minWindowWidth = static_cast<std::int16_t>(w);
}

inline void WindowWidget::setMinWindowHeight(int h)
{
    minWindowHeight = static_cast<std::int16_t>(h);
}

inline void WindowWidget::setButtonTextScaling(Float32 s)
{
    titleBar.setButtonTextScaling(s);
}

inline void WindowWidget::setResizeable(bool resizeable)
{
    setFlag(Flag_NoResizing, !resizeable);
}

inline bool WindowWidget::isResizeable() const
{
    return !testFlag(Flag_NoResizing);
}

} // namespace ntb {}
