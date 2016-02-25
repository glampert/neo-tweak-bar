
#include "neo_tweak_bar.hpp"

//FIXME tidy up, this is temporary!
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#include <vector> // Temp

namespace ntb
{

//TODO temporary! This is a GUI parameter.
static const float g_uiScale = 1.3f;
static const float g_textScaling = 0.6f;

#define NTB_SCALED_BY(val, scale) static_cast<int>(static_cast<float>(val) * (scale))
#define NTB_SCALED(val) NTB_SCALED_BY(val, g_uiScale)

// ========================================================

static void drawCheckerboard(GeometryBatch & geoBatch, const Rectangle & rect,
                             const detail::ColorEx & bgColor, const Color32 outlineColor,
                             const int checkerSize)
{
    static const float black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    static const float white[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    const float alpha = bgColor.rgbaF4.data[3];
    const Color32 colors[] =
    {
        blendColors(black, bgColor.rgbaF4.data, alpha),
        blendColors(white, bgColor.rgbaF4.data, alpha)
    };

    int c = 0;
    int xMins = rect.xMins;
    int yMins = rect.yMins;
    int xMaxs = xMins + checkerSize;
    int yMaxs = yMins + checkerSize;

    while (xMaxs <= rect.xMaxs)
    {
        // Top row:
        geoBatch.drawRectFilled(makeRect(xMins, yMins, xMaxs, yMaxs), colors[c++ & 1]);

        // Bottom row:
        geoBatch.drawRectFilled(makeRect(xMins, yMins + checkerSize, xMaxs, yMaxs + checkerSize), colors[c & 1]);

        // Next square.
        xMins += checkerSize;
        xMaxs += checkerSize;
    }

    // Rectangle with didn't divide evenly by the size of our checker squares.
    // Fill the remaining gap with two extra partial squares.
    if ((rect.getWidth() % checkerSize) != 0)
    {
        xMaxs = rect.xMaxs;
        geoBatch.drawRectFilled(makeRect(xMins, yMins, xMaxs, yMaxs), colors[c++ & 1]);
        geoBatch.drawRectFilled(makeRect(xMins, yMins + checkerSize, xMaxs, yMaxs + checkerSize), colors[c & 1]);
    }

    // Border outline:
    geoBatch.drawRectOutline(rect, outlineColor);
}

static void drawCheckMark(GeometryBatch & geoBatch, const Rectangle & rect,
                          const Color32 color, const Color32 outlineBoxColor)
{
    // Optionally this can draw an outline box around the check mark.
    if (outlineBoxColor != 0)
    {
        geoBatch.drawRectOutline(rect, outlineBoxColor);
    }

    // Invariants for all triangles:
    static const UInt16 indexes[6] = { 0, 1, 2, 2, 1, 3 };
    VertexPTC verts[4];
    verts[0].u = 0.0f;
    verts[0].v = 0.0f;
    verts[0].color = color;
    verts[1].u = 0.0f;
    verts[1].v = 0.0f;
    verts[1].color = color;
    verts[2].u = 0.0f;
    verts[2].v = 0.0f;
    verts[2].color = color;
    verts[3].u = 0.0f;
    verts[3].v = 0.0f;
    verts[3].color = color;

    // Offsets are arbitrary. Decided via visual testing.
    const int halfW = rect.getWidth() / 2;
    const int offset1 = NTB_SCALED(2);
    const int offset2 = NTB_SCALED(3);
    const int offset3 = NTB_SCALED(6);
    const int offset4 = NTB_SCALED(1);
    const int offset5 = NTB_SCALED(4);

    // Large leg of the check mark to the right:
    verts[0].x = rect.xMaxs - offset1;
    verts[0].y = rect.yMins + offset4;
    verts[1].x = rect.xMins + halfW - offset1;
    verts[1].y = rect.yMaxs - offset1;
    verts[2].x = rect.xMaxs;
    verts[2].y = rect.yMins + offset2;
    verts[3].x = rect.xMins + halfW;
    verts[3].y = rect.yMaxs;
    geoBatch.draw2DTriangles(verts, lengthOf(verts), indexes, lengthOf(indexes));

    // Small leg to the left:
    verts[0].x = rect.xMins;
    verts[0].y = rect.yMins + offset3;
    verts[1].x = rect.xMins + halfW - offset1;
    verts[1].y = rect.yMaxs - offset1;
    verts[2].x = rect.xMins + offset1;
    verts[2].y = rect.yMins + offset5;
    verts[3].x = rect.xMins + halfW;
    verts[3].y = rect.yMaxs - offset5;
    geoBatch.draw2DTriangles(verts, lengthOf(verts), indexes, lengthOf(indexes));

    const Color32 borderColorBase = packColor(0, 0, 0);
    const Color32 borderColorTopSides = darkenRGB(color, 50);

    // Add a border to the check mark's base:
    geoBatch.drawLine(verts[0].x, verts[0].y, rect.xMins + halfW, rect.yMaxs, borderColorBase);
    geoBatch.drawLine(rect.xMins + halfW, rect.yMaxs, rect.xMaxs, rect.yMins + offset2, borderColorBase);

    // Top:
    verts[3].x -= NTB_SCALED(0.5);
    verts[3].y -= NTB_SCALED(0.5);
    geoBatch.drawLine(verts[2].x, verts[2].y, verts[3].x, verts[3].y, borderColorTopSides);
    geoBatch.drawLine(verts[3].x, verts[3].y, rect.xMaxs - offset1, rect.yMins + offset4, borderColorTopSides);

    // And sides:
    geoBatch.drawLine(verts[0].x, verts[0].y, verts[2].x, verts[2].y, borderColorTopSides);                               // left
    geoBatch.drawLine(rect.xMaxs - offset1, rect.yMins + offset4, rect.xMaxs, rect.yMins + offset2, borderColorTopSides); // right
}

static void drawPlusSignLines(GeometryBatch & geoBatch,
                              const int xMins, const int xMaxs,
                              const int yMins, const int yMaxs,
                              const int midX,  const int midY,
                              const Color32 lineColor, const Color32 shadeColor)
{
    const int lineOffset = NTB_SCALED(2);
    // [+] shade
    geoBatch.drawLine(xMins + lineOffset, midY + 1, xMaxs - lineOffset, midY + 1, shadeColor); // horizontal
    geoBatch.drawLine(midX + 1, yMins + lineOffset, midX + 1, yMaxs - lineOffset, shadeColor); // vertical
    // [+] lines
    geoBatch.drawLine(xMins + lineOffset, midY, xMaxs - lineOffset, midY, lineColor); // horizontal
    geoBatch.drawLine(midX, yMins + lineOffset, midX, yMaxs - lineOffset, lineColor); // vertical
}

static void drawMinusSignLines(GeometryBatch & geoBatch,
                               const int xMins, const int xMaxs, const int midY,
                               const Color32 lineColor, const Color32 shadeColor)
{
    const int lineOffset = NTB_SCALED(2);
    geoBatch.drawLine(xMins + lineOffset, midY + 1, xMaxs - lineOffset, midY + 1, shadeColor); // [-] shade
    geoBatch.drawLine(xMins + lineOffset, midY,     xMaxs - lineOffset, midY,     lineColor);  // [-] line
}

static void drawUpArrowLines(GeometryBatch & geoBatch,
                             const int xMins, const int xMaxs,
                             const int yMins, const int yMaxs,
                             const int midX,  const int midY,
                             const Color32 lineColor, const Color32 shadeColor)
{
    const int lineOffset = NTB_SCALED(2);
    // [/\] shade
    geoBatch.drawLine(midX + 1, yMins + lineOffset + 1, xMins + lineOffset + 1, yMaxs - lineOffset, shadeColor);
    geoBatch.drawLine(midX + 1, yMins + lineOffset,     xMaxs - lineOffset + 1, yMaxs - lineOffset, shadeColor);
    // [/\] lines
    geoBatch.drawLine(midX, yMins + lineOffset, xMins + lineOffset, yMaxs - lineOffset, lineColor);
    geoBatch.drawLine(midX, yMins + lineOffset, xMaxs - lineOffset, yMaxs - lineOffset, lineColor);
}

static void drawDownArrowLines(GeometryBatch & geoBatch,
                               const int xMins, const int xMaxs,
                               const int yMins, const int yMaxs,
                               const int midX,  const int midY,
                               const Color32 lineColor, const Color32 shadeColor)
{
    const int lineOffset = NTB_SCALED(2);
    // [\/] shade
    geoBatch.drawLine(xMins + lineOffset + 1, yMins + lineOffset, midX + 1, yMaxs - lineOffset, shadeColor);
    geoBatch.drawLine(xMaxs - lineOffset + 1, yMins + lineOffset, midX,     yMaxs - lineOffset, shadeColor);
    // [\/] lines
    geoBatch.drawLine(xMins + lineOffset, yMins + lineOffset, midX,     yMaxs - lineOffset, lineColor);
    geoBatch.drawLine(xMaxs - lineOffset, yMins + lineOffset, midX - 1, yMaxs - lineOffset, lineColor);
}

static void drawLeftRightLines(GeometryBatch & geoBatch,
                               const int xMins, const int xMaxs,
                               const int yMins, const int yMaxs,
                               const int midX,  const int midY,
                               const Color32 lineColor, const Color32 shadeColor)
{
    const int offset1 = NTB_SCALED(1);
    const int offset2 = NTB_SCALED(3);

    // [<] shade
    geoBatch.drawLine(xMins + offset1, midY + 1, midX, yMins + offset2 + 1, shadeColor);
    geoBatch.drawLine(xMins + offset1, midY + 1, midX, yMaxs - offset2 + 1, shadeColor);
    // [>] shade
    geoBatch.drawLine(xMaxs - offset1, midY + 1, midX, yMins + offset2 + 1, shadeColor);
    geoBatch.drawLine(xMaxs - offset1, midY + 1, midX, yMaxs - offset2 + 1, shadeColor);

    // [<] lines
    geoBatch.drawLine(xMins + offset1, midY, midX, yMins + offset2, lineColor);
    geoBatch.drawLine(xMins + offset1, midY, midX, yMaxs - offset2, lineColor);
    // [>] lines
    geoBatch.drawLine(xMaxs - offset1, midY, midX, yMins + offset2, lineColor);
    geoBatch.drawLine(xMaxs - offset1, midY, midX, yMaxs - offset2, lineColor);
}

static void drawQuestionMarkLines(GeometryBatch & geoBatch,
                                  const int xMins, const int xMaxs,
                                  const int yMins, const int yMaxs,
                                  const int midX,  const int midY,
                                  const Color32 lineColor, const Color32 shadeColor)
{
    // These values are pretty much arbitrary, chosen via trial-and-error.
    const int offset1 = NTB_SCALED(1);
    const int offset2 = NTB_SCALED(2);
    const int offset3 = NTB_SCALED(3);

    // [?] handle, unshaded
    geoBatch.drawLine(xMins + offset3, yMins + offset2, xMaxs - offset3, yMins + offset2, lineColor);
    geoBatch.drawLine(xMaxs - offset3, yMins + offset2, xMaxs - offset3, midY, lineColor);
    geoBatch.drawLine(xMaxs - offset3, midY, midX - offset1, midY, lineColor);
    geoBatch.drawLine(midX  - offset1, midY, midX - offset1, yMaxs - offset3, lineColor);

    // The dot at the base
    geoBatch.drawLine(midX - offset1, yMaxs - offset3 + offset1, midX - offset1, yMaxs - offset1, lineColor);
}

//--------------------------------------------------------------------------------------------

static bool leftClick(const MouseButton::Enum button, const int clicks)
{
    return clicks > 0 && button == MouseButton::Left;
}

/* NOT IN USE
static bool leftClickRelease(const MouseButton::Enum button, const int clicks)
{
    return clicks <= 0 && button == MouseButton::Left;
}

static bool rightClick(const MouseButton::Enum button, const int clicks)
{
    return clicks > 0 && button == MouseButton::Right;
}

static bool rightClickRelease(const MouseButton::Enum button, const int clicks)
{
    return clicks <= 0 && button == MouseButton::Right;
}
*/

// ========================================================
// class Widget:
// ========================================================

Widget::Widget()
    : gui(NTB_NULL)
    , parent(NTB_NULL)
    , colors(NTB_NULL)
    , children(sizeof(Widget *))
    , flags(0)
{
    setFlag(FlagVisible, true);
    lastMousePos.setZero();
    rect.setZero();
}

Widget::Widget(GUI * myGUI, Widget * myParent, const Rectangle & myRect)
    : gui(myGUI)
    , parent(myParent)
    , colors(NTB_NULL)
    , children(sizeof(Widget *))
    , rect(myRect)
    , flags(0)
{
    NTB_ASSERT(gui != NTB_NULL);
    setNormalColors();

    setFlag(FlagVisible, true);
    lastMousePos.setZero();
}

Widget::~Widget()
{
    // No-op. Just here to anchor the vtable.
}

void Widget::onDraw(GeometryBatch & geoBatch) const
{
    // Self draw:
    drawSelf(geoBatch);

    // Now draw the children widgets on top:
    drawChildren(geoBatch);
}

void Widget::drawSelf(GeometryBatch & geoBatch) const
{
    if (!isVisible())
    {
        return;
    }

    const ColorScheme & myColors = getColors();

    // Optional drop shadow effect under the element.
    if (myColors.shadow.dark != 0 && myColors.shadow.offset != 0)
    {
        geoBatch.drawRectShadow(rect, myColors.shadow.dark,
                                myColors.shadow.light, myColors.shadow.offset);
    }

    // Body box:
    geoBatch.drawRectFilled(rect, myColors.box.bgTopLeft, myColors.box.bgBottomLeft,
                            myColors.box.bgTopRight, myColors.box.bgBottomRight);

    // Box outline/border:
    geoBatch.drawRectOutline(rect, myColors.box.outlineLeft, myColors.box.outlineBottom,
                             myColors.box.outlineRight, myColors.box.outlineTop);
}

void Widget::drawChildren(GeometryBatch & geoBatch) const
{
    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        getChild(c)->onDraw(geoBatch);
    }
}

bool Widget::onMouseButton(const MouseButton::Enum button, const int clicks)
{
    // Obviously, hidden elements should not normally respond to mouse clicks.
    if (!isVisible())
    {
        return false;
    }

    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        if (getChild(c)->onMouseButton(button, clicks))
        {
            return true;
        }
    }

    // If the cursor is intersecting this element or any
    // of its children, we consume the mouse click, even
    // if it has no input effect in the UI.
    return isMouseIntersecting();
}

bool Widget::onMouseMotion(const int mx, const int my)
{
    // First, handle mouse drag:
    if (isMouseDragEnabled())
    {
        onMove(mx - lastMousePos.x, my - lastMousePos.y);
    }

    // Propagate the event to its children,
    // since they might overlap the parent.
    bool intersectingChildWidget = false;
    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        intersectingChildWidget |= getChild(c)->onMouseMotion(mx, my);
    }

    // Even if it intersected a child element, we want
    // to notify the parent as well in case its rect
    // falls under the mouse cursor too.

    if (rect.containsPoint(mx, my))
    {
        setHighlightedColors();
        setMouseIntersecting(true);
    }
    else
    {
        setNormalColors();
        setMouseIntersecting(false);
    }

    // Remember the mouse pointer position so we can
    // compute the displacements for mouse drag.
    lastMousePos.x = mx;
    lastMousePos.y = my;
    return isMouseIntersecting() | intersectingChildWidget;
}

bool Widget::onMouseScroll(int)
{
    // No scroll event handling by default.
    // Only the scroll bars / sliders use this.
    return false;
}

void Widget::onResize(int, int, Corner)
{
    // Widget is NOT resizeable by default.
    // Resizeable UI elements have to override this method.
}

void Widget::onMove(const int displacementX, const int displacementY)
{
    // Displacement may be positive or negative.
    rect.moveBy(displacementX, displacementY);
}

void Widget::onScrollContentUp()
{
    // Implemented by scroll bars/var widgets.
}

void Widget::onScrollContentDown()
{
    // Implemented by scroll bars/var widgets.
}

void Widget::onAdjustLayout()
{
    // Nothing done here at this level, just a default placeholder.
}

void Widget::onDisableEditing()
{
    if (parent != NTB_NULL)
    {
        parent->onDisableEditing();
    }
}

void Widget::setMouseDragEnabled(const bool enable)
{
    setFlag(FlagMouseDragEnabled, enable);

    // Child elements move with the parent.
    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        getChild(c)->setMouseDragEnabled(enable);
    }
}

void Widget::addChild(Widget * newChild)
{
    NTB_ASSERT(newChild != NTB_NULL);
    children.pushBack<Widget *>(newChild);
}

void Widget::setNormalColors()
{
    colors = getGUI()->getNormalColors();
}

void Widget::setHighlightedColors()
{
    colors = getGUI()->getHighlightedColors();
}

float Widget::getTextScaling() const
{
    return g_textScaling;
}

float Widget::getScaling() const
{
    return g_uiScale;
}

int Widget::uiScaled(const int val) const
{
    return uiScaleBy(val, g_uiScale);
}

