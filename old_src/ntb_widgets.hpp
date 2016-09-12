
// ================================================================================================
// -*- C++ -*-
// File: ntb_widgets.hpp
// Author: Guilherme R. Lampert
// Created on: 21/09/15
// Brief: Widgets are the back-end UI elements/components of NTB.
// ================================================================================================

#ifndef NEO_TWEAK_BAR_WIDGETS_HPP
#define NEO_TWEAK_BAR_WIDGETS_HPP

//for Widget::printHierarchy
#include <iostream>
#define NEO_TWEAK_BAR_DEBUG 1

namespace ntb
{

// Widget types (UI elements):
class Widget;
class ButtonWidget;
class TitleBarWidget;
class InfoBarWidget;
class ScrollBarWidget;
class ValueSliderWidget;
class ColorPickerWidget;
class View3DWidget;
class ListWidget;
class VarDisplayWidget;
class WindowWidget;

class EditField;
class Variable;
class Panel;
class GUI;

class NumberEx;
class BoolEx;
class Float4Ex;
class ColorEx;
class EnumValEx;

struct MouseButton
{
    enum
    {
        Left,
        Right,
        Middle
    };
    typedef Int8 Enum;
    static SmallStr toString(MouseButton::Enum button);
};

//TODO need a global flag to define if we are using Mac-style CMD+C/CMD+V/CMD+etc
//or Windows style CTRL+C/CTRL+V/CTRL+etc

// KeyModifiers can be ORed together.
typedef UInt32 KeyModFlags;

struct KeyModifiers
{
    enum
    {
        Shift = 1 << 0,
        Ctrl  = 1 << 1,
        Cmd   = 1 << 2,
    };
    static SmallStr toString(KeyModFlags modifiers);
};

// KEYBOARD
//
// - Lowercase ASCII keys + 0-9 and chars
// - Special keys as the following enum
//

// Either an ASCII key or one of the SpecialKeys enum.
typedef UInt32 KeyCode;

struct SpecialKeys
{
    enum
    {
        // Zero is reserved as a flag for "no key pressed".
        Null = 0,

        // First 0-255 keys are reserved for the ASCII characters.
        Return = 256,
        Escape,
        Backspace,
        Delete,
        Tab,
        Home,
        End,
        PageUp,
        PageDown,
        UpArrow,
        DownArrow,
        RightArrow,
        LeftArrow,
        Insert,

        // These are not used and free for user-defined bindings.
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,

        // Sentinel value; Used internally.
        LastKey
    };
    static SmallStr toString(KeyCode keyCode);
};

// ========================================================
// struct ColorScheme:
// ========================================================

// user can define color schemes
struct ColorScheme
{
    struct Box
    {
        // Box background:
        Color32 bgTopLeft;
        Color32 bgTopRight;
        Color32 bgBottomLeft;
        Color32 bgBottomRight;
        // Box outline:
        Color32 outlineTop;
        Color32 outlineBottom;
        Color32 outlineLeft;
        Color32 outlineRight;
    } box;

    struct Shadow
    {
        Color32 dark;
        Color32 light;
        int     offset;
    } shadow;

    struct Text
    {
        Color32 normal;
        Color32 alternate;
        Color32 informational;
    } text;

    //TODO
    // possibly also:
    //
    // - button.hover
    // - button.clicked
    // - button.outline (the border of buttons)
    //
    // Something for the window wedge lines...
    //
};

// ========================================================
// class Widget:
// ========================================================

// basically, an interactive screen element.
// A panel has a widget, but so does a button
// or a tweakable parameter. Widget are drawable
// and also respond to input.
class Widget
{
    NTB_DISABLE_COPY_ASSIGN(Widget);

public:

    enum Flags
    {
        FlagVisible           = 1 << 0,
        FlagMinimized         = 1 << 1,
        FlagScrolledOutOfView = 1 << 2,
        FlagMouseIntersecting = 1 << 3,
        FlagMouseDragEnabled  = 1 << 4,
        FlagNoRectShadow      = 1 << 5
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

    Widget();
    Widget(GUI * myGUI, Widget * myParent, const Rectangle & myRect);
    virtual ~Widget();

