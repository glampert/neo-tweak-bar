
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

// TODO: Review these hardcoded values. Do we need to make them scaled?

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

void VariableImpl::init(PanelImpl * myPanel, Variable * myParent, const char * myName, bool isReadOnly,
                        Variable::Type varType, void * varData, int elementCount, const EnumConstant * enumConstants,
                        const VarCallbacksAny * optionalCallbacks)
{
    panel = myPanel;
    setName(myName);

    this->isReadOnly    = isReadOnly;
    this->varType       = varType;
    this->varData       = varData;
    this->elementCount  = elementCount;
    this->enumConstants = enumConstants;

    if (optionalCallbacks != nullptr)
    {
        this->optionalCallbacks = *optionalCallbacks;
    }

    const Rectangle windowRect = panel->getWindow()->getRect();
    const bool visible = panel->getWindow()->isVisible();

    auto parentVarImpl = static_cast<VariableImpl *>(myParent);
    auto parentWidget  = static_cast<VarDisplayWidget *>(parentVarImpl);

    Rectangle varRect{};
    if (parentWidget != nullptr)
    {
        // Adjust for the hierarchy expand/collapse button if needed:
        if (!parentWidget->hasExpandCollapseButton())
        {
            const Rectangle parentRect = parentWidget->getRect();
            parentWidget->setRect(parentRect.shrunk(parentWidget->getExpandCollapseButtonSize(), 0));
        }

        const int siblingCount = parentWidget->getChildCount();

        varRect = parentWidget->getRect();
        varRect.moveBy(kVarNestOffsetX, (kVarHeight + kVarInBetweenSpacing) * (siblingCount + 1));
        varRect = varRect.shrunk(kVarNestOffsetX, 0);
    }
    else
    {
        varRect.xMins = windowRect.getX() + kVarLeftSpacing;
        varRect.yMins = windowRect.getY() + kVarTopSpacing + ((kVarHeight + kVarInBetweenSpacing) * panel->getVariablesCount());
        varRect.xMaxs = varRect.xMins + windowRect.getWidth() - kVarRightSpacing - kVarLeftSpacing;
        varRect.yMaxs = varRect.yMins + kVarHeight;
    }

    VarDisplayWidget::init(panel->getGUI(), parentVarImpl, varRect, visible, panel->getWindow(), name.c_str());
}

Variable * VariableImpl::setName(const char * newName)
{
    name = newName;
    hashCode = hashString(newName);
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

void VariableImpl::drawVarValue(GeometryBatch & geoBatch) const
{
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

    // TODO: WIP

    switch (varType)
    {
    case Variable::Type::Undefined:
        NTB_ASSERT(false && "Invalid variable type!");
        break;

    // Need to query the specific type from the callbacks for these.
    case Variable::Type::Number:
    case Variable::Type::Color:
    case Variable::Type::String:
        break;

    case Variable::Type::Ptr:
        break;

    case Variable::Type::Enum:
        break;

    case Variable::Type::VecF:
        break;

    case Variable::Type::DirVec3:
        break;

    case Variable::Type::Quat4:
        break;

    case Variable::Type::ColorF:
        break;

    case Variable::Type::Color8B:
        break;

    case Variable::Type::ColorU32:
        break;

    case Variable::Type::Bool:
        break;

    case Variable::Type::Int8:
        break;

    case Variable::Type::UInt8:
        break;

    case Variable::Type::Int16:
        break;

    case Variable::Type::UInt16:
        break;

    case Variable::Type::Int32:
        break;

    case Variable::Type::UInt32:
        break;

    case Variable::Type::Int64:
        break;

    case Variable::Type::UInt64:
        break;

    case Variable::Type::Flt32:
        break;

    case Variable::Type::Flt64:
        break;

    case Variable::Type::Char:
        break;

    case Variable::Type::CString:
        break;

    #if NEO_TWEAK_BAR_STD_STRING_INTEROP
    case Variable::Type::StdString:
        break;
    #endif // NEO_TWEAK_BAR_STD_STRING_INTEROP
    }
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
    const ntb::Rectangle rect = { 0, 0, kPanelStartWidth + (int)titleWidth, kPanelStartHeight };
    const bool visible    = true;
    const bool resizeable = true;

    window.init(myGUI, nullptr, rect, visible, resizeable, name.c_str(), kPanelTitleBarHeight, kPanelTitleBarBtnSize, kPanelScrollBarWidth, kPanelScrollBarBtnSize);
}

Variable * PanelImpl::addVariableRO(VarType type, Variable * parent, const char * name, const void * var,
                                    int elementCount, const EnumConstant * enumConstants)
{
    NTB_ASSERT(type != VarType::Undefined);
    NTB_ASSERT(name != nullptr);
    NTB_ASSERT(var  != nullptr);

    VariableImpl * newVar = construct(implAllocT<VariableImpl>());

    const bool isReadOnly = true;
    newVar->init(this, parent, name, isReadOnly, type, const_cast<void *>(var), elementCount, enumConstants, nullptr);

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

    const bool isReadOnly = false;
    newVar->init(this, parent, name, isReadOnly, type, var, elementCount, enumConstants, nullptr);

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

    const bool isReadOnly = (access == VarAccess::RO);
    newVar->init(this, parent, name, isReadOnly, type, nullptr, elementCount, enumConstants, &callbacks);

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