int Widget::uiScaleBy(const int val, const float scale) const
{
    return static_cast<int>(static_cast<float>(val) * scale);
}

#if NEO_TWEAK_BAR_DEBUG
void Widget::printHierarchy(std::ostream & out, const SmallStr & indent) const
{
    out << indent.getCString() << getTypeString().getCString() << "\n";
    out << "|";

    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        SmallStr nextLevel(indent);
        nextLevel += "---";

        getChild(c)->printHierarchy(out, nextLevel);
    }
}
#endif // NEO_TWEAK_BAR_DEBUG

// ========================================================
// class ButtonEventListener:
// ========================================================

bool ButtonEventListener::onButtonDown(ButtonWidget &)
{
    return false; // Button event ignored.
}

ButtonEventListener::~ButtonEventListener()
{
    // Defined here to anchor the vtable. Do not remove.
}

// ========================================================
// class ButtonWidget:
// ========================================================

ButtonWidget::ButtonWidget()
    : eventListener(NTB_NULL)
    , icon(None)
    , state(false)
{
}

ButtonWidget::ButtonWidget(GUI * myGUI, Widget * myParent, const Rectangle & myRect,
                           const Icon myIcon, ButtonEventListener * listener)
    : Widget(myGUI, myParent, myRect)
    , eventListener(listener)
    , icon(myIcon)
    , state(false)
{
}

void ButtonWidget::construct(GUI * myGUI, Widget * myParent, const Rectangle & myRect,
                             const Icon myIcon, ButtonEventListener * listener)
{
    setGUI(myGUI);
    setRect(myRect);
    setParent(myParent);
    setNormalColors();
    setEventListener(listener);
    setIcon(myIcon);
    state = false;
}

void ButtonWidget::onDraw(GeometryBatch & geoBatch) const
{
    // Nothing to display?
    if (icon == None || !isVisible())
    {
        return;
    }

    // A check box fully overrides the widget drawing logic.
    if (isCheckBoxButton())
    {
        Rectangle checkRect = rect;
        checkRect.xMaxs = checkRect.xMins + NTB_SCALED(12);
        checkRect.yMaxs = checkRect.yMins + NTB_SCALED(12);
        drawCheckMark(geoBatch, checkRect, packColor(0, 255, 0), packColor(255, 255, 255));
        return;
    }

    // Draw the box background and outline, if any.
    Widget::onDraw(geoBatch);

    // Shorthand variables:
    const int xMins = rect.xMins;
    const int xMaxs = rect.xMaxs;
    const int yMins = rect.yMins;
    const int yMaxs = rect.yMaxs;
    const int midX  = xMins + (rect.getWidth()  / 2);
    const int midY  = yMins + (rect.getHeight() / 2);
    const Color32 lineColor  = packColor(255, 255, 255); //TODO could be configurable...
    const Color32 shadeColor = packColor(0, 0, 0);

    // Button icon in the center of its rectangle:
    switch (icon)
    {
    case Plus :
        drawPlusSignLines(geoBatch, xMins, xMaxs, yMins, yMaxs, midX, midY, lineColor, shadeColor);
        break;
    case Minus :
        drawMinusSignLines(geoBatch, xMins, xMaxs, midY, lineColor, shadeColor);
        break;
    case UpArrow :
        drawUpArrowLines(geoBatch, xMins, xMaxs, yMins, yMaxs, midX, midY, lineColor, shadeColor);
        break;
    case DownArrow :
        drawDownArrowLines(geoBatch, xMins, xMaxs, yMins, yMaxs, midX, midY, lineColor, shadeColor);
        break;
    case LeftRight :
        drawLeftRightLines(geoBatch, xMins, xMaxs, yMins, yMaxs, midX, midY, lineColor, shadeColor);
        break;
    case Question :
        drawQuestionMarkLines(geoBatch, xMins, xMaxs, yMins, yMaxs, midX, midY, lineColor, shadeColor);
        break;
    default :
        NTB_ERROR("Bad icon enum in ButtonWidget!");
        break;
    } // switch (icon)

    // Shaded wedges in the outer-right side of the button box:
    geoBatch.drawLine(xMaxs + 1, yMins, xMaxs + 1, yMaxs + 1, shadeColor);
    geoBatch.drawLine(xMins, yMaxs + 1, xMaxs + 1, yMaxs + 1, shadeColor);
}

bool ButtonWidget::onMouseButton(const MouseButton::Enum button, const int clicks)
{
    if (icon != None && isVisible() && isMouseIntersecting())
    {
        if (leftClick(button, clicks))
        {
            // Always toggle the button state.
            state = !state;

            // Fire the event if we have a listener.
            if (hasEventListener())
            {
                return eventListener->onButtonDown(*this);
            }
        }
    }
    return isMouseIntersecting();
}

// ========================================================
// class TitleBarWidget:
// ========================================================

static const int TitleBarHeight = NTB_SCALED(18);
static const int buttonSize = NTB_SCALED(10);

TitleBarWidget::TitleBarWidget(GUI * myGUI, Widget * myParent, const Rectangle & myRect,
                               const char * title, const bool minimizeButton, const bool maximizeButton,
                               const int buttonOffsX, const int buttonOffsY)
    : Widget(myGUI, myParent, myRect)
    , titleText(title)
{
    buttonSetup(minimizeButton, maximizeButton, buttonOffsX, buttonOffsY);
}

void TitleBarWidget::construct(GUI * myGUI, Widget * myParent, const Rectangle & myRect,
                               const char * title, const bool minimizeButton, const bool maximizeButton,
                               const int buttonOffsX, const int buttonOffsY)
{
    setGUI(myGUI);
    setRect(myRect);
    setParent(myParent);
    setNormalColors();
    titleText.setCString(title);
    buttonSetup(minimizeButton, maximizeButton, buttonOffsX, buttonOffsY);
}

void TitleBarWidget::buttonSetup(const bool minimizeButton, const bool maximizeButton,
                                 const int buttonOffsX, const int buttonOffsY)
{
    Rectangle btnRect =
    {
        rect.xMins + buttonOffsX,
        rect.yMins + buttonOffsY,
        rect.xMins + buttonOffsX + buttonSize,
        rect.yMins + buttonOffsY + buttonSize
    };

    if (minimizeButton)
    {
        buttons[BtnMinimize].construct(getGUI(), this, btnRect, ButtonWidget::Minus, this);
        addChild(&buttons[BtnMinimize]);
    }
    if (maximizeButton)
    {
        btnRect.xMins += buttonSize + NTB_SCALED(8);
        btnRect.xMaxs += buttonSize + NTB_SCALED(8);
        buttons[BtnMaximize].construct(getGUI(), this, btnRect, ButtonWidget::UpArrow, this);
        addChild(&buttons[BtnMaximize]);
    }
}

void TitleBarWidget::onDraw(GeometryBatch & geoBatch) const
{
    Widget::onDraw(geoBatch);

    if (titleText.isEmpty() || !isVisible())
    {
        return;
    }

    Rectangle textBox = rect;
    textBox.moveBy(0, NTB_SCALED(3)); // Prevent from touching the upper border.

    Rectangle clipBox = textBox;
    if (buttons[BtnMinimize].getIcon() != ButtonWidget::None)
    {
        clipBox.xMins = buttons[BtnMinimize].getRect().xMaxs + NTB_SCALED(4);
    }
    if (buttons[BtnMaximize].getIcon() != ButtonWidget::None)
    {
        clipBox.xMins = buttons[BtnMaximize].getRect().xMaxs + NTB_SCALED(4);
    }

    // Clipped text string:
    geoBatch.drawTextConstrained(titleText.getCString(), titleText.getLength(), textBox, clipBox,
                                 g_textScaling, getColors().text.normal, TextAlign::Center);
}

bool TitleBarWidget::onMouseButton(const MouseButton::Enum button, const int clicks)
{
    if (!isVisible())
    {
        return false;
    }

    // A child button handled it first.
    if (buttons[BtnMinimize].onMouseButton(button, clicks) ||
        buttons[BtnMaximize].onMouseButton(button, clicks))
    {
        return true;
    }

    // If the mouse is currently over the title bar...
    if (isMouseIntersecting())
    {
        NTB_ASSERT(parent != NTB_NULL);

        // And we have a left click, enable window dragging.
        if (leftClick(button, clicks))
        {
            parent->setMouseDragEnabled(true);
        }
        else
        {
            parent->setMouseDragEnabled(false);
        }

        return true;
    }

    // Click didn't interact with the bar.
    return false;
}

void TitleBarWidget::onResize(const int displacementX, const int displacementY, const Corner corner)
{
    // Title bar doesn't change height.
    switch (corner)
    {
    case TopLeft :
        rect.xMins += displacementX;
        rect.yMins += displacementY;
        rect.yMaxs = rect.yMins + TitleBarHeight;
        buttons[BtnMinimize].onMove(displacementX, displacementY);
        buttons[BtnMaximize].onMove(displacementX, displacementY);
        break;

    case BottomLeft :
        rect.xMins += displacementX;
        buttons[BtnMinimize].onMove(displacementX, 0);
        buttons[BtnMaximize].onMove(displacementX, 0);
        break;

    case TopRight :
        rect.xMaxs += displacementX;
        rect.yMins += displacementY;
        rect.yMaxs = rect.yMins + TitleBarHeight;
        buttons[BtnMinimize].onMove(0, displacementY);
        buttons[BtnMaximize].onMove(0, displacementY);
        break;

    case BottomRight :
        rect.xMaxs += displacementX;
        break;

    default :
        NTB_ERROR("Bad corner enum in TitleBarWidget!");
        break;
    } // switch (corner)
}

void TitleBarWidget::onMove(const int displacementX, const int displacementY)
{
    Widget::onMove(displacementX, displacementY);

//    buttons[BtnMinimize].onMove(displacementX, displacementY);
//    buttons[BtnMaximize].onMove(displacementX, displacementY);
}

bool TitleBarWidget::onButtonDown(ButtonWidget & button)
{
    if (&buttons[BtnMinimize] == &button)
    {
        printf("CLICK MINIMIZE BTN TitleBarWidget\n");
        return true;
    }
    if (&buttons[BtnMaximize] == &button)
    {
        printf("CLICK MAXIMIZE BTN TitleBarWidget\n");
        return true;
    }
    return false;
}

// ========================================================
// class InfoBarWidget:
// ========================================================

static const int InfoBarHeight = NTB_SCALED(18);

InfoBarWidget::InfoBarWidget(GUI * myGUI, Widget * myParent, const Rectangle & myRect, const char * myText)
    : Widget(myGUI, myParent, myRect)
    , infoText(myText)
{
}

void InfoBarWidget::construct(GUI * myGUI, Widget * myParent, const Rectangle & myRect, const char * myText)
{
    setGUI(myGUI);
    setRect(myRect);
    setParent(myParent);
    setNormalColors();
    infoText.setCString(myText);
}

void InfoBarWidget::onDraw(GeometryBatch & geoBatch) const
{
    Widget::onDraw(geoBatch);

    if (infoText.isEmpty() || !isVisible())
    {
        return;
    }

    Rectangle textBox = rect.shrunk(NTB_SCALED(2), 0); // Slightly offset the text so that it doesn't touch the borders.
    textBox.moveBy(0, NTB_SCALED(3));                  // Prevent from touching the upper border.

    geoBatch.drawTextConstrained(infoText.getCString(), infoText.getLength(), textBox, textBox,
                                 g_textScaling, getColors().text.informational, TextAlign::Left);
}

void InfoBarWidget::onResize(const int displacementX, const int displacementY, const Corner corner)
{
    // Info bar doesn't change height.
    switch (corner)
    {
    case TopLeft :
        rect.xMins += displacementX;
        break;

    case BottomLeft :
        rect.xMins += displacementX;
        rect.yMins += displacementY;
        rect.yMaxs = rect.yMins + InfoBarHeight;
        break;

    case TopRight :
        rect.xMaxs += displacementX;
        break;

    case BottomRight :
        rect.xMaxs += displacementX;
        rect.yMins += displacementY;
        rect.yMaxs = rect.yMins + InfoBarHeight;
        break;

    default :
        NTB_ERROR("Bad corner enum in InfoBarWidget!");
        break;
    } // switch (corner)
}

// ========================================================
// class ScrollBarWidget:
// ========================================================

//
//TODO:
// [HOME] and [END] keys should scroll all the way to the top and bottom!!!
//
static const int ScrollBarWidth = NTB_SCALED(18);
static const int ScrollBarButtonSize = NTB_SCALED(10);

ScrollBarWidget::ScrollBarWidget()
    : scrollBarOffsetY(0)
    , scrollBarDisplacement(0)
    , scrollBarSizeFactor(0)
    , scrollBarThickness(0)
    , scrollStartY(0)
    , scrollEndY(0)
    , accumulatedScrollSliderDrag(0)
    , totalLines(0)
    , linesOutOfView(0)
    , linesScrolledOut(0)
    , holdingScrollSlider(false)
    , invertMouseScroll(false)
{
    upBtnRect.setZero();
    downBtnRect.setZero();
    barSliderRect.setZero();
    sliderClickInitialPos.setZero();
}

ScrollBarWidget::ScrollBarWidget(GUI * myGUI, Widget * myParent, const Rectangle & myRect)
    : Widget(myGUI, myParent, myRect)
    , accumulatedScrollSliderDrag(0)
    , totalLines(0)
    , linesOutOfView(0)
    , linesScrolledOut(0)
    , holdingScrollSlider(false)
    , invertMouseScroll(false)
{
    upBtnRect.setZero();
    downBtnRect.setZero();
    barSliderRect.setZero();
    sliderClickInitialPos.setZero();
    onAdjustLayout();
}

void ScrollBarWidget::construct(GUI * myGUI, Widget * myParent, const Rectangle & myRect)
{
    setGUI(myGUI);
    setRect(myRect);
    setParent(myParent);
    setNormalColors();
    onAdjustLayout();
}

void ScrollBarWidget::onDraw(GeometryBatch & geoBatch) const
{
    if (!isVisible())
    {
        return;
    }

    // No child elements attached.
    drawSelf(geoBatch);

    // Window contents are not scrollable. Don't draw a bar slider or buttons.
    if (scrollBarSizeFactor <= 0)
    {
        return;
    }

    const ColorScheme & myColors = getColors();

    // Center lines taking the whole length of the scroll bar slider:
    const int lineX = rect.xMins + (rect.getWidth() / 2);
    geoBatch.drawLine(lineX - 1, scrollStartY, lineX - 1, scrollEndY, packColor(50, 50, 50)); //TODO custom colors
    geoBatch.drawLine(lineX,     scrollStartY, lineX,     scrollEndY, packColor(80, 80, 80));
    geoBatch.drawLine(lineX + 1, scrollStartY, lineX + 1, scrollEndY, packColor(50, 50, 50));

    // Bar body/slider (50% lighter than the background):
    geoBatch.drawRectFilled(barSliderRect, lighthenRGB(myColors.box.bgTopLeft, 50), lighthenRGB(myColors.box.bgBottomLeft, 50),
                            lighthenRGB(myColors.box.bgTopRight, 50), lighthenRGB(myColors.box.bgBottomRight, 50));

    // Bar outline/border (50% darker than the background's):
    geoBatch.drawRectOutline(barSliderRect, darkenRGB(myColors.box.outlineLeft, 50), darkenRGB(myColors.box.outlineBottom, 50),
                             darkenRGB(myColors.box.outlineRight, 50), darkenRGB(myColors.box.outlineTop, 50));

    // Up and down arrow buttons:
    geoBatch.drawArrowFilled(upBtnRect, lighthenRGB(myColors.box.bgTopLeft, 80), darkenRGB(myColors.box.outlineTop, 80), 1);
    geoBatch.drawArrowFilled(downBtnRect, lighthenRGB(myColors.box.bgBottomLeft, 80), darkenRGB(myColors.box.outlineBottom, 80), -1);
}

bool ScrollBarWidget::onMouseButton(const MouseButton::Enum button, const int clicks)
{
    if (!isVisible())
    {
        return false;
    }

    holdingScrollSlider = false;

    if ((scrollBarSizeFactor > 0) && isMouseIntersecting() && leftClick(button, clicks))
    {
        if (barSliderRect.containsPoint(lastMousePos)) // Click, hold and move the slider.
        {
            sliderClickInitialPos = lastMousePos;
            holdingScrollSlider = true;
        }
        else if (upBtnRect.containsPoint(lastMousePos)) // Scroll up (-Y)
        {
            doScrollUp();
        }
        else if (downBtnRect.containsPoint(lastMousePos)) // Scroll down (+Y)
        {
            doScrollDown();
        }
    }

    return isMouseIntersecting();
}

bool ScrollBarWidget::onMouseMotion(const int mx, const int my)
{
    bool eventHandled = Widget::onMouseMotion(mx, my);

    if (holdingScrollSlider)
    {
        // Lower threshold the scroll bar slider moves faster, but less precise.
        // Higher value makes it much "harder" to move, but gains precision.
        static const int threshold = 200;

        accumulatedScrollSliderDrag += my - sliderClickInitialPos.y;
        if (accumulatedScrollSliderDrag < -threshold)
        {
            doScrollUp();
            accumulatedScrollSliderDrag = 0;
        }
        else if (accumulatedScrollSliderDrag > threshold)
        {
            doScrollDown();
            accumulatedScrollSliderDrag = 0;
        }

        eventHandled = true;
    }
    else
    {
        accumulatedScrollSliderDrag = 0;
    }

    return eventHandled;
}