    virtual void onDraw(GeometryBatch & geoBatch) const;
    virtual bool onMouseButton(MouseButton::Enum button, int clicks);
    virtual bool onMouseMotion(int mx, int my);
    virtual bool onMouseScroll(int yScroll);
    virtual bool onKeyPressed(KeyCode key, KeyModFlags modifiers);
    virtual void onResize(int displacementX, int displacementY, Corner corner);
    virtual void onMove(int displacementX, int displacementY);
    virtual void onScrollContentUp();
    virtual void onScrollContentDown();
    virtual void onAdjustLayout();
    virtual void onDisableEditing();
    virtual void setVisible(const bool visible) { setFlag(FlagVisible, visible); }
    virtual void setMouseIntersecting(const bool intersect) { setFlag(FlagMouseIntersecting, intersect); }

    bool isVisible() const { return testFlag(FlagVisible); }
    bool isMinimized() const { return testFlag(FlagMinimized); }
    bool isScrolledOutOfView() const { return testFlag(FlagScrolledOutOfView); }
    bool isMouseIntersecting() const { return testFlag(FlagMouseIntersecting); }
    bool isMouseDragEnabled() const { return testFlag(FlagMouseDragEnabled); }

    void setMinimized(const bool minimized) { setFlag(FlagMinimized, minimized); }
    void setScrolledOutOfView(const bool outOfView) { setFlag(FlagScrolledOutOfView, outOfView); }
    void setMouseDragEnabled(bool enabled);

    void setGUI(GUI * newGUI) { NTB_ASSERT(newGUI != NTB_NULL); gui = newGUI; }
    void setParent(Widget * newParent) { parent = newParent; }
    void setColors(const ColorScheme * newColors) { NTB_ASSERT(newColors != NTB_NULL); colors = newColors; }
    void setRect(const Rectangle & newRect) { rect = newRect; }
    void setNormalColors();
    void setHighlightedColors();

    // non-null
    const ColorScheme & getColors() const { NTB_ASSERT(colors != NTB_NULL); return *colors; }
    const Rectangle & getRect() const { return rect; }

    // non-null
    const GUI * getGUI() const { NTB_ASSERT(gui != NTB_NULL); return gui; }
    GUI * getGUI() { NTB_ASSERT(gui != NTB_NULL); return gui; }

    // can be null
    const Widget * getParent() const { return parent; }
    Widget * getParent() { return parent; }

    const Widget * getChild(const int index) const { return children.get<const Widget *>(index); }
    Widget * getChild(const int index) { return children.get<Widget *>(index); }

    bool isChild(const Widget * widget) const;
    void addChild(Widget * newChild);
    int getChildCount() const { return children.getSize(); }

    Float32 getTextScaling() const;
    Float32 getScaling() const;
    int uiScaled(int val) const;
    int uiScaleBy(int val, Float32 scale) const;

    bool testFlag(const UInt32 mask) const
    {
        return !!(flags & mask);
    }
    void setFlag(const UInt32 mask, const bool f)
    {
        // Using one of the Standford bit-hacks:
        // http://graphics.stanford.edu/~seander/bithacks.html
        flags = (flags & ~mask) | (-f & mask);
    }

    #if NEO_TWEAK_BAR_DEBUG
    virtual void printHierarchy(std::ostream & out = std::cout, const SmallStr & indent = "") const;
    virtual SmallStr getTypeString() const { return "Widget"; }
    #endif // NEO_TWEAK_BAR_DEBUG

protected:

    void drawSelf(GeometryBatch & geoBatch) const;
    void drawChildren(GeometryBatch & geoBatch) const;

    GUI * gui;                  // direct pointer to the owning UI for things like colorscheme and scaling. never null
    Widget * parent;            // direct parent in the hierarchy (like a Panel/Window). may be null
    const ColorScheme * colors; // pointer so we can hot swap it. but never null!

    PODArray children; //TODO NOTE Objects not being deleted! Make sure we don't leak!!!
                       // Not all elements are dynamically allocated, so we need a flag for that!

    Rectangle rect;
    Point lastMousePos;

    UInt32 flags;
};

// ========================================================
// class ButtonEventListener:
// ========================================================

class ButtonEventListener
{
public:

    // Callback fired when the button is left-clicked. Should return true if
    // the event was handle. Default implementation is a no-op that return false.
    virtual bool onButtonDown(ButtonWidget & button);

protected:

