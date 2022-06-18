
// ================================================================================================
// -*- C++ -*-
// File: ntb_impl.cpp
// Author: Guilherme R. Lampert
// Created on: 26/09/16
// Brief: Private implementations for the public NTB interfaces.
// ================================================================================================

#include "ntb_impl.hpp"

namespace ntb
{

constexpr int kVarHeight             = 30;
constexpr int kVarTopSpacing         = 55;
constexpr int kVarLeftSpacing        = 15;
constexpr int kVarRightSpacing       = 45;
constexpr int kVarInBetweenSpacing   = 4;
constexpr int kVarNestOffsetX        = 8;

constexpr int kPanelStartWidth       = 150;
constexpr int kPanelStartHeight      = 300;
constexpr int kPanelTitleBarHeight   = 40;
constexpr int kPanelTitleBarBtnSize  = 28;
constexpr int kPanelScrollBarWidth   = 40;
constexpr int kPanelScrollBarBtnSize = 25;

// ========================================================
// class VariableImpl:
// ========================================================

VariableImpl::VariableImpl()
{
}

VariableImpl::~VariableImpl()
{
}

void VariableImpl::init(PanelImpl * myPanel, Variable * myParent, const char * myName, bool readOnly, Variable::Type varType,
                        void * varData, int elementCount, const EnumConstant * enumConstants, const VarCallbacksAny * optionalCallbacks)
{
    panel = myPanel;
    setName(myName);

    this->readOnly      = readOnly;
    this->varType       = varType;
    this->varData       = varData;
    this->elementCount  = elementCount;
    this->enumConstants = enumConstants;

    if (optionalCallbacks != nullptr)
    {
        this->optionalCallbacks = *optionalCallbacks;
    }

    WindowWidget * window = panel->getWindow();
    const Rectangle windowRect = window->getRect();
    const bool visible = window->isVisible();

    auto parentVarImpl = static_cast<VariableImpl *>(myParent);
    auto parentWidget  = static_cast<VarDisplayWidget *>(parentVarImpl);

    Rectangle varRect{};
    if (parentWidget != nullptr)
    {
        // Adjust for the hierarchy expand/collapse button if needed:
        if (!parentWidget->hasExpandCollapseButton())
        {
            Rectangle parentRect = parentWidget->getRect();
            parentRect = parentRect.shrunk(parentWidget->getExpandCollapseButtonSize() / 2, 0);
            parentRect.moveBy(parentWidget->getExpandCollapseButtonSize() / 2, 0);
            parentWidget->setRect(parentRect);
        }

        const int siblingCount = parentWidget->getChildCount();

        varRect = parentWidget->getRect();
        varRect.moveBy(window->uiScaled(kVarNestOffsetX), window->uiScaled(kVarHeight + kVarInBetweenSpacing) * (siblingCount + 1));
        varRect = varRect.shrunk(window->uiScaled(kVarNestOffsetX), 0);
    }
    else
    {
        varRect.xMins = windowRect.getX() + window->uiScaled(kVarLeftSpacing);
        varRect.yMins = windowRect.getY() + window->uiScaled(kVarTopSpacing) + (window->uiScaled(kVarHeight + kVarInBetweenSpacing) * panel->getVariablesCount());
        varRect.xMaxs = varRect.xMins + windowRect.getWidth() - window->uiScaled(kVarRightSpacing) - window->uiScaled(kVarLeftSpacing);
        varRect.yMaxs = varRect.yMins + window->uiScaled(kVarHeight);
    }

    VarDisplayWidget::init(panel->getGUI(), parentVarImpl, varRect, visible, window, name.c_str());
}

Variable * VariableImpl::setName(const char * newName)
{
    name = newName;
    hashCode = hashString(newName);
    titleWidth = (int)GeometryBatch::calcTextWidth(name.c_str(), name.getLength(), getGUI()->getGlobalTextScaling());
    return this;
}

const char * VariableImpl::getName() const
{
    return name.c_str();
}

std::uint32_t VariableImpl::getHashCode() const
{
    return hashCode;
}

const GUI * VariableImpl::getGUI() const
{
    return panel->getGUI();
}

GUI * VariableImpl::getGUI()
{
    return panel->getGUI();
}

const Panel * VariableImpl::getPanel() const
{
    return panel;
}

Panel * VariableImpl::getPanel()
{
    return panel;
}

Variable::Type VariableImpl::getType() const
{
    return varType;
}

bool VariableImpl::isReadOnly() const
{
    return readOnly;
}

void VariableImpl::drawVarValue(GeometryBatch & geoBatch) const
{
    Rectangle editBox = getRect().shrunk(uiScaled(4), uiScaled(4));
    editBox.moveBy(uiScaled(titleWidth) + uiScaled(2), 0);
    editBox.xMaxs = std::min(editBox.xMaxs, getRect().xMaxs - uiScaled(2));

    bool shouldDraw = true;
    const Rectangle parentRect = panel->getWindow()->getUsableRect();

    if (editBox.xMins > editBox.xMaxs)
    {
        shouldDraw = false;
    }
    else if (editBox.xMins > parentRect.xMaxs || editBox.xMaxs < parentRect.xMins)
    {
        shouldDraw = false;
    }
    else if (editBox.yMins > parentRect.yMaxs || editBox.yMaxs < parentRect.yMins || editBox.yMaxs > parentRect.yMaxs)
    {
        shouldDraw = false;
    }
    if (!shouldDraw)
    {
        return;
    }

    void * valuePtr = nullptr;
    NTB_ALIGNED(char tempValue[128], 16) = {};

    if (varData != nullptr)
    {
        valuePtr = varData;
    }
    else
    {
        NTB_ASSERT(!optionalCallbacks.isNull());
        optionalCallbacks.callGetter(tempValue);
        valuePtr = tempValue;
    }

    // Convert value to string:
    SmallStr valueText;
    switch (varType)
    {
    case Variable::Type::Undefined:
        NTB_ASSERT(false && "Invalid variable type!");
        break;

    // Need to query the specific type from the callbacks for these.
    case Variable::Type::NumberCB:
    case Variable::Type::ColorCB:
    case Variable::Type::StringCB:
        {
            // TODO!
            break;
        }
    case Variable::Type::Ptr:
        {
            auto v = reinterpret_cast<const void * const *>(valuePtr);
            valueText = SmallStr::fromPointer(*v);
            break;
        }
    case Variable::Type::Enum:
        {
            NTB_ASSERT(enumConstants != nullptr);
            NTB_ASSERT(elementCount  >= 1);

            std::int64_t enumVal = 0;
            const EnumConstant & enumTypeSize = enumConstants[0];
            switch (enumTypeSize.value)
            {
            case sizeof(std::int8_t)  : enumVal = *reinterpret_cast<const std::int8_t  *>(valuePtr); break;
            case sizeof(std::int16_t) : enumVal = *reinterpret_cast<const std::int16_t *>(valuePtr); break;
            case sizeof(std::int32_t) : enumVal = *reinterpret_cast<const std::int32_t *>(valuePtr); break;
            case sizeof(std::int64_t) : enumVal = *reinterpret_cast<const std::int64_t *>(valuePtr); break;
            default : NTB_ASSERT(false);
            }

            const char * enumName = nullptr;
            for (int i = 1; i < elementCount; ++i)
            {
                if (enumConstants[i].value == enumVal)
                {
                    enumName = enumConstants[i].name;
                    break;
                }
            }

            if (enumName != nullptr)
            {
                valueText = enumName;
            }
            else
            {
                valueText = SmallStr::fromNumber(enumVal);
            }
            break;
        }
    case Variable::Type::VecF:
    case Variable::Type::DirVec3:
    case Variable::Type::Quat4:
    case Variable::Type::ColorF:
        {
            auto v = reinterpret_cast<const Float32 *>(valuePtr);
            for (int i = 0; i < elementCount; ++i)
            {
                valueText += SmallStr::fromNumber(v[i]);
                if (i != (elementCount - 1))
                    valueText.append(',');
            }
            break;
        }
    case Variable::Type::Color8B:
        {
            auto c = reinterpret_cast<const std::uint8_t *>(valuePtr);
            for (int i = 0; i < elementCount; ++i)
            {
                valueText += SmallStr::fromNumber(std::uint64_t(c[i]));
                if (i != (elementCount - 1))
                    valueText.append(',');
            }
            break;
        }
    case Variable::Type::ColorU32:
        {
            auto c = reinterpret_cast<const Color32 *>(valuePtr);
            std::uint8_t rgba[4];
            unpackColor(*c, rgba[0], rgba[1], rgba[2], rgba[3]);
            for (int i = 0; i < lengthOfArray(rgba); ++i)
            {
                valueText += SmallStr::fromNumber(std::uint64_t(rgba[i]));
                if (i != (lengthOfArray(rgba) - 1))
                    valueText.append(',');
            }
            break;
        }
    case Variable::Type::Bool:
        {
            auto b = reinterpret_cast<const bool *>(valuePtr);
            valueText = (*b ? "true" : "false");
            break;
        }
    case Variable::Type::Int8:
        {
            auto i = reinterpret_cast<const std::int8_t *>(valuePtr);
            valueText = SmallStr::fromNumber(std::int64_t(*i));
            break;
        }
    case Variable::Type::UInt8:
        {
            auto i = reinterpret_cast<const std::uint8_t*>(valuePtr);
            valueText = SmallStr::fromNumber(std::uint64_t(*i));
            break;
        }
    case Variable::Type::Int16:
        {
            auto i = reinterpret_cast<const std::int16_t*>(valuePtr);
            valueText = SmallStr::fromNumber(std::int64_t(*i));
            break;
        }
    case Variable::Type::UInt16:
        {
            auto i = reinterpret_cast<const std::uint16_t*>(valuePtr);
            valueText = SmallStr::fromNumber(std::uint64_t(*i));
            break;
        }
    case Variable::Type::Int32:
        {
            auto i = reinterpret_cast<const std::int32_t *>(valuePtr);
            valueText = SmallStr::fromNumber(std::int64_t(*i));
            break;
        }
    case Variable::Type::UInt32:
        {
            auto i = reinterpret_cast<const std::uint32_t *>(valuePtr);
            valueText = SmallStr::fromNumber(std::uint64_t(*i));
            break;
        }
    case Variable::Type::Int64:
        {
            auto i = reinterpret_cast<const std::int64_t *>(valuePtr);
            valueText = SmallStr::fromNumber(*i);
            break;
        }
    case Variable::Type::UInt64:
        {
            auto i = reinterpret_cast<const std::uint64_t *>(valuePtr);
            valueText = SmallStr::fromNumber(*i);
            break;
        }
    case Variable::Type::Flt32:
        {
            auto f = reinterpret_cast<const Float32 *>(valuePtr);
            valueText = SmallStr::fromNumber(*f);
            break;
        }
    case Variable::Type::Flt64:
        {
            auto f = reinterpret_cast<const Float64 *>(valuePtr);
            valueText = SmallStr::fromNumber(*f);
            break;
        }
    case Variable::Type::Char:
        {
            auto c = reinterpret_cast<const char *>(valuePtr);
            valueText.append(*c);
            break;
        }
    case Variable::Type::CString:
        {
            auto s = reinterpret_cast<const char *>(valuePtr);
            valueText = s;
            break;
        }
    #if NEO_TWEAK_BAR_STD_STRING_INTEROP
    case Variable::Type::StdString:
        {
            auto s = reinterpret_cast<const std::string *>(valuePtr);
            valueText.append(s->c_str(), int(s->length()));
            break;
        }
    #endif // NEO_TWEAK_BAR_STD_STRING_INTEROP
    }

    const ColorScheme & myColors = getColors();
    const Color32 editBackground = lighthenRGB(myColors.box.bgTopLeft, 30);

    EditField & editField = getEditField();
    editField.drawSelf(geoBatch, editBox, valueText.c_str(), valueText.getLength(),
                       myColors.text.normal, myColors.text.selection, myColors.text.cursor,
                       editBackground, getTextScaling(), getScaling());
}

// ========================================================
// class PanelImpl:
// ========================================================

PanelImpl::PanelImpl()
{
}

PanelImpl::~PanelImpl()
{
}

void PanelImpl::init(GUIImpl * myGUI, const char * myName)
{
    setName(myName);

    // Guess our window size based on the title length:
    const Float32 titleWidth = GeometryBatch::calcTextWidth(name.c_str(), name.getLength(), myGUI->getGlobalTextScaling());

    // TODO: Make these configurable?
    const auto scale = myGUI->getGlobalUIScaling();
    const Rectangle rect = { 0, 0, Widget::uiScaleBy(kPanelStartWidth, scale) + int(titleWidth), Widget::uiScaleBy(kPanelStartHeight, scale) };
    const bool visible = true;
    const bool resizeable = true;

    window.init(myGUI, nullptr, rect, visible, resizeable, name.c_str(),
                Widget::uiScaleBy(kPanelTitleBarHeight, scale), Widget::uiScaleBy(kPanelTitleBarBtnSize, scale),
                Widget::uiScaleBy(kPanelScrollBarWidth, scale), Widget::uiScaleBy(kPanelScrollBarBtnSize, scale));
}

Variable * PanelImpl::addVariableRO(VarType type, Variable * parent, const char * name, const void * var,
                                    int elementCount, const EnumConstant * enumConstants)
{
    NTB_ASSERT(type != VarType::Undefined);
    NTB_ASSERT(name != nullptr);
    NTB_ASSERT(var  != nullptr);

    VariableImpl * newVar = construct(implAllocT<VariableImpl>());

    const bool readOnly = true;
    newVar->init(this, parent, name, readOnly, type, const_cast<void *>(var), elementCount, enumConstants, nullptr);

    variables.pushBack(newVar);
    return newVar;
}

Variable * PanelImpl::addVariableRW(VarType type, Variable * parent, const char * name, void * var,
                                    int elementCount, const EnumConstant * enumConstants)
{
    NTB_ASSERT(type != VarType::Undefined);
    NTB_ASSERT(name != nullptr);
    NTB_ASSERT(var  != nullptr);

    VariableImpl * newVar = construct(implAllocT<VariableImpl>());

    const bool readOnly = false;
    newVar->init(this, parent, name, readOnly, type, var, elementCount, enumConstants, nullptr);

    variables.pushBack(newVar);
    return newVar;
}

Variable * PanelImpl::addVariableCB(VarType type, Variable * parent, const char * name, const VarCallbacksAny & callbacks,
                                    VarAccess access, int elementCount, const EnumConstant * enumConstants)
{
    NTB_ASSERT(type != VarType::Undefined);
    NTB_ASSERT(name != nullptr);
    NTB_ASSERT(!callbacks.isNull());

    VariableImpl * newVar = construct(implAllocT<VariableImpl>());

    const bool readOnly = (access == VarAccess::RO);
    newVar->init(this, parent, name, readOnly, type, nullptr, elementCount, enumConstants, &callbacks);

    variables.pushBack(newVar);
    return newVar;
}

Variable * PanelImpl::findVariable(const char * varName) const
{
    return findItemByName<VariableImpl *>(variables, varName);
}

Variable * PanelImpl::findVariable(std::uint32_t varNameHashCode) const
{
    return findItemByHashCode<VariableImpl *>(variables, varNameHashCode);
}

bool PanelImpl::destroyVariable(Variable * variable)
{
    return eraseAndDestroyItem<VariableImpl *>(variables, variable);
}

void PanelImpl::destroyAllVariables()
{
    destroyAllItems<VariableImpl *>(variables);
}

int PanelImpl::getVariablesCount() const
{
    return variables.getSize();
}

void PanelImpl::enumerateAllVariables(VariableEnumerateCallback enumCallback, void * userContext)
{
    variables.forEach<VariableImpl *>(enumCallback, userContext);
}

bool PanelImpl::onKeyPressed(KeyCode key, KeyModFlags modifiers)
{
    return window.onKeyPressed(key, modifiers);
}

bool PanelImpl::onMouseButton(MouseButton button, int clicks)
{
    return window.onMouseButton(button, clicks);
}

bool PanelImpl::onMouseMotion(int mx, int my)
{
    return window.onMouseMotion(mx, my);
}

bool PanelImpl::onMouseScroll(int yScroll)
{
    return window.onMouseScroll(yScroll);
}

void PanelImpl::onFrameRender(GeometryBatch & geoBatch, bool forceRefresh)
{
    // TODO: Implement forced refresh?
    (void)forceRefresh;

    window.onDraw(geoBatch);
}

Variable * PanelImpl::addHierarchyParent(const char * name)
{
    // TODO: Implement
    NTB_ASSERT(false);
    return nullptr;
}

Variable * PanelImpl::addHierarchyParent(Variable * parent, const char * name)
{
    // TODO: Implement
    NTB_ASSERT(false);
    return nullptr;
}

Panel * PanelImpl::setPosition(int newPosX, int newPosY)
{
    const auto oldRect = window.getRect();
    window.setRect(Rectangle{
        newPosX, newPosY,
        newPosX + oldRect.getWidth(), newPosY + oldRect.getHeight()
    });

    // TODO: Should this be automatic when calling setRect?
    window.onAdjustLayout();
    return this;
}

Panel * PanelImpl::setSize(int newWidth, int newHeight)
{
    const auto oldRect = window.getRect();
    window.setRect(Rectangle{
        oldRect.getX(), oldRect.getY(),
        oldRect.getX() + newWidth, oldRect.getY() + newHeight
    });

    // TODO: Should this be automatic when calling setRect?
    window.onAdjustLayout();
    return this;
}

// ========================================================
// class GUIImpl:
// ========================================================

GUIImpl::GUIImpl()
{
}

GUIImpl::~GUIImpl()
{
}

void GUIImpl::init(const char * myName)
{
    setName(myName);
}

Panel * GUIImpl::findPanel(const char * panelName) const
{
    return findItemByName<PanelImpl *>(panels, panelName);
}

Panel * GUIImpl::findPanel(std::uint32_t panelNameHashCode) const
{
    return findItemByHashCode<PanelImpl *>(panels, panelNameHashCode);
}

Panel * GUIImpl::createPanel(const char * panelName)
{
    NTB_ASSERT(panelName != nullptr);
    PanelImpl * panel = construct(implAllocT<PanelImpl>());
    panel->init(this, panelName);
    panels.pushBack(panel);
    return panel;
}

bool GUIImpl::destroyPanel(Panel * panel)
{
    return eraseAndDestroyItem<PanelImpl *>(panels, panel);
}

void GUIImpl::destroyAllPanels()
{
    destroyAllItems<PanelImpl *>(panels);
}

int GUIImpl::getPanelCount() const
{
    return panels.getSize();
}

void GUIImpl::enumerateAllPanels(PanelEnumerateCallback enumCallback, void * userContext)
{
    panels.forEach<PanelImpl *>(enumCallback, userContext);
}

bool GUIImpl::onKeyPressed(KeyCode key, KeyModFlags modifiers)
{
    const int count = panels.getSize();
    for (int i = 0; i < count; ++i)
    {
        PanelImpl * panel = panels.get<PanelImpl *>(i);
        if (panel->onKeyPressed(key, modifiers))
        {
            return true; // If the event was consumed we stop propagating it.
        }
    }
    return false;
}

bool GUIImpl::onMouseButton(MouseButton button, int clicks)
{
    const int count = panels.getSize();
    for (int i = 0; i < count; ++i)
    {
        PanelImpl * panel = panels.get<PanelImpl *>(i);
        if (panel->onMouseButton(button, clicks))
        {
            return true; // If the event was consumed we stop propagating it.
        }
    }
    return false;
}

bool GUIImpl::onMouseMotion(int mx, int my)
{
    const int count = panels.getSize();
    for (int i = 0; i < count; ++i)
    {
        PanelImpl * panel = panels.get<PanelImpl *>(i);
        if (panel->onMouseMotion(mx, my))
        {
            return true; // If the event was consumed we stop propagating it.
        }
    }
    return false;
}

bool GUIImpl::onMouseScroll(int yScroll)
{
    const int count = panels.getSize();
    for (int i = 0; i < count; ++i)
    {
        PanelImpl * panel = panels.get<PanelImpl *>(i);
        if (panel->onMouseScroll(yScroll))
        {
            return true; // If the event was consumed we stop propagating it.
        }
    }
    return false;
}

void GUIImpl::onFrameRender(bool forceRefresh)
{
    geoBatch.beginDraw();

    const int count = panels.getSize();
    for (int i = 0; i < count; ++i)
    {
        PanelImpl * panel = panels.get<PanelImpl *>(i);
        panel->onFrameRender(geoBatch, forceRefresh);
    }

    // Submit to the RenderInterface.
    geoBatch.endDraw();
}

void GUIImpl::minimizeAllPanels()
{
    const int count = panels.getSize();
    for (int i = 0; i < count; ++i)
    {
        PanelImpl * panel = panels.get<PanelImpl *>(i);
        panel->setMinimized(true);
    }
}

void GUIImpl::maximizeAllPanels()
{
    const int count = panels.getSize();
    for (int i = 0; i < count; ++i)
    {
        PanelImpl * panel = panels.get<PanelImpl *>(i);
        panel->setMinimized(false);
    }
}

void GUIImpl::hideAllPanels()
{
    const int count = panels.getSize();
    for (int i = 0; i < count; ++i)
    {
        PanelImpl * panel = panels.get<PanelImpl *>(i);
        panel->setVisible(false);
    }
}

void GUIImpl::showAllPanels()
{
    const int count = panels.getSize();
    for (int i = 0; i < count; ++i)
    {
        PanelImpl * panel = panels.get<PanelImpl *>(i);
        panel->setVisible(true);
    }
}

void GUIImpl::setGlobalUIScaling(Float32 scaling)
{
    const int count = panels.getSize();
    for (int i = 0; i < count; ++i)
    {
        PanelImpl * panel = panels.get<PanelImpl *>(i);
        panel->setUIScaling(scaling);
    }
}

void GUIImpl::setGlobalTextScaling(Float32 scaling)
{
    const int count = panels.getSize();
    for (int i = 0; i < count; ++i)
    {
        PanelImpl * panel = panels.get<PanelImpl *>(i);
        panel->setTextScaling(scaling);
    }
}

} // namespace ntb {}