bool ScrollBarWidget::onMouseScroll(const int yScroll)
{
    if (scrollBarSizeFactor <= 0)
    {
        return false; // No scrolling enabled.
    }

    if (yScroll > 0)
    {
        if (invertMouseScroll)
        {
            doScrollDown();
        }
        else
        {
            doScrollUp();
        }
        return true; // Handled the scroll event.
    }

    if (yScroll < 0)
    {
        if (invertMouseScroll)
        {
            doScrollUp();
        }
        else
        {
            doScrollDown();
        }
        return true; // Handled the scroll event.
    }

    return false;
}

void ScrollBarWidget::doScrollUp()
{
    if (parent == NTB_NULL || barSliderRect.yMins <= scrollStartY)
    {
        return;
    }

    parent->onScrollContentUp();

    if ((barSliderRect.yMins - (scrollBarDisplacement * 2)) < scrollStartY) // Don't go out of bounds
    {
        scrollBarOffsetY -= scrollBarDisplacement;
        // Snap to to the beginning of the scroll area.
        scrollBarOffsetY += scrollStartY - (barSliderRect.yMins - scrollBarDisplacement);
    }
    else
    {
        scrollBarOffsetY -= scrollBarDisplacement;
    }

    --linesScrolledOut;
    barSliderRect = makeInnerBarRect();
}

void ScrollBarWidget::doScrollDown()
{
    if (parent == NTB_NULL || barSliderRect.yMaxs >= scrollEndY)
    {
        return;
    }

    parent->onScrollContentDown();

    if ((barSliderRect.yMaxs + (scrollBarDisplacement * 2)) > scrollEndY) // Don't go out of bounds
    {
        scrollBarOffsetY += scrollBarDisplacement;
        // Snap to to the end of the scroll area.
        scrollBarOffsetY -= (barSliderRect.yMaxs + scrollBarDisplacement) - scrollEndY;
    }
    else
    {
        scrollBarOffsetY += scrollBarDisplacement;
    }

    ++linesScrolledOut;
    barSliderRect = makeInnerBarRect();
}

void ScrollBarWidget::onResize(const int displacementX, const int displacementY, const Corner corner)
{
    // Scroll bar doesn't change width.
    switch (corner)
    {
    case TopLeft :
        rect.yMins += displacementY;
        break;

    case BottomLeft :
        rect.yMaxs += displacementY;
        break;

    case TopRight :
        rect.yMins += displacementY;
        rect.xMins += displacementX;
        rect.xMaxs = rect.xMins + ScrollBarWidth;
        break;

    case BottomRight :
        rect.yMaxs += displacementY;
        rect.xMins += displacementX;
        rect.xMaxs = rect.xMins + ScrollBarWidth;
        break;

    default :
        NTB_ERROR("Bad corner enum in ScrollBarWidget!");
        break;
    } // switch (corner)

    onAdjustLayout();
}

void ScrollBarWidget::onAdjustLayout()
{
    if (linesOutOfView > 0)
    {
        // 4 seems to be the magic number here, not quite sure why...
        // If it gets below 4, things start to get, humm, weird...
        if ((totalLines - linesOutOfView) >= 4)
        {
            // map [0,totalLines] to [0,100] range:
            scrollBarSizeFactor = remap(totalLines - linesOutOfView, 0, totalLines, 0, 100);
        }
        else
        {
            scrollBarSizeFactor = remap(4, 0, totalLines, 0, 100);
        }
    }
    else
    {
        scrollBarSizeFactor = 0;
        scrollBarDisplacement = 0;
    }

    scrollBarOffsetY = 0;
    scrollBarThickness = NTB_SCALED_BY(rect.getWidth(), 0.6f) / 2;

    upBtnRect    = makeUpButtonRect();
    downBtnRect  = makeDownButtonRect();
    scrollStartY = upBtnRect.yMaxs   + NTB_SCALED(5);
    scrollEndY   = downBtnRect.yMins - NTB_SCALED(5);

    if (linesOutOfView > 0)
    {
        const int sliderHeight = makeInnerBarRect().getHeight();
        scrollBarDisplacement = (scrollEndY - scrollStartY - sliderHeight) / linesOutOfView;
        scrollBarOffsetY = scrollBarDisplacement * linesScrolledOut;
    }

    // Now that we have the correct scrollBarOffsetY, rebuild the slider box:
    barSliderRect = makeInnerBarRect();
}

void ScrollBarWidget::updateLineScrollState(const int lineCount, const int linesOut)
{
    totalLines = lineCount;
    linesOutOfView = linesOut;
    onAdjustLayout();
}

void ScrollBarWidget::onMove(const int displacementX, const int displacementY)
{
    Widget::onMove(displacementX, displacementY);

    upBtnRect.moveBy(displacementX, displacementY);
    downBtnRect.moveBy(displacementX, displacementY);
    barSliderRect.moveBy(displacementX, displacementY);

    scrollStartY = upBtnRect.yMaxs + NTB_SCALED(5);
    scrollEndY = downBtnRect.yMins - NTB_SCALED(5);
}

Rectangle ScrollBarWidget::makeInnerBarRect() const
{
    const int xMins  = rect.xMins + scrollBarThickness;
    const int xMaxs  = rect.xMaxs - scrollBarThickness;
    const int yMins  = scrollStartY + scrollBarOffsetY;
    const int height = scrollEndY - scrollStartY;

    int yMaxs = yMins + NTB_SCALED_BY(height, scrollBarSizeFactor * 0.01f); // map [0,100] to [0,1] range
    if (yMaxs <= yMins)                                                     // So that it doesn't get too small.
    {
        yMaxs = yMins + NTB_SCALED(4);
    }

    return makeRect(xMins, yMins, xMaxs, yMaxs);
}

Rectangle ScrollBarWidget::makeUpButtonRect() const
{
    const int topOffset = NTB_SCALED(2);
    const int xMins = rect.xMins + scrollBarThickness;
    const int xMaxs = rect.xMaxs - scrollBarThickness;
    const int yMins = rect.yMins + topOffset;
    const int yMaxs = yMins + ScrollBarButtonSize;

    return makeRect(xMins, yMins, xMaxs, yMaxs);
}

Rectangle ScrollBarWidget::makeDownButtonRect() const
{
    const int bottomOffset = NTB_SCALED(18);
    const int xMins = rect.xMins + scrollBarThickness;
    const int xMaxs = rect.xMaxs - scrollBarThickness;
    const int yMins = rect.yMaxs - ScrollBarButtonSize - bottomOffset;
    const int yMaxs = yMins + ScrollBarButtonSize;

    return makeRect(xMins, yMins, xMaxs, yMaxs);
}

// ========================================================
// class ValueSliderWidget:
// ========================================================

//static const int ValueSliderWidth  = NTB_SCALED(190);
//static const int ValueSliderHeight = NTB_SCALED(30);
static const int NumSliderTicks = 10;

ValueSliderWidget::ValueSliderWidget(GUI * myGUI, Widget * myParent, const Rectangle & myRect)
    : Widget(myGUI, myParent, myRect)
{
    Rectangle buttonRect0;
    buttonRect0.xMins = rect.xMins + NTB_SCALED(4);
    buttonRect0.yMins = rect.yMins + (rect.getHeight() / 2) - NTB_SCALED(5); //btn_size/2
    buttonRect0.xMaxs = buttonRect0.xMins + NTB_SCALED(10);
    buttonRect0.yMaxs = buttonRect0.yMins + NTB_SCALED(10); //FIXME better use a buttonSize constant!

    Rectangle buttonRect1;
    buttonRect1.xMins = rect.xMaxs - NTB_SCALED(10 + 4);                          //btn_size+offset
    buttonRect1.yMins = rect.yMins + (rect.getHeight() / 2) - NTB_SCALED(10 / 2); //btn_size/2
    buttonRect1.xMaxs = buttonRect1.xMins + NTB_SCALED(10);
    buttonRect1.yMaxs = buttonRect1.yMins + NTB_SCALED(10); //FIXME better use a buttonSize constant!

    // Bar box:
    const int barHeight = NTB_SCALED(4);
    barRect.xMins = buttonRect0.xMaxs + NTB_SCALED(8);
    barRect.yMins = buttonRect0.yMins + (buttonRect0.getHeight() / 2) - (barHeight / 2);
    barRect.xMaxs = buttonRect1.xMins - NTB_SCALED(8);
    barRect.yMaxs = barRect.yMins + barHeight;

    // Without this check, we get an infinite loop if scale < 1 :(
    // Not running this adjustment won't ensure the tick marks
    // align with the end of the bar.
    if (g_uiScale >= 1.0f)
    {
        while ((barRect.getWidth() % (NumSliderTicks - 1)) != 0)
        {
            barRect.xMins += NTB_SCALED(1);
            barRect.xMaxs -= NTB_SCALED(1);
        }
    }

    // Initial position of the slider:
    //FIXME this depends on the initial value of the slider!
    sliderRect.xMins = barRect.xMins;
    sliderRect.yMins = buttonRect0.yMins - NTB_SCALED(4);
    sliderRect.xMaxs = sliderRect.xMins  + NTB_SCALED(6);
    sliderRect.yMaxs = buttonRect0.yMaxs + NTB_SCALED(4);

    buttons[BtnMinus].construct(myGUI, this, buttonRect0, ButtonWidget::Minus, this); // Left button
    buttons[BtnPlus ].construct(myGUI, this, buttonRect1, ButtonWidget::Plus,  this); // Right button
    addChild(&buttons[BtnMinus]);
    addChild(&buttons[BtnPlus]);
}

void ValueSliderWidget::onDraw(GeometryBatch & geoBatch) const
{
    if (!isVisible())
    {
        return;
    }

    const Color32 black = packColor(0, 0, 0);
    const Color32 gray1 = packColor(160, 160, 160);
    const Color32 gray2 = packColor(128, 128, 128);

    // [-][+] are drawn automatically by Widget, since they are children of this element.
    Widget::onDraw(geoBatch);

    // Slider bar:
    geoBatch.drawRectFilled(barRect, packColor(255, 255, 255)); // TODO configurable colors!
    geoBatch.drawRectOutline(barRect, gray1, black, black, gray2);

    // Draw 10 vertical lines over the bar to better
    // indicate the percentage moved by the slider.
    {
        const int y0 = buttons[BtnMinus].getRect().yMins;
        const int y1 = buttons[BtnMinus].getRect().yMaxs + NTB_SCALED(1); // looks better with this +1 in small sizes, not sure why...
        const int step = barRect.getWidth() / (NumSliderTicks - 1);

        int x = barRect.xMins;
        for (int i = 0; i < NumSliderTicks; ++i)
        {
            geoBatch.drawLine(x, y0, x, y1, black);
            x += step;
        }
    }

    // Slider button:
    geoBatch.drawRectFilled(sliderRect, packColor(0, 255, 0)); //COLORS!
    geoBatch.drawRectOutline(sliderRect, gray1, black, black, gray2);
}

void ValueSliderWidget::onMove(const int displacementX, const int displacementY)
{
    Widget::onMove(displacementX, displacementY);

    barRect.moveBy(displacementX, displacementY);
    sliderRect.moveBy(displacementX, displacementY);

//    buttons[BtnMinus].onMove(displacementX, displacementY);
//    buttons[BtnPlus].onMove(displacementX, displacementY);
}

bool ValueSliderWidget::onButtonDown(ButtonWidget & button)
{
    if (&buttons[BtnMinus] == &button)
    {
        printf("CLICK MINUS BTN ValueSliderWidget\n");
        return true;
    }
    if (&buttons[BtnPlus] == &button)
    {
        printf("CLICK PLUS BTN ValueSliderWidget\n");
        return true;
    }
    return false;
}

// ========================================================
// Built-in table with named colors for the Color Picker:
// ========================================================

#ifndef NEO_TWEAK_BAR_SORT_COLORTABLE
    #define NEO_TWEAK_BAR_SORT_COLORTABLE 1
#endif // NEO_TWEAK_BAR_SORT_COLORTABLE

struct NamedColor
{
    const char * name;
    Color32 value;
};

//
// The 140 standard HTML colors from here:
// http://www.w3schools.com/html/html_colorvalues.asp
// Format: 0xAARRGGBB
//
static NamedColor g_colorTable[] =
{
    { "AliceBlue",            0xFFF0F8FF },
    { "AntiqueWhite",         0xFFFAEBD7 },
    { "Aquamarine",           0xFF7FFFD4 },
    { "Azure",                0xFFF0FFFF },
    { "Beige",                0xFFF5F5DC },
    { "Bisque",               0xFFFFE4C4 },
    { "Black",                0xFF000000 },
    { "BlanchedAlmond",       0xFFFFEBCD },
    { "Blue",                 0xFF0000FF },
    { "BlueViolet",           0xFF8A2BE2 },
    { "Brown",                0xFFA52A2A },
    { "BurlyWood",            0xFFDEB887 },
    { "CadetBlue",            0xFF5F9EA0 },
    { "Chartreuse",           0xFF7FFF00 },
    { "Chocolate",            0xFFD2691E },
    { "Coral",                0xFFFF7F50 },
    { "CornflowerBlue",       0xFF6495ED },
    { "Cornsilk",             0xFFFFF8DC },
    { "Crimson",              0xFFDC143C },
    { "Cyan",                 0xFF00FFFF },
    { "DarkBlue",             0xFF00008B },
    { "DarkCyan",             0xFF008B8B },
    { "DarkGoldenRod",        0xFFB8860B },
    { "DarkGray",             0xFFA9A9A9 },
    { "DarkGreen",            0xFF006400 },
    { "DarkKhaki",            0xFFBDB76B },
    { "DarkMagenta",          0xFF8B008B },
    { "DarkOliveGreen",       0xFF556B2F },
    { "DarkOrange",           0xFFFF8C00 },
    { "DarkOrchid",           0xFF9932CC },
    { "DarkRed",              0xFF8B0000 },
    { "DarkSalmon",           0xFFE9967A },
    { "DarkSeaGreen",         0xFF8FBC8F },
    { "DarkSlateBlue",        0xFF483D8B },
    { "DarkSlateGray",        0xFF2F4F4F },
    { "DarkTurquoise",        0xFF00CED1 },
    { "DarkViolet",           0xFF9400D3 },
    { "DeepPink",             0xFFFF1493 },
    { "DeepSkyBlue",          0xFF00BFFF },
    { "DimGray",              0xFF696969 },
    { "DodgerBlue",           0xFF1E90FF },
    { "FireBrick",            0xFFB22222 },
    { "FloralWhite",          0xFFFFFAF0 },
    { "ForestGreen",          0xFF228B22 },
    { "Gainsboro",            0xFFDCDCDC },
    { "GhostWhite",           0xFFF8F8FF },
    { "Gold",                 0xFFFFD700 },
    { "GoldenRod",            0xFFDAA520 },
    { "Gray",                 0xFF808080 },
    { "Green",                0xFF008000 },
    { "GreenYellow",          0xFFADFF2F },
    { "HoneyDew",             0xFFF0FFF0 },
    { "HotPink",              0xFFFF69B4 },
    { "IndianRed",            0xFFCD5C5C },
    { "Indigo",               0xFF4B0082 },
    { "Ivory",                0xFFFFFFF0 },
    { "Khaki",                0xFFF0E68C },
    { "Lavender",             0xFFE6E6FA },
    { "LavenderBlush",        0xFFFFF0F5 },
    { "LawnGreen",            0xFF7CFC00 },
    { "LemonChiffon",         0xFFFFFACD },
    { "LightBlue",            0xFFADD8E6 },
    { "LightCoral",           0xFFF08080 },
    { "LightCyan",            0xFFE0FFFF },
    { "LightGoldenYellow",    0xFFFAFAD2 },
    { "LightGray",            0xFFD3D3D3 },
    { "LightGreen",           0xFF90EE90 },
    { "LightPink",            0xFFFFB6C1 },
    { "LightSalmon",          0xFFFFA07A },
    { "LightSeaGreen",        0xFF20B2AA },
    { "LightSkyBlue",         0xFF87CEFA },
    { "LightSlateGray",       0xFF778899 },
    { "LightSteelBlue",       0xFFB0C4DE },
    { "LightYellow",          0xFFFFFFE0 },
    { "Lime",                 0xFF00FF00 },
    { "LimeGreen",            0xFF32CD32 },
    { "Linen",                0xFFFAF0E6 },
    { "Magenta",              0xFFFF00FF },
    { "Maroon",               0xFF800000 },
    { "MediumAquaMarine",     0xFF66CDAA },
    { "MediumBlue",           0xFF0000CD },
    { "MediumOrchid",         0xFFBA55D3 },
    { "MediumPurple",         0xFF9370DB },
    { "MediumSeaGreen",       0xFF3CB371 },
    { "MediumSlateBlue",      0xFF7B68EE },
    { "MediumSpringGreen",    0xFF00FA9A },
    { "MediumTurquoise",      0xFF48D1CC },
    { "MediumVioletRed",      0xFFC71585 },
    { "MidnightBlue",         0xFF191970 },
    { "MintCream",            0xFFF5FFFA },
    { "MistyRose",            0xFFFFE4E1 },
    { "Moccasin",             0xFFFFE4B5 },
    { "NavajoWhite",          0xFFFFDEAD },
    { "Navy",                 0xFF000080 },
    { "OldLace",              0xFFFDF5E6 },
    { "Olive",                0xFF808000 },
    { "OliveDrab",            0xFF6B8E23 },
    { "Orange",               0xFFFFA500 },
    { "OrangeRed",            0xFFFF4500 },
    { "Orchid",               0xFFDA70D6 },
    { "PaleGoldenRod",        0xFFEEE8AA },
    { "PaleGreen",            0xFF98FB98 },
    { "PaleTurquoise",        0xFFAFEEEE },
    { "PaleVioletRed",        0xFFDB7093 },
    { "PapayaWhip",           0xFFFFEFD5 },
    { "PeachPuff",            0xFFFFDAB9 },
    { "Peru",                 0xFFCD853F },
    { "Pink",                 0xFFFFC0CB },
    { "Plum",                 0xFFDDA0DD },
    { "PowderBlue",           0xFFB0E0E6 },
    { "Purple",               0xFF800080 },
    { "RebeccaPurple",        0xFF663399 },
    { "Red",                  0xFFFF0000 },
    { "RosyBrown",            0xFFBC8F8F },
    { "RoyalBlue",            0xFF4169E1 },
    { "SaddleBrown",          0xFF8B4513 },
    { "Salmon",               0xFFFA8072 },
    { "SandyBrown",           0xFFF4A460 },
    { "SeaGreen",             0xFF2E8B57 },
    { "SeaShell",             0xFFFFF5EE },
    { "Sienna",               0xFFA0522D },
    { "Silver",               0xFFC0C0C0 },
    { "SkyBlue",              0xFF87CEEB },
    { "SlateBlue",            0xFF6A5ACD },
    { "SlateGray",            0xFF708090 },
    { "Snow",                 0xFFFFFAFA },
    { "SpringGreen",          0xFF00FF7F },
    { "SteelBlue",            0xFF4682B4 },
    { "Tan",                  0xFFD2B48C },
    { "Teal",                 0xFF008080 },
    { "Thistle",              0xFFD8BFD8 },
    { "Tomato",               0xFFFF6347 },
    { "Turquoise",            0xFF40E0D0 },
    { "Violet",               0xFFEE82EE },
    { "Wheat",                0xFFF5DEB3 },
    { "White",                0xFFFFFFFF },
    { "WhiteSmoke",           0xFFF5F5F5 },
    { "Yellow",               0xFFFFFF00 },
    { "YellowGreen",          0xFF9ACD32 },
    { "ZeroAlpha",            0x00000000 }
};