    // Protected and non-virtual, since this class is meant for use as a mixin.
    ~ButtonEventListener();
};

// ========================================================
// class ButtonWidget:
// ========================================================

class ButtonWidget NTB_FINAL_CLASS
    : public Widget
{
public:

    enum Icon
    {
        None,      // No button. Nothing drawn, no events.
        Plus,      // [+] Plus sign.
        Minus,     // [-] Minis sign/dash.
        UpArrow,   // [/\] Two lines similar to an arrowhead pointing up.
        DownArrow, // [\/] Two lines similar to an arrowhead pointing down.
        LeftRight, // [<>] Four lines similar to the less/greater signs.
        Question,  // [?] A few lines that look like a question mark.
        CheckMark, // Actually turns the button into a check mark switch.

        // Number of entries in this enum. Internal use.
        IconCount
    };

    ButtonWidget();
    ButtonWidget(GUI * myGUI, Widget * myParent, const Rectangle & myRect,
                 Icon myIcon, ButtonEventListener * listener = NTB_NULL);

    //TODO should make this construct/init interface uniform with all widgets...
    void reset(GUI * myGUI, Widget * myParent, const Rectangle & myRect,
               Icon myIcon, ButtonEventListener * listener = NTB_NULL);

    void onDraw(GeometryBatch & geoBatch) const NTB_OVERRIDE;
    bool onMouseButton(MouseButton::Enum button, int clicks) NTB_OVERRIDE;

    bool isCheckBoxButton() const { return icon == CheckMark; }

    bool getState() const { return state; }
    void setState(bool newState) { state = newState; }

    Icon getIcon() const { return icon; }
    void setIcon(Icon newIcon) { icon = newIcon; }

    bool hasEventListener() const { return eventListener != NTB_NULL; }
    ButtonEventListener * getEventListener() const { return eventListener; }
    void setEventListener(ButtonEventListener * newListener) { eventListener = newListener; }

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const NTB_OVERRIDE { return "ButtonWidget"; }
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    ButtonEventListener * eventListener; // not owned by button!
    Icon icon;
    bool state; // flipped at each click event. starts with false
};

// ========================================================
// class TitleBarWidget:
// ========================================================

class TitleBarWidget NTB_FINAL_CLASS
    : public Widget, public ButtonEventListener
{
public:

    TitleBarWidget() { }
    TitleBarWidget(GUI * myGUI, Widget * myParent, const Rectangle & myRect,
                   const char * title, bool minimizeButton, bool maximizeButton,
                   int buttonOffsX, int buttonOffsY);

    void reset(GUI * myGUI, Widget * myParent, const Rectangle & myRect,
               const char * title, bool minimizeButton, bool maximizeButton,
               int buttonOffsX, int buttonOffsY);

    // Widget methods:
    void onDraw(GeometryBatch & geoBatch) const NTB_OVERRIDE;
    bool onMouseButton(MouseButton::Enum button, int clicks) NTB_OVERRIDE;
    void onMove(int displacementX, int displacementY) NTB_OVERRIDE;
    void onResize(int displacementX, int displacementY, Corner corner) NTB_OVERRIDE;

    // ButtonEventListener methods:
    bool onButtonDown(ButtonWidget & button) NTB_OVERRIDE;

    void setTitle(const char * newTitle) { titleText = newTitle; }
    const char * getTitle() const { return titleText.c_str(); }

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const NTB_OVERRIDE { return "TitleBarWidget"; }
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    void buttonSetup(bool minimizeButton, bool maximizeButton,
                     int buttonOffsX, int buttonOffsY);

    enum { BtnMinimize, BtnMaximize, BtnCount };
    ButtonWidget buttons[BtnCount];
    SmallStr titleText;
};

// ========================================================
// class InfoBarWidget:
// ========================================================

// bar at the base of a window/panel that displays things like keys pressed
class InfoBarWidget NTB_FINAL_CLASS
    : public Widget
{
public:

    InfoBarWidget();
    InfoBarWidget(GUI * myGUI, Widget * myParent, const Rectangle & myRect, const char * myText);
    void reset(GUI * myGUI, Widget * myParent, const Rectangle & myRect, const char * myText);

    void onDraw(GeometryBatch & geoBatch) const NTB_OVERRIDE;
    void onResize(int displacementX, int displacementY, Corner corner) NTB_OVERRIDE;

    void setText(const char * newText) { infoText = newText; }
    const char * getText() const { return infoText.c_str(); }

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const NTB_OVERRIDE { return "InfoBarWidget"; }
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    SmallStr infoText;
};

// ========================================================
// class ScrollBarWidget:
// ========================================================

class ScrollBarWidget NTB_FINAL_CLASS
    : public Widget
{
public:

    ScrollBarWidget();
    ScrollBarWidget(GUI * myGUI, Widget * myParent, const Rectangle & myRect);
    void reset(GUI * myGUI, Widget * myParent, const Rectangle & myRect);

    void onDraw(GeometryBatch & geoBatch) const NTB_OVERRIDE;

    bool onMouseButton(MouseButton::Enum button, int clicks) NTB_OVERRIDE;
    bool onMouseMotion(int mx, int my) NTB_OVERRIDE;
    bool onMouseScroll(int yScroll) NTB_OVERRIDE;

    void onMove(int displacementX, int displacementY) NTB_OVERRIDE;
    void onResize(int displacementX, int displacementY, Corner corner) NTB_OVERRIDE;

    void onAdjustLayout() NTB_OVERRIDE;

    void updateLineScrollState(int lineCount, int linesOut);

    // TODO expose somewhere in the front-end!
    void setInvertMouseScroll(const bool invert) { invertMouseScroll = invert; }
    bool isMouseScrollInverted() const { return invertMouseScroll; }

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const NTB_OVERRIDE { return "ScrollBarWidget"; }
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    void doScrollUp();
    void doScrollDown();

    Rectangle makeInnerBarRect()   const;
    Rectangle makeUpButtonRect()   const;
    Rectangle makeDownButtonRect() const;

    // Scroll slider states:
    int scrollBarOffsetY;      // Current Y start of slider bar.
    int scrollBarDisplacement; // Amount to move each click.
    int scrollBarSizeFactor;   // Slider box scale: [0,100].
    int scrollBarThickness;    // Thickness of slider bar. A fraction of the bar's box.

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

    //TODO note: might combine these into the Widget::Flags in the future...
    bool holdingScrollSlider;
    bool invertMouseScroll;
};

// ========================================================
// class ValueSliderWidget:
// ========================================================

//  slider bar [+]------I------[+]
//             Min             Max
//
class ValueSliderWidget NTB_FINAL_CLASS
    : public Widget, public ButtonEventListener
{
public:

    ValueSliderWidget(GUI * myGUI, Widget * myParent, const Rectangle & myRect);

    // Widget methods:
    void onDraw(GeometryBatch & geoBatch) const NTB_OVERRIDE;
    void onMove(int displacementX, int displacementY) NTB_OVERRIDE;

    // ButtonEventListener methods:
    bool onButtonDown(ButtonWidget & button) NTB_OVERRIDE;

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const NTB_OVERRIDE { return "ValueSliderWidget"; }
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    Rectangle barRect;
    Rectangle sliderRect;

    enum { BtnMinus, BtnPlus, BtnCount };
    ButtonWidget buttons[BtnCount];

    // probably also a min/max value as a normalized 0,1 Float32.
};

// ========================================================
// class ColorPickerWidget:
// ========================================================

// NOTE:
//  Can't have more than one instance of a color picker open at a time,
//  so we can make it shared with all windows that have the same color scheme
//  (probably at the GUI level)
//
class ColorPickerWidget NTB_FINAL_CLASS
    : public Widget, public ButtonEventListener
{
public:

    ColorPickerWidget(GUI * myGUI, Widget * myParent, int xStart, int yStart);

    void onDraw(GeometryBatch & geoBatch) const NTB_OVERRIDE;
    void onMove(int displacementX, int displacementY) NTB_OVERRIDE;

    bool onMouseButton(MouseButton::Enum button, int clicks) NTB_OVERRIDE;
    bool onMouseMotion(int mx, int my) NTB_OVERRIDE;
    bool onMouseScroll(int yScroll) NTB_OVERRIDE;

    void setMouseIntersecting(bool intersect) NTB_OVERRIDE;

    void onScrollContentUp() NTB_OVERRIDE;
    void onScrollContentDown() NTB_OVERRIDE;

    bool onButtonDown(ButtonWidget & button) NTB_OVERRIDE;

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const NTB_OVERRIDE { return "ColorPickerWidget"; }
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    void refreshUsableRect();
    bool drawColorButton(Rectangle colorRect, int colorIndex, GeometryBatch * pGeoBatch) const;
    bool testColorButtonClick(Rectangle colorRect, int colorIndex, GeometryBatch * pUnused) const;

    typedef bool (ColorPickerWidget::*ButtonFunc)(Rectangle, int, GeometryBatch *) const;
    bool forEachColorButton(ButtonFunc pFunc, GeometryBatch * pGeoBatch) const;

    Rectangle usableRect; // discounts the top/side bars

    int colorButtonLinesScrolledUp;
    static const int None = -1;
    mutable int selectedColorIndex; // index into g_colorTable[]. -1 no selection

    TitleBarWidget titleBar;
    ScrollBarWidget scrollBar;
};

// ========================================================
// class View3DWidget:
// ========================================================

struct ProjectionParameters
{
    Rectangle viewport;
    Float32 fovYRadians;
    Float32 aspectRatio;
    Float32 zNear;
    Float32 zFar;
    bool autoAdjustAspect;
    Mat4x4 viewProjMatrix;
};

class View3DWidget NTB_FINAL_CLASS
    : public Widget
{
public:

    View3DWidget(GUI * myGUI, Widget * myParent, const Rectangle & myRect,
                 const char * title, const ProjectionParameters & proj);

    void onDraw(GeometryBatch & geoBatch) const NTB_OVERRIDE;
    void onMove(int displacementX, int displacementY) NTB_OVERRIDE;

    bool onMouseButton(MouseButton::Enum button, int clicks) NTB_OVERRIDE;
    bool onMouseMotion(int mx, int my) NTB_OVERRIDE;
    bool onMouseScroll(int yScroll) NTB_OVERRIDE;

    void setMouseIntersecting(bool intersect) NTB_OVERRIDE;

    // TODO expose somewhere in the front-end!
    void setInvertMouseY(const bool invert) { invertMouseY = invert; }
    bool isMouseYInverted() const { return invertMouseY; }

    void  setMouseSensitivity(const Float32 sensitivity) { mouseSensitivity = sensitivity; }
    Float32 getMouseSensitivity() const { return mouseSensitivity; }

    void setMaxMouseDelta(const int max) { maxMouseDelta = max; }
    int  getMaxMouseDelta() const { return maxMouseDelta; }

    void setShowXyzLabels(const bool show) { showXyzLabels = show; }
    bool isShowingXyzLabels() const { return showXyzLabels; }

    void setInteractive(const bool interactive) { interactiveControls = interactive; }
    bool isInteractive() const { return interactiveControls; }

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const NTB_OVERRIDE { return "View3DWidget"; }
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    //TODO vertex caches should be preallocated on construction!
    enum ArrowDir { ArrowDirX, ArrowDirY, ArrowDirZ };
    void clearScreenVertexCaches() const;
    void submitScreenVertexCaches(GeometryBatch & geoBatch) const;
    void addScreenProjectedSphere(const Mat4x4 & modelToWorldMatrix, Float32 scaleXYZ) const;
    void addScreenProjectedArrow(const Mat4x4 & modelToWorldMatrix, Float32 scaleXYZ, Color32 color, ArrowDir dir) const;
    void addScreenProjectedBox(const Mat4x4 & modelToWorldMatrix, Float32 w, Float32 h, Float32 d, Color32 color) const;
    void refreshProjectionViewport();

    // Local mouse states:
    Point   mouseDelta;                   // XY deltas to compute the mouse rotations.
    Float32 mouseSensitivity;             // [0,1] range: 0=very low; 1=very high.
    int     maxMouseDelta;                // Any range; default is 20.
    bool    invertMouseY;                 // Inverts mouse rotation of the object in the Y-axis.
    bool    leftMouseButtonDown;

    // Misc switches:
    bool interactiveControls;             // Allow mouse input and the reset button. Defaults to true.
    bool showXyzLabels;                   // Show the XYZ label at the right corner. Defaults to true.

    // Rotation angles:
    mutable bool  updateScrGeometry;      // Only update the geometry caches when needed (on input/angles changed).
    mutable bool  resettingAngles;        // True when "R" clicked. New mouse input cancels it.
    mutable Vec3  rotationDegrees;        // AKA pitch (X), yaw (Y) and roll (Z).
    mutable Int64 prevFrameTimeMs;        // Needed to compute a delta-time for angle reset lerp.
    Rectangle     resetAnglesBtnRect;     // Tiny reset button in the corner ("R").

    // Screen projected geometry caches:
    mutable PODArray scrProjectedVerts;   // Cached 3D object vertexes projected to screen, ready for drawing.
    mutable PODArray scrProjectedIndexes; // Index buffer for the above verts, since RenderInterface requires it.
    ProjectionParameters projParams;      // Cached projection/viewport settings.

    // The (optional) title bar:
    TitleBarWidget titleBar;
};

// ========================================================
// class ListWidget:
// ========================================================

// +---------+
// | Entry 0 |
// +---------+
// | Entry 1 |
// +---------+
// | .....   |
// +---------+
// Window size adjusted to the longest string.
//
// NOTE: might be worth exposing this widget in the
// front-end as a ButtonPanel class...
//
class ListWidget NTB_FINAL_CLASS
    : public Widget
{
public:

    ListWidget(GUI * myGUI, Widget * myParent, const Rectangle & myRect);

    void onDraw(GeometryBatch & geoBatch) const NTB_OVERRIDE;
    void onMove(int displacementX, int displacementY) NTB_OVERRIDE;

    bool onMouseButton(MouseButton::Enum button, int clicks) NTB_OVERRIDE;
    bool onMouseMotion(int mx, int my) NTB_OVERRIDE;

    void allocEntries(int count);
    int getNumOfEntries() const;

    void addEntryText(int index, const char * value);
    SmallStr getEntryText(int index) const;

    int  getSelectedEntry() const;
    bool hasSelectedEntry() const;
    void clearSelectedEntry();

private:

    void addEntryRect(int entryIndex, int entryLengthInChars);
    int findEntryForPoint(int x, int y) const;

    struct Entry
    {
        // Clickable rectangle:
        Rectangle rect;

        // Offsets into 'strings':
        int firstChar;
        int lengthInChars;
    };
    PODArray entries; // [Entry]

    // User selection in the list. No selection if < 0.
    static const int None = -1;
    int selectedEntry;
    int hoveredEntry;

    // All strings in the list entries/buttons are
    // packed into this string, no spacing in between.
    SmallStr strings;
};

// ========================================================
// class EditField and EditCommands enum:
// ========================================================

struct EditCommand
{
    enum
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
    typedef Int8 Enum;
};

//
// EditField stores some state related to editing a string of
// text with a cursor/caret via keyboard or mouse input. It also
// stores some state about a selected range inside the text.
//
// EditField is capable of drawing the text string, selected
// region and the cursor/caret, but it does not keep a pointer
// to the text. Instead, the text and its containing rectangle
// must always be passed as parameters.
//
// The total length of the text string is saved by drawSelf(),
// so they should always be kept in synch. When a char is removed
// by a key command the cursor position and text length are
// updated so you should update the string to match.
//
// handleSpecialKey() will reposition the cursor accordingly
// and will return an EditCommand for keys that are not handled
// directly here, like TAB and PAGE-UP/DOWN. ENTER/RETURN and
// ESCAPE will return the DoneEditing command.
//
// Each VarDisplayWidget has an EditField and they are also added
// to a linked list in the WindowWidget for ease of access.
//
class EditField NTB_FINAL_CLASS
    : public ListNode
{
public:

    // Cursor bar draw once every this many milliseconds.
    static const Int64 CursorBlinkIntervalMs = 500;

    //
    // Most fields use a very short integer range, so we ca use Int16s to save some space.
    // Each UI Variable has an EditField, so to scale, we can have a nice gain there.
    //
    Int64     cursorBlinkTimeMs;  // Keeps track of time to switch the cursor draw on and off each frame
    Rectangle cursorRect;         // Rect of the cursor indicator. Updated by updateCursorPos()
    Rectangle prevCursorRect;     // Text selection requires one level of cursor history
    Rectangle selectionRect;      // Rect to draw the current text selection (if any)
    Int16     textLength;         // Updated by drawSelf()
    Int16     selectionStart;     // Char where a text selection starts (zero-based)
    Int16     selectionEnd;       // Where it ends
    Int16     prevSelectionStart; // Used to estimate the selection direction when selecting via mouse
    Int16     prevSelectionEnd;   // Ditto
    Int16     cursorPos;          // Position within the line for input
    Int16     prevCursorPos;      // Used during text selection navigation
    UInt16    selectionDir;       // Set to LeftArrow or RightArrow to indicate selection direction
    bool      isActive;           // When active the cursor and selection are drawn
    bool      isInInsertMode;     // User hit the [INSERT] key (cursor draws as a full char overlay)
    bool      shouldDrawCursor;   // "Ping Pong" flag to switch cursor draw between frames
    bool      endKeySel;          // Set when [SHIFT]+[END]  is hit to select all the way to the end
    bool      homeKeySel;         // Set when [SHIFT]+[HOME] is hit to select all the way to the beginning
//TODO could probably shave another 4 bytes by replacing the bools with bitflags

    EditField();
    void reset();

    bool hasTextSelection() const;
    void clearSelection();

    void setActive(bool trueIfActive);
    void setDrawCursor(bool trueIfShouldDraw);

    void drawSelf(GeometryBatch & geoBatch, Rectangle displayBox, const SmallStr & text, Color32 textColor);
    EditCommand::Enum handleSpecialKey(const Rectangle & displayBox, KeyCode key, KeyModFlags modifiers);

    void updateCursorPos(const Rectangle & displayBox, Point pos);
    void updateSelection(const Rectangle & displayBox, Point pos);
    void charInserted(const Rectangle & displayBox);

    void saveCursorPos();
    void restoreCursorPos();

    void moveCursorRight(const Rectangle & displayBox);
    void moveCursorLeft(const Rectangle & displayBox);
    void moveCursorHome(const Rectangle & displayBox);
    void moveCursorEnd(const Rectangle & displayBox);
    Rectangle moveCursor(const Rectangle & displayBox, Float32 newPos);
};

// ========================================================
// class VarDisplayWidget:
// ========================================================

class VarDisplayWidget
    : public Widget, public ButtonEventListener
{
public:

    VarDisplayWidget(GUI * myGUI, WindowWidget * myWindow, VarDisplayWidget * myParent, const char * name);
    virtual ~VarDisplayWidget();

    // Widget methods:
    virtual void onDraw(GeometryBatch & geoBatch) const NTB_OVERRIDE;
    virtual void onMove(int displacementX, int displacementY) NTB_OVERRIDE;
    virtual void onResize(int displacementX, int displacementY, Corner corner) NTB_OVERRIDE;
    virtual void onAdjustLayout() NTB_OVERRIDE;

    virtual void onDisableEditing() NTB_OVERRIDE {  } // sine we have EditFields, do nothing. only Windows should handle it
    virtual void setVisible(bool visible) NTB_OVERRIDE;

    virtual bool onMouseButton(MouseButton::Enum button, int clicks) NTB_OVERRIDE;
    virtual bool onMouseMotion(int mx, int my) NTB_OVERRIDE;
    virtual bool onMouseScroll(int yScroll) NTB_OVERRIDE;
    virtual bool onKeyPressed(KeyCode key, KeyModFlags modifiers) NTB_OVERRIDE;

    // ButtonEventListener methods:
    virtual bool onButtonDown(ButtonWidget & button) NTB_OVERRIDE;

    bool hasExpandCollapseButton() const { return expandCollapseButton.getIcon() != ButtonWidget::None; }
    void addExpandCollapseButton();

    void setCustomTextColor(const Color32 newColor) { customTextColor = newColor; }
    Color32 getCustomTextColor() const { return customTextColor; }

    const Rectangle & getDataDisplayRect() const { return dataDisplayRect; }
    void setDataDisplayRect(const Rectangle & newRect) { dataDisplayRect = newRect; }

    const WindowWidget * getParentWindow() const { return &parentWindow; }
    WindowWidget * getParentWindow() { return &parentWindow; }

    const char * getVarName() const { return varName.c_str(); }

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const NTB_OVERRIDE;
    #endif // NEO_TWEAK_BAR_DEBUG

protected:

    bool isHierarchyCollapsed() const
    {
        return hasExpandCollapseButton() && (expandCollapseButton.getState() == false);
    }

    void drawVarName(GeometryBatch & geoBatch) const;

    bool hasValueEditButtons() const;
    void enableValueEditButtons(bool enable);
    void drawValueEditButtons(GeometryBatch & geoBatch) const;

    void setUpVarValueDisplay(Panel & owner, SmallStr  & value);
    void setUpVarValueDisplay(Panel & owner, NumberEx  & value);
    void setUpVarValueDisplay(Panel & owner, BoolEx    & value);
    void setUpVarValueDisplay(Panel & owner, ColorEx   & value);
    void setUpVarValueDisplay(Panel & owner, Float4Ex  & value);
    void setUpVarValueDisplay(Panel & owner, EnumValEx & value);

    void drawVarValue(GeometryBatch & geoBatch, const SmallStr  & value) const;
    void drawVarValue(GeometryBatch & geoBatch, const NumberEx  & value) const;
    void drawVarValue(GeometryBatch & geoBatch, const BoolEx    & value) const;
    void drawVarValue(GeometryBatch & geoBatch, const ColorEx   & value) const;
    void drawVarValue(GeometryBatch & geoBatch, const Float4Ex  & value) const;
    void drawVarValue(GeometryBatch & geoBatch, const EnumValEx & value) const;

    virtual bool onKeyEdit(char inputChar, int inputPosition, EditCommand::Enum cmd); // returns true if the edit is valid
    virtual void onValueIncremented();
    virtual void onValueDecremented();
    virtual void onOpenValueEditPopup();

private:

    static void setHierarchyVisibility(VarDisplayWidget * child, bool visible);
    void setExpandCollapseState(bool expanded);

    int getMinDataDisplayRectWidth() const;
    Rectangle makeDataDisplayAndButtonRects(bool editButtons);
    Rectangle makeExpandCollapseButtonRect() const;

    // Need the extra reference to the parent window because the
    // 'parent' field of a VarDisplayWidget might be another
    // VarDisplayWidget, for nested var instances.
    WindowWidget & parentWindow;

    Rectangle incrButton;
    Rectangle decrButton;
    Rectangle editPopupButton;

    // Box where the variable value is displayed.
    // Text is clipped to fit this rect.
    Rectangle dataDisplayRect;

    // Button state true if hierarchy open, false if collapsed.
    // NOTE that the button will be made a child of the parent widget (Window),
    // so the VarDisplayWidget.children list only containers nested child VarDisplayWidgets.
    ButtonWidget expandCollapseButton;

    // Mutable because EditField::drawSelf() updates some internal state.
    mutable EditField editField;

    Color32 customTextColor;

    //TODO note: might combine these into the Widget::Flags in the future...
    bool withValueEditBtns;
    bool valueEditBtnsEnabled;
    bool valueClickAndHold;

    // Name displayed in the UI.
    // Can contain any ASCII character, including spaces.
    const SmallStr varName;
};

// ========================================================
// class WindowWidget:
// ========================================================

class WindowWidget
    : public Widget
{
public:

    WindowWidget(GUI * myGUI, Widget * myParent, const Rectangle & myRect, const char * title);
    virtual ~WindowWidget();

    // Widget methods:
    virtual void onDraw(GeometryBatch & geoBatch) const NTB_OVERRIDE;
    virtual void onMove(int displacementX, int displacementY) NTB_OVERRIDE;
    virtual void onAdjustLayout() NTB_OVERRIDE;

    virtual bool onMouseButton(MouseButton::Enum button, int clicks) NTB_OVERRIDE;
    virtual bool onMouseMotion(int mx, int my) NTB_OVERRIDE;
    virtual bool onMouseScroll(int yScroll) NTB_OVERRIDE;
    virtual bool onKeyPressed(KeyCode key, KeyModFlags modifiers) NTB_OVERRIDE;
    virtual void setMouseIntersecting(bool intersect) NTB_OVERRIDE;

    // deactivates current active one (if any!)
    virtual void onDisableEditing() NTB_OVERRIDE;

    const Rectangle & getUsableRect() const { return usableRect; }
    void setUsableRect(const Rectangle & newRect) { usableRect = newRect; }

    ScrollBarWidget & getScrollBar() { return scrollBar; }
    IntrusiveList & getEditFieldList() { return editFields; }

    // these ARE scaled!
    int getMinWindowWidth() const;
    int getMinWindowHeight() const;

    #if NEO_TWEAK_BAR_DEBUG
    SmallStr getTypeString() const NTB_OVERRIDE { return "WindowWidget"; }
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    void drawResizeHandles(GeometryBatch & geoBatch) const;
    void resizeWithMin(Corner corner, int & x, int & y, int offsetX, int offsetY);
    void refreshBarRects(const char * newTitle, const char * newInfoString);
    void refreshUsableRect();

    Rectangle       usableRect;     // Size discounts the top/side bars.
    Corner          resizingCorner; // Which corner being resized by user input.
    Widget *        popupWidget;    // Each window can have one open popup at a time. Popups are always window children.
    IntrusiveList   editFields;     // Global list of EditFields attached to this window for easy access.

    // These are always present in a window, so we
    // can avoid a malloc and declare them inline.
    ScrollBarWidget scrollBar;
    TitleBarWidget  titleBar;
    InfoBarWidget   infoBar;
};

} // namespace ntb {}

#endif // NEO_TWEAK_BAR_WIDGETS_HPP