// Table can be sorted once on startup to group similar colors.
#if NEO_TWEAK_BAR_SORT_COLORTABLE
static bool g_colorTableSorted = false;
#endif // NEO_TWEAK_BAR_SORT_COLORTABLE

// ========================================================
// class ColorPickerWidget:
// ========================================================

static const int ColorPickerWidth  = NTB_SCALED(155);
static const int ColorPickerHeight = NTB_SCALED(210);

ColorPickerWidget::ColorPickerWidget(GUI * myGUI, Widget * myParent, const int xStart, const int yStart)
    : Widget(myGUI, myParent, makeRect(xStart, yStart, xStart + ColorPickerWidth, yStart + ColorPickerHeight))
{
    Rectangle barRect;

    // Vertical scroll bar (right side):
    barRect.xMins = rect.xMaxs - ScrollBarWidth;
    barRect.yMins = rect.yMins + TitleBarHeight + 1;
    barRect.xMaxs = rect.xMaxs;
    barRect.yMaxs = rect.yMaxs;
    scrollBar.construct(myGUI, this, barRect);

    // Title bar:
    barRect.xMins = rect.xMins;
    barRect.yMins = rect.yMins;
    barRect.xMaxs = rect.xMaxs;
    barRect.yMaxs = rect.yMins + TitleBarHeight;
    titleBar.construct(myGUI, this, barRect, "Color Picker", true, false, NTB_SCALED(4), NTB_SCALED(4));

    addChild(&scrollBar);
    addChild(&titleBar);
    refreshUsableRect();

    // 20 lines total, only 10 fit in the Color Picker window.
    scrollBar.updateLineScrollState(20, 10);
    colorButtonLinesScrolledUp = 0;
    selectedColorIndex = None;

    // The color table colors are initially sorted by
    // name but grouping similar colors together looks
    // better in the window. This is only done once for
    // when the first ColorPickerWidget is created. You
    // can also disable this code completely if you don't
    // care about that.
    #if NEO_TWEAK_BAR_SORT_COLORTABLE
    struct ColorSort
    {
        bool operator()(const NamedColor & a, const NamedColor & b) const
        {
            UByte alpha;
            UByte aR, aG, aB;
            UByte bR, bG, bB;
            unpackColor(a.value, aR, aG, aB, alpha);
            unpackColor(b.value, bR, bG, bB, alpha);

            float aH, aL, aS;
            float bH, bL, bS;
            RGBToHLS(byteToFloat(aR), byteToFloat(aG), byteToFloat(aB), aH, aL, aS);
            RGBToHLS(byteToFloat(bR), byteToFloat(bG), byteToFloat(bB), bH, bL, bS);

            // NOTE: Sorting by Hue is not very accurate, but more or less
            // bunches similar colors together. Combining the other components
            // doesn't provide much better results either.
            return aH > bH;
        }
    };
    if (!g_colorTableSorted)
    {
        ColorSort predicate;
        std::sort(g_colorTable, g_colorTable + lengthOf(g_colorTable), predicate);
        g_colorTableSorted = true;
    }
    #endif // NEO_TWEAK_BAR_SORT_COLORTABLE
}

bool ColorPickerWidget::forEachColorButton(ButtonFunc pFunc, GeometryBatch * pGeoBatch) const
{
    // We have one small box/button for each color in the table.
    const int colorButtonCount  = lengthOf(g_colorTable);
    const int colorButtonWidth  = NTB_SCALED(15);
    const int colorButtonHeight = NTB_SCALED(15);
    const int gapBetweenButtons = NTB_SCALED(4);
    const int maxButtonsPerLine = 7;

    int colorIndex   = colorButtonLinesScrolledUp * maxButtonsPerLine;
    int colorButtonX = usableRect.xMins;
    int colorButtonY = usableRect.yMins;
    int buttonsInCurrLine = 0;

    for (; colorIndex < colorButtonCount; ++colorIndex)
    {
        const Rectangle colorRect = makeRect(
                        colorButtonX,  colorButtonY,
                        colorButtonX + colorButtonWidth,
                        colorButtonY + colorButtonHeight);

        const bool shouldStop = (this->*pFunc)(colorRect, colorIndex, pGeoBatch);
        if (shouldStop)
        {
            return true;
        }

        colorButtonX += colorButtonWidth + gapBetweenButtons;
        ++buttonsInCurrLine;

        if (buttonsInCurrLine == maxButtonsPerLine)
        {
            buttonsInCurrLine = 0;
            colorButtonX = usableRect.xMins;
            colorButtonY += colorButtonHeight + gapBetweenButtons;

            if ((colorButtonY + colorButtonHeight) > usableRect.yMaxs)
            {
                break; // Already filled the ColorPicker window. Stop.
            }
        }
    }

    return false; // Was not interrupted by the callback.
}

bool ColorPickerWidget::drawColorButton(Rectangle colorRect, const int colorIndex, GeometryBatch * pGeoBatch) const
{
    NTB_ASSERT(pGeoBatch != NTB_NULL);
    const ColorScheme & myColors = getColors();

    // Optional drop shadow effect under the color button:
    if (myColors.shadow.dark != 0 && myColors.shadow.offset != 0)
    {
        const int shadowOffset = ((colorIndex != selectedColorIndex) ?
                                   std::max(myColors.shadow.offset - 1, 0) :
                                   myColors.shadow.offset + 2);

        pGeoBatch->drawRectShadow(colorRect, myColors.shadow.dark,
                                  myColors.shadow.light, shadowOffset);
    }

    // The button box:
    if (g_colorTable[colorIndex].value == 0) // ZeroAlpha (null color), draw an [X]:
    {
        if (colorIndex == selectedColorIndex) // Highlight if selected
        {
            colorRect = colorRect.expanded(NTB_SCALED(2), NTB_SCALED(2));
        }

        pGeoBatch->drawRectFilled(colorRect, packColor(0, 0, 0));

        const Color32 outlineColor = packColor(255, 255, 255);
        pGeoBatch->drawLine(colorRect.xMins, colorRect.yMins,
                            colorRect.xMaxs, colorRect.yMaxs,
                            outlineColor);
        pGeoBatch->drawLine(colorRect.xMaxs, colorRect.yMins,
                            colorRect.xMins, colorRect.yMaxs,
                            outlineColor);
        pGeoBatch->drawRectOutline(colorRect, outlineColor);
    }
    else // Opaque color, draw filled:
    {
        if (colorIndex == selectedColorIndex)
        {
            colorRect = colorRect.expanded(NTB_SCALED(2), NTB_SCALED(2));
        }
        pGeoBatch->drawRectFilled(colorRect, g_colorTable[colorIndex].value);
    }

    // Continue till the end or window filled with buttons.
    return false;
}

bool ColorPickerWidget::testColorButtonClick(Rectangle colorRect, const int colorIndex, GeometryBatch * /* pUnused */) const
{
    if (colorRect.containsPoint(lastMousePos))
    {
        selectedColorIndex = colorIndex;
        return true;
    }
    return false; // Continue to the next button.
}

void ColorPickerWidget::onDraw(GeometryBatch & geoBatch) const
{
    Widget::onDraw(geoBatch);
    forEachColorButton(&ColorPickerWidget::drawColorButton, &geoBatch);
}

void ColorPickerWidget::onMove(const int displacementX, const int displacementY)
{
    Widget::onMove(displacementX, displacementY);

//    titleBar.onMove(displacementX, displacementY);
//    scrollBar.onMove(displacementX, displacementY);
    usableRect.moveBy(displacementX, displacementY);
}

bool ColorPickerWidget::onMouseButton(const MouseButton::Enum button, const int clicks)
{
    if (isMouseIntersecting() && leftClick(button, clicks))
    {
        if (forEachColorButton(&ColorPickerWidget::testColorButtonClick, NTB_NULL))
        {
            titleBar.setTitle(g_colorTable[selectedColorIndex].name);
            printf("SELECTED COLOR %s\n", g_colorTable[selectedColorIndex].name);
            return true; // Got a button click.
        }
    }

    return Widget::onMouseButton(button, clicks);
}

bool ColorPickerWidget::onMouseMotion(const int mx, const int my)
{
    // Prevent it from being dragged out the top of the screen:
    int clampedY = my;
    if (isMouseDragEnabled())
    {
        const int displacementY = my - lastMousePos.y;
        if ((rect.yMins + displacementY) < 0)
        {
            clampedY = my - (rect.yMins + displacementY);
        }
    }
    return Widget::onMouseMotion(mx, clampedY);
}

bool ColorPickerWidget::onMouseScroll(const int yScroll)
{
    // Only scroll if the mouse is hovering this window!
    if (scrollBar.isVisible() && isMouseIntersecting())
    {
        return scrollBar.onMouseScroll(yScroll);
    }
    return false;
}

void ColorPickerWidget::setMouseIntersecting(const bool intersect)
{
    Widget::setMouseIntersecting(intersect);

    // We want to highlight the side/top bars when the parent gains focus.
    if (intersect)
    {
        titleBar.setHighlightedColors();
        scrollBar.setHighlightedColors();
    }
}

bool ColorPickerWidget::onButtonDown(ButtonWidget & button)
{
    //TODO handle the close button at the top bar
    return false;
}

void ColorPickerWidget::onScrollContentUp()
{
    --colorButtonLinesScrolledUp;
}

void ColorPickerWidget::onScrollContentDown()
{
    ++colorButtonLinesScrolledUp;
}

void ColorPickerWidget::refreshUsableRect()
{
    const int offset = NTB_SCALED(5);

    usableRect = rect;
    usableRect.xMins += offset;
    usableRect.xMaxs -= scrollBar.getRect().getWidth();
    usableRect.yMins += titleBar.getRect().getHeight() + offset;
    usableRect.yMaxs -= offset;
}

// ========================================================
// Helpers for the 3D view widget:
// ========================================================

struct SphereVert
{
    Vec3 position;
    Color32 color;
};

struct ArrowVert
{
    Vec3 position;
    Vec3 normal;
};

struct BoxVert
{
    Vec3 position;
    Vec3 normal;
    float u, v;
    Color32 color;
};

//TODO these will be merged eventually...
#include "sphere.cpp"
#include "arrow.cpp"

// Box geometry with proper UVs and normals
static void makeTexturedBox(BoxVert vertsOut[24], UInt16 indexesOut[36], const Color32 faceColors[6],
                            const float width, const float height, const float depth)
{
    //
    // -0.5,+0.5 indexed box:
    //
    static const UInt16 boxFaces[][4] =
    {
        { 0, 1, 5, 4 },
        { 4, 5, 6, 7 },
        { 7, 6, 2, 3 },
        { 1, 0, 3, 2 },
        { 1, 2, 6, 5 },
        { 0, 4, 7, 3 }
    };
    static const float boxPositions[][3] =
    {
        { -0.5f, -0.5f, -0.5f },
        { -0.5f, -0.5f,  0.5f },
        {  0.5f, -0.5f,  0.5f },
        {  0.5f, -0.5f, -0.5f },
        { -0.5f,  0.5f, -0.5f },
        { -0.5f,  0.5f,  0.5f },
        {  0.5f,  0.5f,  0.5f },
        {  0.5f,  0.5f, -0.5f },
    };
    static const float boxNormalVectors[][3] =
    {
        { -1.0f,  0.0f,  0.0f },
        {  0.0f,  1.0f,  0.0f },
        {  1.0f,  0.0f,  0.0f },
        {  0.0f, -1.0f,  0.0f },
        {  0.0f,  0.0f,  1.0f },
        {  0.0f,  0.0f, -1.0f }
    };
    static const float boxTexCoords[][2] =
    {
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
        { 1.0f, 0.0f },
        { 0.0f, 0.0f }
    };

    const Color32 * pColor = faceColors;
    BoxVert       * pVert  = vertsOut;
    UInt16        * pFace  = indexesOut;

    // 'i' iterates over the faces, 2 triangles per face:
    UInt16 vertIndex = 0;
    for (int i = 0; i < 6; ++i, vertIndex += 4)
    {
        for (int j = 0; j < 4; ++j, ++pVert)
        {
            pVert->position.x = boxPositions[boxFaces[i][j]][0] * width;
            pVert->position.y = boxPositions[boxFaces[i][j]][1] * height;
            pVert->position.z = boxPositions[boxFaces[i][j]][2] * depth;
            pVert->normal.x = boxNormalVectors[i][0];
            pVert->normal.y = boxNormalVectors[i][1];
            pVert->normal.z = boxNormalVectors[i][2];
            pVert->u = boxTexCoords[j][0];
            pVert->v = boxTexCoords[j][1];
            pVert->color = *pColor;
        }
        ++pColor;
        pFace[0] = vertIndex;
        pFace[1] = vertIndex + 1;
        pFace[2] = vertIndex + 2;
        pFace += 3;
        pFace[0] = vertIndex + 2;
        pFace[1] = vertIndex + 3;
        pFace[2] = vertIndex;
        pFace += 3;
    }
}

static void screenProjectionXY(VertexPTC & vOut, const VertexPTC & vIn, const Mat4x4 & viewProjMatrix, const Rectangle & viewport)
{
    // Project the vertex (we don't care about z/depth here):
    const Mat4x4::Vec4Ptr * M = viewProjMatrix.getRows();
    const float vx = (M[0][0] * vIn.x) + (M[1][0] * vIn.y) + (M[2][0] * vIn.z) + M[3][0];
    const float vy = (M[0][1] * vIn.x) + (M[1][1] * vIn.y) + (M[2][1] * vIn.z) + M[3][1];
    const float vw = (M[0][3] * vIn.x) + (M[1][3] * vIn.y) + (M[2][3] * vIn.z) + M[3][3];

    // Perspective divide:
    const float ndcX = vx / vw;
    const float ndcY = vy / vw;

    // Map to window coordinates:
    vOut.x = (((ndcX * 0.5f) + 0.5f) * viewport.getWidth())  + viewport.getX();
    vOut.y = (((ndcY * 0.5f) + 0.5f) * viewport.getHeight()) + viewport.getY();
}

static float normalizeAngle360(float degrees)
{
    if (degrees >= 360.0f || degrees < 0.0f)
    {
        degrees -= std::floor(degrees * (1.0f / 360.0f)) * 360.0f;
    }
    return degrees;
}

static float normalizeAngle180(float degrees)
{
    degrees = normalizeAngle360(degrees);
    if (degrees > 180.0f)
    {
        degrees -= 360.0f;
    }
    return degrees;
}

static bool angleNearZero(const float num)
{
    return std::fabs(num) <= 0.01f;
}

static float lerpAngles(float a, float b, const float t)
{
    // Ensure we wrap around the shortest way.
    a = normalizeAngle180(a);
    b = normalizeAngle180(b);
	return (a + t * (b - a));
}

// ========================================================
// class View3DWidget:
// ========================================================

View3DWidget::View3DWidget(GUI * myGUI, Widget * myParent, const Rectangle & myRect,
                           const char * title, const ProjectionParameters & proj)
    : Widget(myGUI, myParent, myRect)
    , mouseSensitivity(0.5f)
    , maxMouseDelta(20)
    , invertMouseY(false)
    , leftMouseButtonDown(false)
    , interactiveControls(true)
    , showXyzLabels(true)
    , updateScrGeometry(true)
    , resettingAngles(false)
    , prevFrameTimeMs(0)
    , scrProjectedVerts(sizeof(VertexPTC))
    , scrProjectedIndexes(sizeof(UInt16))
    , projParams(proj)
{
    mouseDelta.setZero();
    rotationDegrees.setZero();

    // Title bar is optional in this widget, so we can also use it as a component attached
    // to a WindowWidget or as a standalone popup-like window when a title/top-bar is provided.
    if (title != NTB_NULL)
    {
        const Rectangle barRect = makeRect(rect.xMins, rect.yMins, rect.xMaxs, rect.yMins + TitleBarHeight);
        titleBar.construct(myGUI, this, barRect, title, true, false, NTB_SCALED(4), NTB_SCALED(4));
    }
    else
    {
        titleBar.construct(myGUI, this, makeRect(0, 0, 0, 0), "", false, false, 0, 0);
        titleBar.setVisible(false);
    }

    addChild(&titleBar);
    refreshProjectionViewport();

    // Need to be cached for the click tests.
    const float chrW = GeometryBatch::getCharWidth()  * g_textScaling;
    const float chrH = GeometryBatch::getCharHeight() * g_textScaling;
    resetAnglesBtnRect.xMins = projParams.viewport.xMins + NTB_SCALED(3);
    resetAnglesBtnRect.yMins = projParams.viewport.yMaxs - NTB_SCALED(2) - chrH;
    resetAnglesBtnRect.xMaxs = resetAnglesBtnRect.xMins + chrW + NTB_SCALED(3);
    resetAnglesBtnRect.yMaxs = resetAnglesBtnRect.yMins + chrH + NTB_SCALED(2);
}

void View3DWidget::onDraw(GeometryBatch & geoBatch) const
{
    const Int64 currentTimeMs = getShellInterface()->getTimeMilliseconds();
    const Int64 deltaTimeMs   = currentTimeMs - prevFrameTimeMs;
    prevFrameTimeMs = currentTimeMs;

    if (resettingAngles)
    {
        const float resetSpeed = 2.0f;
        const float deltaTimeSeconds = deltaTimeMs * 0.001;

        rotationDegrees.x = lerpAngles(rotationDegrees.x, 0.0f, resetSpeed * deltaTimeSeconds);
        rotationDegrees.y = lerpAngles(rotationDegrees.y, 0.0f, resetSpeed * deltaTimeSeconds);
        rotationDegrees.z = lerpAngles(rotationDegrees.z, 0.0f, resetSpeed * deltaTimeSeconds);
        updateScrGeometry = true;

        if (angleNearZero(rotationDegrees.x) &&
            angleNearZero(rotationDegrees.y) &&
            angleNearZero(rotationDegrees.z))
        {
            rotationDegrees.setZero();
            resettingAngles = false;
        }
    }

    if (!isVisible())
    {
        return;
    }

    const Color32 vpOutlineColor = packColor(255, 255, 255);
    const Color32 resetBtnColor  = packColor(255, 255, 225);
    const Color32 xAxisColor     = packColor(225, 0, 0); // X=red
    const Color32 yAxisColor     = packColor(0, 225, 0); // Y=green
    const Color32 zAxisColor     = packColor(0, 0, 225); // Z=blue

    Widget::onDraw(geoBatch);
    geoBatch.drawRectOutline(projParams.viewport, vpOutlineColor);

    if (interactiveControls)
    {
        Rectangle textBox = resetAnglesBtnRect;
        textBox.moveBy(NTB_SCALED(2), NTB_SCALED(2)); // Better center the "R"
        geoBatch.drawTextConstrained("R", 1, textBox, textBox, g_textScaling, resetBtnColor, TextAlign::Left);
    }

    if (showXyzLabels)
    {
        const float chrW = GeometryBatch::getCharWidth()  * g_textScaling;
        const float chrH = GeometryBatch::getCharHeight() * g_textScaling;

        Rectangle textBox;
        textBox.xMins = projParams.viewport.xMaxs - chrW - NTB_SCALED(2);
        textBox.yMins = projParams.viewport.yMaxs - chrH * 3;
        textBox.xMaxs = textBox.xMins + chrW + NTB_SCALED(2);
        textBox.yMaxs = textBox.yMins + chrH * 3;

        geoBatch.drawTextConstrained("x", 1, textBox, textBox, g_textScaling, xAxisColor, TextAlign::Right);
        textBox = textBox.shrunk(0, chrH);
        geoBatch.drawTextConstrained("y", 1, textBox, textBox, g_textScaling, yAxisColor, TextAlign::Right);
        textBox = textBox.shrunk(0, chrH);
        geoBatch.drawTextConstrained("z", 1, textBox, textBox, g_textScaling, zAxisColor, TextAlign::Right);
    }

    if (updateScrGeometry)
    {
        const Mat4x4 matRx = Mat4x4::rotationX(degToRad(rotationDegrees.x));
        const Mat4x4 matRy = Mat4x4::rotationY(degToRad(rotationDegrees.y));
        const Mat4x4 matRz = Mat4x4::rotationZ(degToRad(rotationDegrees.z));
        const Mat4x4 modelToWorldMatrix = Mat4x4::multiply(Mat4x4::multiply(matRz, matRx), matRy);

        clearScreenVertexCaches();

        // SPHERE
        addScreenProjectedArrow(modelToWorldMatrix,  0.28f, xAxisColor, ArrowDirX);
        addScreenProjectedArrow(modelToWorldMatrix,  0.28f, yAxisColor, ArrowDirY);
        addScreenProjectedArrow(modelToWorldMatrix,  0.28f, zAxisColor, ArrowDirZ);
        addScreenProjectedSphere(modelToWorldMatrix, 0.20f);
        addScreenProjectedSphere(modelToWorldMatrix, 0.20f);

        // ARROW
        //addScreenProjectedArrow(modelToWorldMatrix, 0.35f, packColor(255, 255, 0), ArrowDirZ);

        // BOX
        //addScreenProjectedBox(modelToWorldMatrix, 0.4f, 0.4f, 0.4f, packColor(0, 128, 128));

        updateScrGeometry = false;
    }

    submitScreenVertexCaches(geoBatch);
}

void View3DWidget::clearScreenVertexCaches() const
{
    scrProjectedVerts.clear();
    scrProjectedIndexes.clear();
}

void View3DWidget::submitScreenVertexCaches(GeometryBatch & geoBatch) const
{
    geoBatch.drawClipped2DTriangles(
            scrProjectedVerts.getData<VertexPTC>(), scrProjectedVerts.getSize(),
            scrProjectedIndexes.getData<UInt16>(),  scrProjectedIndexes.getSize(),
            projParams.viewport, projParams.viewport);
}

void View3DWidget::addScreenProjectedSphere(const Mat4x4 & modelToWorldMatrix, const float scaleXYZ) const
{
    RenderInterface * renderer  = getRenderInterface();
    const Rectangle scrViewport = renderer->getViewport();
    const bool highlighted      = isMouseIntersecting();
    const Color32 brightness    = highlighted ? packColor(255, 255, 255) : packColor(200, 200, 200);
    const Color32 shadeColor    = packColor(0, 0, 0, 255);
    const SphereVert * pVert    = g_sphereVerts;
    const int vertCount         = lengthOf(g_sphereVerts);
    UInt16 nextVertexIndex      = scrProjectedVerts.getSize();

    for (int v = 0; v < vertCount; ++v, ++pVert)
    {
        const Vec3 wp = Mat4x4::transformPointAffine(pVert->position, modelToWorldMatrix);
        const Color32 vertColor = blendColors(shadeColor, pVert->color & brightness,
                                              std::fabs(clamp(wp.z, -1.0f, 1.0f)));

        VertexPTC scrVert = { wp.x * scaleXYZ, wp.y * scaleXYZ, wp.z * scaleXYZ, 0.0f, 0.0f, vertColor };
        screenProjectionXY(scrVert, scrVert, projParams.viewProjMatrix, scrViewport);

        scrProjectedVerts.pushBack<VertexPTC>(scrVert);
        scrProjectedIndexes.pushBack<UInt16>(nextVertexIndex++);
    }
}

void View3DWidget::addScreenProjectedArrow(const Mat4x4 & modelToWorldMatrix, const float scaleXYZ,
                                           const Color32 color, const ArrowDir dir) const
{
    RenderInterface * renderer  = getRenderInterface();
    const Rectangle scrViewport = renderer->getViewport();
    const bool highlighted      = isMouseIntersecting();
    const Color32 brightness    = highlighted ? packColor(255, 255, 255) : packColor(200, 200, 200);
    const Color32 shadeColor    = packColor(0, 0, 0, 255);
    const ArrowVert * pVert     = g_arrowVerts;
    const int vertCount         = lengthOf(g_arrowVerts);
    UInt16 nextVertexIndex      = scrProjectedVerts.getSize();

    for (int v = 0; v < vertCount; ++v, ++pVert)
    {
        ArrowVert av = *pVert;
        switch (dir)
        {
        case ArrowDirX :
            std::swap(av.position.x, av.position.z);
            std::swap(av.normal.x,   av.normal.z);
            break;
        case ArrowDirY :
            std::swap(av.position.y, av.position.z);
            std::swap(av.normal.y,   av.normal.z);
            break;
        default :
            // ArrowDirZ is the default direction.
            break;
        } // switch (dir)

        const Vec3 wp = Mat4x4::transformPointAffine(av.position, modelToWorldMatrix);
        const Vec3 wn = Mat4x4::transformPointAffine(av.normal,   modelToWorldMatrix);

        const Color32 vertColor = blendColors(shadeColor, color & brightness,
                                              std::fabs(clamp(wn.z, -1.0f, 1.0f)));

        VertexPTC scrVert = { wp.x * scaleXYZ, wp.y * scaleXYZ, wp.z * scaleXYZ, 0.0f, 0.0f, vertColor };
        screenProjectionXY(scrVert, scrVert, projParams.viewProjMatrix, scrViewport);

        scrProjectedVerts.pushBack<VertexPTC>(scrVert);
        scrProjectedIndexes.pushBack<UInt16>(nextVertexIndex++);
    }
}

void View3DWidget::addScreenProjectedBox(const Mat4x4 & modelToWorldMatrix, const float w,
                                         const float h, const float d, const Color32 color) const
{
    BoxVert tempBoxVerts[24];
    UInt16  tempBoxIndexes[36];

    RenderInterface * renderer  = getRenderInterface();
    const Rectangle scrViewport = renderer->getViewport();
    const bool highlighted      = isMouseIntersecting();
    const Color32 brightness    = highlighted ? packColor(255, 255, 255) : packColor(200, 200, 200);
    const Color32 shadeColor    = packColor(0, 0, 0, 255);
    const BoxVert * pVert       = tempBoxVerts;
    const UInt16  * pIndex      = tempBoxIndexes;
    const int vertCount         = lengthOf(tempBoxVerts);
    const int indexCount        = lengthOf(tempBoxIndexes);

    // Each face could be colored independently, but we aren't using that now.
    const Color32 tempFaceColors[6] = { color, color, color, color, color, color };
    makeTexturedBox(tempBoxVerts, tempBoxIndexes, tempFaceColors, w, h, d);

    scrProjectedVerts.allocateExact(vertCount);
    scrProjectedIndexes.allocateExact(indexCount);

    for (int v = 0; v < vertCount; ++v, ++pVert)
    {
        const Vec3 wp = Mat4x4::transformPointAffine(pVert->position, modelToWorldMatrix);
        const Vec3 wn = Mat4x4::transformPointAffine(pVert->normal,   modelToWorldMatrix);

        const Color32 vertColor = blendColors(shadeColor, pVert->color & brightness,
                                              std::fabs(clamp(wn.z, -1.0f, 1.0f)));

        VertexPTC scrVert = { wp.x, wp.y, wp.z, pVert->u, pVert->v, vertColor };
        screenProjectionXY(scrVert, scrVert, projParams.viewProjMatrix, scrViewport);

        scrProjectedVerts.pushBack<VertexPTC>(scrVert);
    }

    // Unlike the others, the box geometry is actually indexed!
    for (int i = 0; i < indexCount; ++i, ++pIndex)
    {
        scrProjectedIndexes.pushBack<UInt16>(*pIndex);
    }
}

void View3DWidget::onMove(const int displacementX, const int displacementY)
{
    Widget::onMove(displacementX, displacementY);

//    titleBar.onMove(displacementX, displacementY);
    resetAnglesBtnRect.moveBy(displacementX, displacementY);
    refreshProjectionViewport();
}

bool View3DWidget::onMouseButton(const MouseButton::Enum button, const int clicks)
{
    const bool eventHandled = Widget::onMouseButton(button, clicks);

    if (interactiveControls && isMouseIntersecting())
    {
        if (leftClick(button, clicks))
        {
            // Clicking the "R" button resets the angles.
            if (resetAnglesBtnRect.containsPoint(lastMousePos))
            {
                resettingAngles   = true;
                updateScrGeometry = true;
            }
            else
            {
                leftMouseButtonDown = true;
            }
        }
        else if (clicks <= 0)
        {
            leftMouseButtonDown = false;
            mouseDelta.setZero();
        }
    }

    return eventHandled | leftMouseButtonDown;
}

bool View3DWidget::onMouseMotion(const int mx, const int my)
{
    mouseDelta.x = mx - lastMousePos.x;
    mouseDelta.y = my - lastMousePos.y;
    mouseDelta.x = clamp(mouseDelta.x, -maxMouseDelta, maxMouseDelta);
    mouseDelta.y = clamp(mouseDelta.y, -maxMouseDelta, maxMouseDelta);

    // Prevent it from being dragged out the top of the screen:
    int clampedY = my;
    if (isMouseDragEnabled())
    {
        const int displacementY = my - lastMousePos.y;
        if ((rect.yMins + displacementY) < 0)
        {
            clampedY = my - (rect.yMins + displacementY);
        }
    }
    bool eventHandled = Widget::onMouseMotion(mx, clampedY);

    // We want to highlight these when the parent window gains focus.
    if (interactiveControls && leftMouseButtonDown &&
        isMouseIntersecting() && projParams.viewport.containsPoint(mx, my))
    {
        const float dirY = invertMouseY ? -1.0f : 1.0f;
        rotationDegrees.x -= mouseDelta.y * mouseSensitivity * dirY;
        rotationDegrees.y += mouseDelta.x * mouseSensitivity;
        rotationDegrees.x = normalizeAngle360(rotationDegrees.x);
        rotationDegrees.y = normalizeAngle360(rotationDegrees.y);

        mouseDelta.setZero();
        resettingAngles   = false;
        updateScrGeometry = true;
        eventHandled      = true;
    }

    return eventHandled;
}

bool View3DWidget::onMouseScroll(const int yScroll)
{
    if (isVisible() && isMouseIntersecting() && interactiveControls && leftMouseButtonDown)
    {
        resettingAngles   = false;
        updateScrGeometry = true;
        rotationDegrees.z = normalizeAngle360(rotationDegrees.z + (yScroll * mouseSensitivity));
        return true;
    }
    return false;
}

void View3DWidget::setMouseIntersecting(const bool intersect)
{
    Widget::setMouseIntersecting(intersect);

    if (intersect)
    {
        titleBar.setHighlightedColors();
    }
    else // If we lost mouse focus just cancel the last button down event.
    {
        leftMouseButtonDown = false;
    }
}

void View3DWidget::refreshProjectionViewport()
{
    const int vpOffset = NTB_SCALED(4);
    const float oldAspectRatio = projParams.viewport.getAspect();

    // Update the viewport rect:
    projParams.viewport = rect;
    projParams.viewport.xMins += vpOffset;
    projParams.viewport.xMaxs -= vpOffset;
    projParams.viewport.yMins += titleBar.getRect().getHeight() + vpOffset;
    projParams.viewport.yMaxs -= vpOffset;

    // Might also have to recompute the projection/view,
    // since the aspect-ratio might have changed.
    if (projParams.autoAdjustAspect && oldAspectRatio != projParams.viewport.getAspect())
    {
        projParams.aspectRatio = projParams.viewport.getAspect();
        const Mat4x4 projMatrix =
            Mat4x4::perspective(projParams.fovYRadians,
                                projParams.aspectRatio,
                                projParams.zNear,
                                projParams.zFar);
        const Mat4x4 viewMatrix =
            Mat4x4::lookAt(makeVec3(0.0f, 0.0f, +1.0f),
                           makeVec3(0.0f, 0.0f, -1.0f),
                           makeVec3(0.0f, 1.0f,  0.0f));
        projParams.viewProjMatrix = Mat4x4::multiply(viewMatrix, projMatrix);
    }
}

// ========================================================
// class ListWidget:
// ========================================================

ListWidget::ListWidget(GUI * myGUI, Widget * myParent, const Rectangle & myRect)
    : Widget(myGUI, myParent, myRect)
    , entries(sizeof(Entry))
    , selectedEntry(None)
    , hoveredEntry(None)
{
}

void ListWidget::onDraw(GeometryBatch & geoBatch) const
{
    Widget::onDraw(geoBatch);

    const ColorScheme & myColors = getColors();
    const int entryCount = entries.getSize();

    //TODO configurable color for this!
    const Color32 btnFillColorNormal     = packColor(80,  80,  80 );
    const Color32 btnFillColorSelected   = packColor(110, 110, 110);
    const Color32 btnOutlineColorNormal  = packColor(0,   0,   0  );
    const Color32 btnOutlineColorHovered = packColor(180, 180, 180);

    for (int e = 0; e < entryCount; ++e)
    {
        const Entry & entry = entries.get<Entry>(e);

        geoBatch.drawRectFilled(entry.rect,  (e == selectedEntry) ? btnFillColorSelected   : btnFillColorNormal);
        geoBatch.drawRectOutline(entry.rect, (e == hoveredEntry)  ? btnOutlineColorHovered : btnOutlineColorNormal);

        Rectangle textBox = entry.rect;
        textBox.moveBy(0, uiScaled(3)); // Prevent from touching the upper border.

        const char * pEntryText = strings.getCString() + entry.firstChar;
        geoBatch.drawTextConstrained(pEntryText, entry.lengthInChars, textBox, textBox,
                                     g_textScaling, myColors.text.alternate, TextAlign::Center);
    }
}

void ListWidget::onMove(const int displacementX, const int displacementY)
{
    Widget::onMove(displacementX, displacementY);

    const int entryCount = entries.getSize();
    for (int e = 0; e < entryCount; ++e)
    {
        Entry & entry = entries.get<Entry>(e);
        entry.rect.moveBy(displacementX, displacementY);
    }
}

bool ListWidget::onMouseButton(const MouseButton::Enum button, const int clicks)
{
    bool eventHandled = Widget::onMouseButton(button, clicks);

    // Find if one of the entries was clicked:
    if (isMouseIntersecting())
    {
        const int index = findEntryForPoint(lastMousePos.x, lastMousePos.y);
        if (index != None)
        {
            selectedEntry = index;
            eventHandled  = true;
        }
    }

    return eventHandled;
}

bool ListWidget::onMouseMotion(const int mx, const int my)
{
    bool eventHandled = Widget::onMouseMotion(mx, my);

    // Check for intersection with the entries to highlight the hovered item:
    if (isMouseIntersecting())
    {
        hoveredEntry = findEntryForPoint(mx, my);
        if (hoveredEntry != None)
        {
            eventHandled = true;
        }
    }
    else
    {
        hoveredEntry = None;
    }

    return eventHandled;
}

int ListWidget::findEntryForPoint(const int x, const int y) const
{
    const int entryCount = entries.getSize();
    for (int e = 0; e < entryCount; ++e)
    {
        const Entry & entry = entries.get<Entry>(e);
        if (entry.rect.containsPoint(x, y))
        {
            return e;
        }
    }
    return None;
}

void ListWidget::allocEntries(const int count)
{
    strings.clear();
    entries.clear();
    entries.resize(count);
    entries.zeroFill();
    selectedEntry = None;
    hoveredEntry  = None;
}

int ListWidget::getNumOfEntries() const
{
    return entries.getSize();
}

void ListWidget::addEntryText(const int index, const char * value)
{
    Entry & entry       = entries.get<Entry>(index);
    entry.firstChar     = strings.getLength();
    entry.lengthInChars = lengthOf(value);

    addEntryRect(index, entry.lengthInChars);
    strings.append(value, entry.lengthInChars);
}

SmallStr ListWidget::getEntryText(const int index) const
{
    const Entry & entry = entries.get<Entry>(index);
    return SmallStr(strings.getCString() + entry.firstChar, entry.lengthInChars);
}

int ListWidget::getSelectedEntry() const
{
    return selectedEntry;
}

bool ListWidget::hasSelectedEntry() const
{
    return selectedEntry != None;
}

void ListWidget::clearSelectedEntry()
{
    selectedEntry = None;
}

void ListWidget::addEntryRect(const int entryIndex, const int entryLengthInChars)
{
    const int spacing = NTB_SCALED(3);
    const int entryHeight = (GeometryBatch::getCharHeight() * g_textScaling) + spacing;
    const int entryWidth  = (GeometryBatch::getCharWidth() * g_textScaling * entryLengthInChars) + (spacing * 2);

    Rectangle newRect = rect.shrunk(spacing, spacing);
    newRect.yMins += (entryHeight + spacing) * entryIndex;
    newRect.yMaxs  = newRect.yMins + entryHeight;
    newRect.xMaxs  = newRect.xMins + entryWidth;
    entries.get<Entry>(entryIndex).rect = newRect;

    // Expand the background window if needed:
    if (newRect.xMaxs > rect.xMaxs)
    {
        rect.xMaxs = newRect.xMaxs + spacing;
    }
    if (newRect.yMaxs > rect.yMaxs)
    {
        rect.yMaxs = newRect.yMaxs + spacing;
    }

    // Adjust the remaining rectangles to match the widest one:
    int e, widest = 0;
    const int entryCount = entries.getSize();
    for (e = 0; e < entryCount; ++e)
    {
        const Entry & entry = entries.get<Entry>(e);
        if (entry.rect.xMaxs > widest)
        {
            widest = entry.rect.xMaxs;
        }
    }
    for (e = 0; e < entryCount; ++e)
    {
        entries.get<Entry>(e).rect.xMaxs = widest;
    }
}

// ========================================================
// class EditField:
// ========================================================

//
//FIXME: Text selection won't scroll with the window contents!!!
//

EditField::EditField()
{
    reset();
}

void EditField::drawSelf(GeometryBatch & geoBatch, Rectangle displayBox, const SmallStr & text)
{
    //TEMP debugging
    /*
    {
        const int x0 = displayBox.xMins + NTB_SCALED(2);
        const int length = text.getLength();

        const float fixedWidth = GeometryBatch::getCharWidth() * g_textScaling;
        float xMins = x0;
        float xMaxs = x0;

        for (int i = 0; i < length; ++i)
        {
            xMaxs += fixedWidth;
            geoBatch.drawRectFilled(makeRect(xMins, displayBox.yMins, xMaxs, displayBox.yMaxs),
                    (i & 1) ? packColor(255, 255, 255, 100) : packColor(0, 0, 255, 100));
            xMins += fixedWidth;
        }
    }
    //*/

    textLength = text.getLength();

    // Border box:
    displayBox = displayBox.shrunk(NTB_SCALED(1), NTB_SCALED(1));
    geoBatch.drawRectFilled(displayBox, isActive() ? packColor(100, 100, 100) : packColor(80, 80, 80));

    // Selected range of characters:
    if (isActive() && hasTextSelection())
    {
        const Color32 selectionColor = packColor(0, 0, 255, 100); //TODO customizable color
        geoBatch.drawRectFilled(selectionRect.shrunk(NTB_SCALED(1), NTB_SCALED(1)), selectionColor);

        /* //DEBUG
        SmallStr selText;
        selText.setCString(text.getCString() + selectionStart, std::min(selectionEnd - selectionStart, text.getLength() - selectionStart));
        Rectangle r = displayBox;
        r.xMins -= 100;
        r.xMaxs -= 100;
        geoBatch.drawTextConstrained(selText.getCString(), selText.getLength(), r, r,
                                     g_textScaling, packColor(255, 255, 255), TextAlign::Left);
        */
    }

    // Further offset the text so that it doesn't touch the borders.
    displayBox = displayBox.shrunk(NTB_SCALED(1), NTB_SCALED(2));
    geoBatch.drawTextConstrained(text.getCString(), text.getLength(), displayBox, displayBox,
                                 g_textScaling, packColor(255, 255, 255), TextAlign::Left);

    // Cursor when active:
    if (isActive())
    {
        // Draw anyways when selecting text. Otherwise when the blink cool-down allows.
        if (hasTextSelection() || cursorBlinkPingPong)
        {
            geoBatch.drawRectFilled(cursorRect.shrunk(NTB_SCALED(1), NTB_SCALED(1)), packColor(0, 255, 0)); //TODO customizable color? same for selection?
        }

        ShellInterface * shell = getShellInterface();
        if (shell->getTimeMilliseconds() >= cursorBlinkTimeMs)
        {
            cursorBlinkTimeMs   = shell->getTimeMilliseconds() + cursorBlinkIntervalMs;
            cursorBlinkPingPong = !cursorBlinkPingPong;
        }
    }
}

void EditField::updateCursorPos(const Rectangle & displayBox, const Point & pos)
{
    const int yMins = displayBox.yMins;
    const int yMaxs = displayBox.yMaxs;
    const float xStart = displayBox.xMins + NTB_SCALED(2);
    const float fixedWidth = GeometryBatch::getCharWidth() * g_textScaling;

    bool  hit   = false;
    float xMins = xStart;
    float xMaxs = xStart;

    // NOTE: We must use floating point here because integers would
    // round incorrectly and the glyph rects would be mispositioned.
    // Since we have to go through the trouble of doing that, we might
    // as well store the generated cursor rectangle for later drawing.
    for (int i = 0; i < textLength; ++i)
    {
        xMaxs += fixedWidth;
        const Rectangle rect = makeRect(xMins, yMins, xMaxs, yMaxs);
        if (rect.containsPoint(pos))
        {
            cursorPos  = i;
            cursorRect = rect;
            hit        = true;
            break;
        }
        xMins += fixedWidth;
    }

    // Not clicking on the text, position the cursor at the end of the current string.
    if (!hit)
    {
        cursorPos = textLength;
        cursorRect.set(xMins, yMins, xMaxs, yMaxs);
    }

    // Cursor thickness is just one point. Adjust it.
    cursorRect.xMaxs = cursorRect.xMins + NTB_SCALED(1);

    // Selection gets reset to the cursor position.
    selectionRect  = cursorRect;
    selectionStart = cursorPos;
    selectionEnd   = cursorPos;
}

void EditField::updateSelection(const Rectangle & displayBox, const Point & pos)
{
    const int yMins = displayBox.yMins;
    const int yMaxs = displayBox.yMaxs;
    const float xStart = displayBox.xMins + NTB_SCALED(2);
    const float fixedWidth = GeometryBatch::getCharWidth() * g_textScaling;

    // Same of what is done above with the cursorRect:
    float xMins = xStart;
    float xMaxs = xStart;
    for (int i = 0; i < textLength; ++i)
    {
        xMaxs += fixedWidth;
        const Rectangle rect = makeRect(xMins, yMins, xMaxs, yMaxs);
        if (rect.containsPoint(pos))
        {
            selectionRect.expandWidth(rect);
            selectionStart = std::min(selectionStart, i);
            selectionEnd   = std::max(selectionEnd, i + 1);
            break;
        }
        xMins += fixedWidth;
    }

    // no cursor while selecting???
    cursorRect.setZero();

//with cursor while selecting:
/*
    // Put the cursor at the end of the selection:
    cursorRect = selectionRect;

    // Selecting from let to right
    if (pos.x > lastSelectionX)
    {
        cursorRect.xMins = cursorRect.xMaxs - NTB_SCALED(1);
    }
    else // Right to left
    {
        cursorRect.xMaxs = cursorRect.xMins + NTB_SCALED(1);
    }
    lastSelectionX = pos.x;
//*/
}

// ========================================================
// class VarDisplayWidget:
// ========================================================

//TODO VarDisplayWidget:
//
// draw the title then a box with the vars value, according to var type.
//
// my var     | 42     |
// foo bar    | 3.14   |
//
// also have to handle nesting of variables / hierarchies
//
// [-] My Color   | <colored box> |
//     R          | 102           |
//     G          | 110           |
//     B          | 120           |
//     A          | 255           |
//

static const int VarDisplayWidgetHeight = NTB_SCALED(16);
static int my_offsetY = NTB_SCALED(8); //FIXME my_offsetY belongs to the panel/WindowWidget!

static Widget * fixVarParent(WindowWidget * window, Widget * parent)
{
    return (parent != NTB_NULL) ? parent : static_cast<Widget *>(window);
}

static Rectangle makeVarRect(WindowWidget * window, Widget * parent)
{
    NTB_ASSERT(window != NTB_NULL);

    const Rectangle windowRect = window->getRect();
    parent = fixVarParent(window, parent);

    Rectangle varRect;
    varRect.xMins = parent->getRect().xMins + buttonSize + NTB_SCALED(6);
    varRect.yMins = windowRect.yMins + TitleBarHeight + my_offsetY;
    varRect.xMaxs = windowRect.xMaxs - NTB_SCALED(22);
    varRect.yMaxs = varRect.yMins    + NTB_SCALED(16); //FIXME replace magic values!

    my_offsetY += NTB_SCALED(16+8);
    return varRect;
}

VarDisplayWidget::VarDisplayWidget(GUI * myGUI, WindowWidget * myWindow, VarDisplayWidget * myParent, const char * name)
    : Widget(myGUI, fixVarParent(myWindow, myParent), makeVarRect(myWindow, myParent))
    , parentWindow(*myWindow)
    , withValueEditBtns(false)
    , valueEditBtnsEnabled(false)
    , valueClickAndHold(false)
    , varName(name)
{
    if (myParent != NTB_NULL)
    {
        // Parent var display widget gets an expand collapse button [+]/[-]
        myParent->addExpandCollapseButton();
        myParent->addChild(this);
    }
    else
    {
        // Child of the window/panel.
        myWindow->addChild(this);
    }

    dataDisplayRect = makeDataDisplayAndButtonRects(false);
    parentWindow.getEditFieldList().pushBack(&editField);
}

VarDisplayWidget::~VarDisplayWidget()
{
    // In case we are being deleted before the WindowWidget.
    parentWindow.getEditFieldList().unlink(&editField);
}

void VarDisplayWidget::addExpandCollapseButton()
{
    if (hasExpandCollapseButton())
    {
        return; // Already has it.
    }

    // Hierarchy initially open:
    const Rectangle btnRect = makeExpandCollapseButtonRect();
    expandCollapseButton.construct(getGUI(), this, btnRect, ButtonWidget::Minus, this);
    expandCollapseButton.setState(true);

    // NOTE: Button is a child of the parent widget (WindowWidget), so that we
    // keep this widget's children list reserved for nested VarDisplayWidgets.
    parentWindow.addChild(&expandCollapseButton);
}

void VarDisplayWidget::onDraw(GeometryBatch & geoBatch) const
{
    drawSelf(geoBatch);
    if (!isHierarchyCollapsed())
    {
        drawChildren(geoBatch);
    }
}

int VarDisplayWidget::getMinDataDisplayRectWidth() const
{
    // Reserve space for about 3 characters...
    return (GeometryBatch::getCharWidth() * 3) * g_textScaling + NTB_SCALED(4);
}

bool VarDisplayWidget::hasValueEditButtons() const
{
    return withValueEditBtns && valueEditBtnsEnabled;
}

void VarDisplayWidget::enableValueEditButtons(const bool enable)
{
    withValueEditBtns    = enable;
    valueEditBtnsEnabled = enable;
}

void VarDisplayWidget::drawValueEditButtons(GeometryBatch & geoBatch) const
{
    if (!hasValueEditButtons())
    {
        return;
    }

    geoBatch.drawRectFilled(incrButton, packColor(0, 0, 255, 128)); //TODO add configurable colors!
    geoBatch.drawRectFilled(decrButton, packColor(0, 255, 0, 128));
    geoBatch.drawRectFilled(editPopupButton, packColor(0, 0, 0, 128));

    const Color32 lineColor  = packColor(255, 255, 255); //TODO could be configurable...
    const Color32 shadeColor = packColor(0, 0, 0);

    drawPlusSignLines(geoBatch, incrButton.xMins, incrButton.xMaxs + 1,
                      incrButton.yMins + NTB_SCALED(3), incrButton.yMaxs - NTB_SCALED(3),
                      incrButton.xMins + (incrButton.getWidth()  / 2) + NTB_SCALED(1),
                      incrButton.yMins + (incrButton.getHeight() / 2), lineColor, shadeColor);

    drawMinusSignLines(geoBatch, decrButton.xMins, decrButton.xMaxs,
                       decrButton.yMins + (decrButton.getHeight() / 2),
                       lineColor, shadeColor);

    const int w = (editPopupButton.getWidth()  / 4) + NTB_SCALED(1);
    const int h = (editPopupButton.getHeight() / 3) + NTB_SCALED(1);
    geoBatch.drawRectFilled(makeRect(editPopupButton.xMins + w,
                                     editPopupButton.yMins + h,
                                     editPopupButton.xMaxs - w,
                                     editPopupButton.yMaxs - h),
                                     lineColor);
}

void VarDisplayWidget::drawVarName(GeometryBatch & geoBatch) const
{
    Rectangle textBox = rect.shrunk(uiScaled(2), 0); // Slightly offset the text so that it doesn't touch the borders.
    textBox.moveBy(0, uiScaled(2));                  // Prevent from touching the upper border.

    geoBatch.drawTextConstrained(varName.getCString(), varName.getLength(), textBox, textBox,
                                 g_textScaling, colors->text.informational, TextAlign::Left);
}

void VarDisplayWidget::setUpVarValueDisplay(Panel &, SmallStr &)
{
    // No extra setup required.
}

void VarDisplayWidget::setUpVarValueDisplay(Panel &, detail::NumberEx &)
{
    enableValueEditButtons(true);
}

void VarDisplayWidget::setUpVarValueDisplay(Panel &, detail::BoolEx &)
{
    // boolean toggles with a click on the value
//    enableValueEditButtons(true);
}

//
//TODO figure out how to handle the switch between float/byte color display formats!
//probably add both vars, but one is always hidden...
//
void VarDisplayWidget::setUpVarValueDisplay(Panel & owner, detail::ColorEx & value)
{
    // Need to force the cast because of the protected inheritance in Variable...
    Variable * parentVar = reinterpret_cast<Variable *>(this);

    // 4 strings each color mode in colorModeChannelNames[]
    const int colorMode = value.colorMode * 4;

    if (parentVar->getAccessMode() == Variable::ReadWrite)
    {
        owner.addNumberRW(parentVar, detail::colorModeChannelNames[colorMode + 0], &value.bR);
        owner.addNumberRW(parentVar, detail::colorModeChannelNames[colorMode + 1], &value.bG);
        owner.addNumberRW(parentVar, detail::colorModeChannelNames[colorMode + 2], &value.bB);
        if (value.numChannels == 4)
        {
            owner.addNumberRW(parentVar, detail::colorModeChannelNames[colorMode + 3], &value.bA);
        }
    }
    else
    {
        owner.addNumberRO(parentVar, detail::colorModeChannelNames[colorMode + 0], &value.bR);
        owner.addNumberRO(parentVar, detail::colorModeChannelNames[colorMode + 1], &value.bG);
        owner.addNumberRO(parentVar, detail::colorModeChannelNames[colorMode + 2], &value.bB);
        if (value.numChannels == 4)
        {
            owner.addNumberRO(parentVar, detail::colorModeChannelNames[colorMode + 3], &value.bA);
        }
    }

    owner.addEnumRW(parentVar, "Mode", &value.colorMode,
                    detail::colorModeEnum, lengthOf(detail::colorModeEnum));

    owner.addEnumRW(parentVar, "Range", &value.displayMode,
                    detail::colorDisplayEnum, lengthOf(detail::colorDisplayEnum));
}

void VarDisplayWidget::setUpVarValueDisplay(Panel & owner, detail::Float4Ex & value)
{
    //TODO
}

void VarDisplayWidget::setUpVarValueDisplay(Panel & owner, detail::EnumValEx & value)
{
    //TODO
}

void VarDisplayWidget::drawVarValue(GeometryBatch & geoBatch, const SmallStr & value) const
{
    const Color32 dataBoxColor = packColor(180, 180, 180); //TODO configurable!

    if (hasValueEditButtons())
    {
        Rectangle fullRect = dataDisplayRect;
        fullRect.xMaxs = rect.xMaxs;
        geoBatch.drawRectFilled(fullRect, dataBoxColor);
    }
    else
    {
        geoBatch.drawRectFilled(dataDisplayRect, dataBoxColor);
    }

    if (!value.isEmpty())
    {
        editField.drawSelf(geoBatch, dataDisplayRect, value);
    }
}

void VarDisplayWidget::drawVarValue(GeometryBatch & geoBatch, const detail::NumberEx & value) const
{
    // Numbers always display as strings.
    drawVarValue(geoBatch, value.toString());
}

void VarDisplayWidget::drawVarValue(GeometryBatch & geoBatch, const detail::BoolEx & value) const
{
    const Color32 dataBoxColor = packColor(180, 180, 180); //TODO configurable!
    geoBatch.drawRectFilled(dataDisplayRect, dataBoxColor);

    if (value.displayMode == detail::BoolEx::String)
    {
        drawVarValue(geoBatch, value.toString());
    }
    else
    {
        Rectangle checkRect;
        checkRect.xMins = dataDisplayRect.xMins + NTB_SCALED(2);
        checkRect.yMins = dataDisplayRect.yMins + NTB_SCALED(2);
        checkRect.xMaxs = checkRect.xMins + NTB_SCALED(12);
        checkRect.yMaxs = checkRect.yMins + NTB_SCALED(12);

        if (value.isSet())
        {
            drawCheckMark(geoBatch, checkRect, packColor(0, 255, 0), 0);
        }
        else // A small dash / minus sign:
        {
            const int halfH = checkRect.getHeight() / 2;
            drawMinusSignLines(geoBatch, checkRect.xMins, checkRect.xMaxs, checkRect.yMins + halfH,
                               packColor(0, 255, 0), packColor(0, 0, 0));
        }
    }
}

void VarDisplayWidget::drawVarValue(GeometryBatch & geoBatch, const detail::ColorEx & value) const
{
    const Color32 outlineColor = packColor(0, 0, 0);//TODO configurable outline color???
    const int checkerSize = NTB_SCALED(8);

    if (value.hasTransparency())
    {
        drawCheckerboard(geoBatch, dataDisplayRect, value, outlineColor, checkerSize);
    }
    else
    {
        Color32 color;
        value.getColor32(color);
        geoBatch.drawRectFilled(dataDisplayRect, color);
        geoBatch.drawRectOutline(dataDisplayRect, outlineColor);
    }
}

void VarDisplayWidget::drawVarValue(GeometryBatch & geoBatch, const detail::Float4Ex & value) const
{
    //TODO temp
    drawVarValue(geoBatch, value.toString());
}

void VarDisplayWidget::drawVarValue(GeometryBatch & geoBatch, const detail::EnumValEx & value) const
{
    SmallStr displayValue;
    const int enumValue  = value.getEnumValue();
    const int constCount = value.getNumOfConsts();

    for (int i = 0; i < constCount; ++i)
    {
        if (value.getConstValue(i) == enumValue)
        {
            displayValue = value.getConstName(i);
            break;
        }
    }

    // Not found? Use the raw numeric value.
    if (displayValue.isEmpty())
    {
        displayValue = SmallStr::fromNumber(static_cast<Int64>(enumValue));
    }

    drawVarValue(geoBatch, displayValue);
}

void VarDisplayWidget::onResize(const int displacementX, const int displacementY, const Corner corner)
{
    switch (corner)
    {
    case TopLeft :
        rect.xMins += displacementX;
        rect.yMins += displacementY;
        rect.yMaxs = rect.yMins + VarDisplayWidgetHeight;
        dataDisplayRect.xMins += displacementX;
        break;

    case BottomLeft :
        rect.xMins += displacementX;
        dataDisplayRect.xMins += displacementX;
        break;

    case TopRight :
        rect.xMaxs += displacementX;
        rect.yMins += displacementY;
        rect.yMaxs = rect.yMins + VarDisplayWidgetHeight;
        break;

    case BottomRight :
        rect.xMaxs += displacementX;
        break;

    default :
        NTB_ERROR("Bad corner enum in VarDisplayWidget!");
        break;
    } // switch (corner)

    // Propagate to the var hierarchy:
    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        getChild(c)->onResize(displacementX, displacementY, corner);
    }
}

void VarDisplayWidget::onMove(const int displacementX, const int displacementY)
{
    Widget::onMove(displacementX, displacementY);

    incrButton.moveBy(displacementX, displacementY);
    decrButton.moveBy(displacementX, displacementY);
    editPopupButton.moveBy(displacementX, displacementY);
    dataDisplayRect.moveBy(displacementX, displacementY);

    /*
    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        getChild(c)->onMove(displacementX, displacementY);
    }
    */
}

bool VarDisplayWidget::onMouseButton(const MouseButton::Enum button, const int clicks)
{
    valueClickAndHold = false;

    if (isVisible() && isMouseIntersecting() && leftClick(button, clicks))
    {
        if (hasValueEditButtons())
        {
            if (incrButton.containsPoint(lastMousePos))
            {
                onValueIncremented();
                return true;
            }
            else if (decrButton.containsPoint(lastMousePos))
            {
                onValueDecremented();
                return true;
            }
            else if (editPopupButton.containsPoint(lastMousePos))
            {
                onOpenValueEditPopup();
                return true;
            }
        }

        if (dataDisplayRect.containsPoint(lastMousePos))
        {
            if (clicks >= 2)
            {
                printf("EDITING CONTENTS - double click\n");
                //TODO select whole text
            }
            else
            {
                printf("EDITING CONTENTS - one click\n");
                // TODO position cursor at the beginning
            }

            valueClickAndHold = true;
            editField.setActive(true);
            editField.updateCursorPos(dataDisplayRect, lastMousePos);

            // Active edit field goes to the front of the list if not already there.
            IntrusiveList & editFieldList = parentWindow.getEditFieldList();
            EditField * pActiveEdit = editFieldList.getFirst<EditField>();

            if (pActiveEdit != &editField)
            {
                editFieldList.unlink(&editField);
                editFieldList.pushFront(&editField);

                if (pActiveEdit != NTB_NULL)
                {
                    pActiveEdit->setActive(false);
                }
            }
            return true;
        }
        else
        {
            parentWindow.onDisableEditing();
        }

        // A click on the parent of a var hierarchy toggles
        // the thing as if clicking the expand/collapse button.
        if (hasExpandCollapseButton())
        {
            const bool state = !expandCollapseButton.getState();
            setExpandCollapseState(state);
            return true;
        }
    }

    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        if (getChild(c)->onMouseButton(button, clicks))
        {
            return true;
        }
    }

    return isMouseIntersecting();
}

bool VarDisplayWidget::onMouseMotion(const int mx, const int my)
{
// interfered with edit fields when the hierarchy parent is scrolled out of view
    /*if (!isVisible())
    {
        return false;
    }*/

    bool eventHandled = Widget::onMouseMotion(mx, my);

    if (valueClickAndHold)
    {
        if (dataDisplayRect.containsPoint(mx, my))
        {
            editField.updateSelection(dataDisplayRect, makePoint(mx, my));
            eventHandled = true;
        }
    }

    return eventHandled;
}

bool VarDisplayWidget::onMouseScroll(const int yScroll)
{
    if (isVisible() && isMouseIntersecting() && valueClickAndHold)
    {
        if (yScroll > 0) // Forward
        {
            onValueIncremented();
            return true; // Handled the scroll event.
        }
        if (yScroll < 0) // Back
        {
            onValueDecremented();
            return true; // Handled the scroll event.
        }
    }

    return false;
}

bool VarDisplayWidget::onButtonDown(ButtonWidget & button)
{
    if (hasExpandCollapseButton() && (&expandCollapseButton == &button))
    {
        const bool state = expandCollapseButton.getState();
        setExpandCollapseState(state);
        return true;
    }

    return false;
}

void VarDisplayWidget::onValueIncremented()
{
    //TODO
    parentWindow.onDisableEditing();
    printf("EDITING CONTENTS - increment value\n");
}

void VarDisplayWidget::onValueDecremented()
{
    //TODO
    parentWindow.onDisableEditing();
    printf("EDITING CONTENTS - decrement value\n");
}

void VarDisplayWidget::onOpenValueEditPopup()
{
    //TODO
    parentWindow.onDisableEditing();
    printf("EDITING CONTENTS - popup edit\n");
}

void VarDisplayWidget::onAdjustLayout()
{
    dataDisplayRect = makeDataDisplayAndButtonRects(valueEditBtnsEnabled);

    // Only if edit buttons are allowed for this variable type.
    if (withValueEditBtns)
    {
        // See if we need to disable the value edit buttons and reset the data display rect:
        if (dataDisplayRect.getWidth() <= getMinDataDisplayRectWidth())
        {
            valueEditBtnsEnabled = false;
            dataDisplayRect = makeDataDisplayAndButtonRects(false);
        }
        // Or if the width is now big enough, restore the buttons:
        else if (!valueEditBtnsEnabled)
        {
            valueEditBtnsEnabled = true;
            const Rectangle newRect = makeDataDisplayAndButtonRects(true);

            // If re-enabling the buttons violates the min width. Don't do it.
            if (newRect.getWidth() <= getMinDataDisplayRectWidth())
            {
                valueEditBtnsEnabled = false;
            }
            else
            {
                dataDisplayRect = newRect;
            }
        }
    }

    // This button is only present on variable hierarchies.
    if (hasExpandCollapseButton())
    {
        expandCollapseButton.setRect(makeExpandCollapseButtonRect());
    }
}

void VarDisplayWidget::setVisible(const bool visible)
{
    Widget::setVisible(visible);
    expandCollapseButton.setVisible(visible);
}

void VarDisplayWidget::setHierarchyVisibility(VarDisplayWidget * child, const bool visible)
{
    NTB_ASSERT(child != NTB_NULL);

    child->setVisible(visible);
    child->setMinimized(!visible);

    const int childCount = child->getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        if (!child->isHierarchyCollapsed())
        {
            setHierarchyVisibility(static_cast<VarDisplayWidget *>(child->getChild(c)), visible);
        }
    }
}

void VarDisplayWidget::setExpandCollapseState(const bool expanded)
{
    // Mark each child as hidden or visible, recursively:
    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        // Children of VarDisplayWidget should only be other VarDisplayWidgets.
        // The button is a child of the parent WindowWidget/Panel.
        setHierarchyVisibility(static_cast<VarDisplayWidget *>(getChild(c)), expanded);
    }

    // Change the icon appropriately:
    expandCollapseButton.setIcon(expanded ? ButtonWidget::Minus : ButtonWidget::Plus);
    expandCollapseButton.setState(expanded);

    // Collapse the hidden variables in the window to fill the gaps.
    parentWindow.onAdjustLayout();
}

Rectangle VarDisplayWidget::makeExpandCollapseButtonRect() const
{
    const int xMins = rect.xMins - buttonSize - NTB_SCALED(4);
    const int yMins = rect.yMins + NTB_SCALED(3);
    const int xMaxs = xMins + buttonSize;
    const int yMaxs = yMins + buttonSize;
    return makeRect(xMins, yMins, xMaxs, yMaxs);
}

Rectangle VarDisplayWidget::makeDataDisplayAndButtonRects(const bool editButtons)
{
    const int buttonWidth = NTB_SCALED(8);
    int xMins = rect.xMins + (rect.getWidth() / 2) + NTB_SCALED(10);
    int yMins = rect.yMins;
    int xMaxs = rect.xMaxs;
    int yMaxs = rect.yMaxs;

    // The offsets are really just one pixel, I didn't forget NTB_SCALED!
    editPopupButton.xMins = xMaxs - buttonWidth;
    editPopupButton.yMins = yMins + 1;
    editPopupButton.xMaxs = xMaxs - 1;
    editPopupButton.yMaxs = yMaxs - 1;

    decrButton = editPopupButton;
    decrButton.xMins -= buttonWidth + 1;
    decrButton.xMaxs -= buttonWidth + 1;

    incrButton = decrButton;
    incrButton.xMins -= buttonWidth + 1;
    incrButton.xMaxs -= buttonWidth + 1;

    if (editButtons)
    {
        const int buttonsWidthTotal = incrButton.getWidth() +
                                      decrButton.getWidth() +
                                      editPopupButton.getWidth();
        xMaxs -= buttonsWidthTotal;
        xMaxs -= NTB_SCALED(4);
    }

    return makeRect(xMins, yMins, xMaxs, yMaxs);
}

#if NEO_TWEAK_BAR_DEBUG
SmallStr VarDisplayWidget::getTypeString() const
{
    SmallStr varStr = "VarDisplayWidget ";
    varStr += "(";
    varStr += getVarName();
    varStr += ")";
    return varStr;
}
#endif // NEO_TWEAK_BAR_DEBUG

// ========================================================
// class WindowWidget:
// ========================================================

WindowWidget::WindowWidget(GUI * myGUI, Widget * myParent, const Rectangle & myRect, const char * title)
    : Widget(myGUI, myParent, myRect)
    , resizingCorner(CornerNone)
    , popupWidget(NTB_NULL)
{
    Rectangle barRect;

    // Vertical scroll bar (right side):
    barRect.xMins = rect.xMaxs - ScrollBarWidth;
    barRect.yMins = rect.yMins + TitleBarHeight + 1;
    barRect.xMaxs = rect.xMaxs;
    barRect.yMaxs = rect.yMaxs;
    scrollBar.construct(myGUI, this, barRect);

    // Title bar:
    barRect.xMins = rect.xMins;
    barRect.yMins = rect.yMins;
    barRect.xMaxs = rect.xMaxs;
    barRect.yMaxs = rect.yMins + TitleBarHeight;
    titleBar.construct(myGUI, this, barRect, title, true, true, NTB_SCALED(20), NTB_SCALED(4));

    // Info bar at the bottom:
    barRect.xMins = rect.xMins + ScrollBarWidth;
    barRect.yMins = rect.yMaxs - InfoBarHeight;
    barRect.xMaxs = rect.xMaxs - ScrollBarWidth - 1;
    barRect.yMaxs = rect.yMaxs;
    infoBar.construct(myGUI, this, barRect, "test string");

    addChild(&scrollBar);
    addChild(&titleBar);
    addChild(&infoBar);

    refreshUsableRect();

    //TODO TESTS; remove
    auto lw = new ListWidget(myGUI, this, makeRect(rect.xMaxs + NTB_SCALED(10), rect.yMins, rect.xMaxs + NTB_SCALED(60), rect.yMins + NTB_SCALED(50)));
    lw->allocEntries(5);
    lw->addEntryText(0, "Hello");
    lw->addEntryText(1, "Hello World!");
    lw->addEntryText(2, "Testing 2");
    lw->addEntryText(3, "Testing a slightly longer string");
    lw->addEntryText(4, "Testing 4");
    assert(lw->getEntryText(0) == "Hello");
    assert(lw->getEntryText(1) == "Hello World!");
    assert(lw->getEntryText(2) == "Testing 2");
    assert(lw->getEntryText(3) == "Testing a slightly longer string");
    assert(lw->getEntryText(4) == "Testing 4");
    addChild(lw);
    popupWidget = lw;
    /*
    auto cp = new ColorPickerWidget(myGUI, this, rect.xMaxs + NTB_SCALED(10), rect.yMins);
    addChild(cp);
    popupWidget = cp;
    //*/
    /*
    {
        Rectangle box;
        box.xMins = rect.xMaxs + NTB_SCALED(190);
        box.yMins = rect.yMins;
        box.xMaxs = box.xMins + NTB_SCALED(140);//256 + 12;
        box.yMaxs = box.yMins + NTB_SCALED(155);//256 + 35;

        ProjectionParameters projParams;
        projParams.fovYRadians = degToRad(60.0f);
        projParams.aspectRatio = 0.0f; // auto compute
        projParams.zNear = 0.5f;
        projParams.zFar = 100.0f;
        projParams.autoAdjustAspect = true;

        auto v3d = new View3DWidget(myGUI, this, box, "3D View 1", projParams);
        //v3d->setInteractive(false);
        //v3d->setShowXyzLabels(false);
        addChild(v3d);
        popupWidget = v3d;
    }
    //*/
    /*
    {
        Rectangle box;
        box.xMins = rect.xMaxs + NTB_SCALED(10);
        box.yMins = rect.yMins + NTB_SCALED(100);
        box.xMaxs = box.xMins + NTB_SCALED(300);
        box.yMaxs = box.yMins + NTB_SCALED(200);

        ProjectionParameters projParams;
        projParams.fovYRadians = degToRad(60.0f);
        projParams.aspectRatio = 1.0f/1.6f;
        projParams.zNear = 0.5f;
        projParams.zFar = 100.0f;
        projParams.autoAdjustAspect = false;

        const Mat4x4 projMatrix =
            Mat4x4::perspective(projParams.fovYRadians,
                                projParams.aspectRatio,
                                projParams.zNear,
                                projParams.zFar);
        const Mat4x4 viewMatrix =
            Mat4x4::lookAt(makeVec3(0.0f, 0.0f, +1.0f),
                           makeVec3(0.0f, 0.0f, -1.0f),
                           makeVec3(0.0f, 1.0f, 0.0f));
        projParams.viewProjMatrix = Mat4x4::multiply(viewMatrix, projMatrix);

        auto v3d = new View3DWidget(myGUI, this, box, NTB_NULL, projParams);
        addChild(v3d);
        popupWidget = v3d;
    }
    //*/
}

WindowWidget::~WindowWidget()
{
    //TODO NOTE might need to delete the dynamic child elements!

    // Edit fields are never dynamically allocated. Just unlink.
    editFields.unlinkAll();
}

void WindowWidget::onDraw(GeometryBatch & geoBatch) const
{
    if (!isVisible())
    {
        return;
    }

    // First draw the window itself:
    drawSelf(geoBatch);

    // If we have an open popup, it must draw last, at the top.
    // We have to override Widget::onDraw to draw the children
    // first and them the popup (which is also a window child).
    if (popupWidget != NTB_NULL)
    {
        const int childCount = getChildCount();
        for (int c = 0; c < childCount; ++c)
        {
            const Widget * child = getChild(c);
            if (child != popupWidget)
            {
                child->onDraw(geoBatch);
            }
        }
    }
    else // Same logic of Widget::onDraw
    {
        drawChildren(geoBatch);
    }

    // Has to be on top of everything...
    drawResizeHandles(geoBatch);

    // But the popup still has to be above the resize handles.
    if (popupWidget != NTB_NULL)
    {
        popupWidget->onDraw(geoBatch);
    }
}

void WindowWidget::drawResizeHandles(GeometryBatch & geoBatch) const
{
    // This messy block of code draws the wedges in each corner
    // of the panel to indicate the window is resizeable.
    //
    // Each wedge is a main colored line and
    // a shade line offset by one pixel.
    //
    const Color32 wedgeColor = packColor(255, 255, 255); //TODO could be configurable...
    const Color32 shadeColor = packColor(0, 0, 0);
    const int cornerWedgeLineSize = NTB_SCALED(12);
    const int cornerWedgeLineOffs = NTB_SCALED(4);

    const int xMins = rect.xMins;
    const int xMaxs = rect.xMaxs;
    const int yMins = rect.yMins;
    const int yMaxs = rect.yMaxs;
    int xFrom, yFrom, xTo, yTo;

    // Top-left corner, horizontal line width shade:
    xFrom = xMins + cornerWedgeLineOffs;
    yFrom = yMins + cornerWedgeLineOffs;
    xTo = xMins + cornerWedgeLineSize;
    yTo = yFrom;
    geoBatch.drawLine(xFrom, yFrom, xTo, yTo, wedgeColor);
    geoBatch.drawLine(xFrom, yFrom + 1, xTo, yTo + 1, shadeColor);

    // Top-left corner, vertical line with shade:
    xFrom = xMins + cornerWedgeLineOffs;
    yFrom = yMins + cornerWedgeLineOffs;
    xTo = xFrom;
    yTo = yMins + cornerWedgeLineSize;
    geoBatch.drawLine(xFrom, yFrom, xTo, yTo, wedgeColor);
    geoBatch.drawLine(xFrom + 1, yFrom + 1, xTo + 1, yTo, shadeColor);

    // Top-right corner, horizontal line width shade:
    xFrom = xMaxs - cornerWedgeLineSize;
    yFrom = yMins + cornerWedgeLineOffs;
    xTo = xMaxs - cornerWedgeLineOffs;
    yTo = yFrom;
    geoBatch.drawLine(xFrom, yFrom, xTo, yTo, wedgeColor);
    geoBatch.drawLine(xFrom, yFrom + 1, xTo, yTo + 1, shadeColor);

    // Top-right corner, vertical line with shade:
    xFrom = xMaxs - cornerWedgeLineOffs;
    yFrom = yMins + cornerWedgeLineOffs;
    xTo = xFrom;
    yTo = yMins + cornerWedgeLineSize;
    geoBatch.drawLine(xFrom, yFrom, xTo, yTo, wedgeColor);
    geoBatch.drawLine(xFrom + 1, yFrom + 1, xTo + 1, yTo, shadeColor);

    // Bottom-left corner, horizontal line width shade:
    xFrom = xMins + cornerWedgeLineOffs;
    yFrom = yMaxs - cornerWedgeLineOffs;
    xTo = xMins + cornerWedgeLineSize;
    yTo = yFrom;
    geoBatch.drawLine(xFrom, yFrom, xTo, yTo, wedgeColor);
    geoBatch.drawLine(xFrom, yFrom + 1, xTo, yTo + 1, shadeColor);

    // Bottom-left corner, vertical line with shade:
    xFrom = xMins + cornerWedgeLineOffs;
    yFrom = yMaxs - cornerWedgeLineOffs;
    xTo = xFrom;
    yTo = yMaxs - cornerWedgeLineSize;
    geoBatch.drawLine(xFrom, yFrom, xTo, yTo, wedgeColor);
    geoBatch.drawLine(xFrom + 1, yFrom - 1, xTo + 1, yTo, shadeColor);

    // Bottom-right corner, horizontal line width shade:
    xFrom = xMaxs - cornerWedgeLineOffs;
    yFrom = yMaxs - cornerWedgeLineOffs;
    xTo = xMaxs - cornerWedgeLineSize;
    yTo = yFrom;
    geoBatch.drawLine(xFrom, yFrom, xTo, yTo, wedgeColor);
    geoBatch.drawLine(xFrom + 1, yFrom + 1, xTo, yTo + 1, shadeColor);

    // Bottom-right corner, vertical line with shade:
    xFrom = xMaxs - cornerWedgeLineOffs;
    yFrom = yMaxs - cornerWedgeLineOffs;
    xTo = xFrom;
    yTo = yMaxs - cornerWedgeLineSize;
    geoBatch.drawLine(xFrom, yFrom, xTo, yTo, wedgeColor);
    geoBatch.drawLine(xFrom + 1, yFrom + 1, xTo + 1, yTo, shadeColor);

    //DEBUG
    /*
    const int handleSize = NTB_SCALED(12);

    const Rectangle topLeft(xMins, yMins, xMins + handleSize, yMins + handleSize);
    const Rectangle topRight(xMaxs - handleSize, yMins, xMaxs, yMins + handleSize);

    const Rectangle bottomLeft(xMins, yMaxs - handleSize, xMins + handleSize, yMaxs);
    const Rectangle bottomRight(xMaxs - handleSize, yMaxs - handleSize, xMaxs, yMaxs);

    geoBatch.drawRectFilled(topLeft, packColor(255,0,0));
    geoBatch.drawRectFilled(bottomLeft, packColor(255,0,0));
    geoBatch.drawRectFilled(topRight, packColor(255,0,0));
    geoBatch.drawRectFilled(bottomRight, packColor(255,0,0));
    //*/

    //geoBatch.drawRectFilled(usableRect, packColor(255,0,0, 100));
}

bool WindowWidget::onMouseButton(const MouseButton::Enum button, const int clicks)
{
    if (!isVisible())
    {
        return false;
    }

    resizingCorner = CornerNone;

    // Check for intersection with the resize handles in the corners.
    // We can resize the window if there's a left click.
    if (isMouseIntersecting() && leftClick(button, clicks))
    {
        const int xMins = rect.xMins;
        const int xMaxs = rect.xMaxs;
        const int yMins = rect.yMins;
        const int yMaxs = rect.yMaxs;
        const int handleSize = NTB_SCALED(12);

        Rectangle resizeHandles[CornerCount];
        resizeHandles[ TopLeft     ].set(xMins, yMins, xMins + handleSize, yMins + handleSize); // top-left
        resizeHandles[ BottomLeft  ].set(xMins, yMaxs - handleSize, xMins + handleSize, yMaxs); // bottom-left
        resizeHandles[ TopRight    ].set(xMaxs - handleSize, yMins, xMaxs, yMins + handleSize); // top-right
        resizeHandles[ BottomRight ].set(xMaxs - handleSize, yMaxs - handleSize, xMaxs, yMaxs); // bottom-right

        for (int c = 0; c < CornerCount; ++c)
        {
            if (resizeHandles[c].containsPoint(lastMousePos))
            {
                resizingCorner = Corner(c); // Save for onMouseMotion()
                onDisableEditing();         // All edit fields lose focus
                setMouseDragEnabled(false); // Cancel any mouse drag
                return true;
            }
        }
    }

    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        Widget * child = getChild(c);
        if (child->onMouseButton(button, clicks))
        {
            // Bubble this message up to the parent (this)
            // or break the chain if one of the Widgets is
            // the active/focused edit field. Simply calling
            // onDisableEditing() wouldn't work.
            child->onDisableEditing();
            return true;
        }
    }

    // If the cursor is intersecting this element or any
    // of its children, we consume the mouse click, even
    // if it has no input effect in the UI.
    if (isMouseIntersecting())
    {
        onDisableEditing();
        return true;
    }
    return false;
}

bool WindowWidget::onMouseMotion(const int mx, const int my)
{
    if (!isVisible())
    {
        return false;
    }

    // Window can go out from the sides and bottom, but not
    // out the top of the screen. We preempt movement at the
    // window level if that's the case.
    int clampedY = my;
    if (isMouseDragEnabled())
    {
        const int displacementY = my - lastMousePos.y;
        if ((rect.yMins + displacementY) < 0)
        {
            clampedY = my - (rect.yMins + displacementY);
        }
    }

    // Handle resizing each corner:
    switch (resizingCorner)
    {
    case TopLeft :
        resizeWithMin(resizingCorner, rect.xMins, rect.yMins,
                      mx - lastMousePos.x, clampedY - lastMousePos.y);
        break;

    case BottomLeft :
        resizeWithMin(resizingCorner, rect.xMins, rect.yMaxs,
                      mx - lastMousePos.x, clampedY - lastMousePos.y);
        break;

    case TopRight :
        resizeWithMin(resizingCorner, rect.xMaxs, rect.yMins,
                      mx - lastMousePos.x, clampedY - lastMousePos.y);
        break;

    case BottomRight :
        resizeWithMin(resizingCorner, rect.xMaxs, rect.yMaxs,
                      mx - lastMousePos.x, clampedY - lastMousePos.y);
        break;

    default :
        // Not an error, just means we are not resizing.
        break;
    } // switch (resizingCorner)

    return Widget::onMouseMotion(mx, clampedY);
}

bool WindowWidget::onMouseScroll(const int yScroll)
{
    // Allow child element to respond the event first (like a ColorPicker window)
    const int childCount = getChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        if (getChild(c)->isMouseIntersecting() &&
            getChild(c)->onMouseScroll(yScroll))
        {
            return true;
        }
    }

    // Only scroll if the mouse is hovering this window!
    if (isMouseIntersecting())
    {
        return scrollBar.onMouseScroll(yScroll);
    }

    return false;
}

void WindowWidget::setMouseIntersecting(const bool intersect)
{
    Widget::setMouseIntersecting(intersect);

    // We want to highlight these when the parent window gains focus.
    if (intersect)
    {
        scrollBar.setHighlightedColors();
        titleBar.setHighlightedColors();
        infoBar.setHighlightedColors();
    }
}

void WindowWidget::onMove(const int displacementX, const int displacementY)
{
    Widget::onMove(displacementX, displacementY);

    // Keep the effective usable area up-to-date.
    refreshUsableRect();
}

void WindowWidget::onAdjustLayout()
{
    // Keep the effective usable area up-to-date.
    refreshUsableRect();
}

void WindowWidget::resizeWithMin(const Corner corner, int & x, int & y, int offsetX, int offsetY)
{
    const int minWindowWidth  = NTB_SCALED(145);
    const int minWindowHeight = NTB_SCALED(115);

    const Rectangle old = rect;

    // x/y are refs to 'this->rect'.
    x += offsetX;
    y += offsetY;

    // Rollback if we are below the size minimum.
    if (rect.getWidth() < minWindowWidth)
    {
        rect.xMins = old.xMins;
        rect.xMaxs = old.xMaxs;
        offsetX = 0;
    }
    if (rect.getHeight() < minWindowHeight)
    {
        rect.yMins = old.yMins;
        rect.yMaxs = old.yMaxs;
        offsetY = 0;
    }
    // Never go out the top of the screen.
    if (rect.yMins < 0)
    {
        rect.yMins = old.yMins;
        offsetY = 0;
    }

    // Only notify the children and perform adjustments if there's a change in size.
    // We might have snapped them to zero if already at the min size.
    if (offsetX != 0 || offsetY != 0)
    {
        for (int c = 0; c < getChildCount(); ++c)
        {
            getChild(c)->onResize(offsetX, offsetY, corner);
        }
        onAdjustLayout();
    }
}

void WindowWidget::refreshUsableRect()
{
    // Need to refresh the usable sub-rect taking into account
    // the space occupied by the top/bottom/side bars.
    usableRect = rect;

    // Add a small hand-adjusted offset to the top/bottom.
    const int offset = NTB_SCALED(4);
    usableRect.xMaxs -= scrollBar.getRect().getWidth();
    usableRect.yMins += titleBar.getRect().getHeight() + offset;
    usableRect.yMaxs -= infoBar.getRect().getHeight()  + offset;
}

void WindowWidget::onDisableEditing()
{
    EditField * pEdit = editFields.getFirst<EditField>();
    if (pEdit != NTB_NULL)
    {
        pEdit->setActive(false);
    }
}

} // namespace ntb {}
