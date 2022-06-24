
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

// ========================================================
// Constants:
// TODO: Make these configurable?
// ========================================================

// This effectively limits the length of C strings from callbacks.
constexpr int kVarCallbackDataMaxSize = 256;

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

constexpr const char * kBoolTrueStr  = "On";
constexpr const char * kBoolFalseStr = "Off";

// ========================================================
// class VariableImpl:
// ========================================================

VariableImpl::VariableImpl()
{
}

VariableImpl::~VariableImpl()
{
}

void VariableImpl::init(PanelImpl * myPanel, Variable * myParent, const char * myName, bool readOnly, VariableType varType,
                        void * varData, int elementCount, const EnumConstant * enumConstants, const VarCallbacksAny * optionalCallbacks)
{
    this->panel         = myPanel;
    this->hashCode      = hashString(myName);
    this->readOnly      = readOnly;
    this->varType       = varType;
    this->varData       = varData;
    this->elementCount  = elementCount;
    this->enumConstants = enumConstants;

    if (optionalCallbacks != nullptr)
    {
        this->optionalCallbacks = *optionalCallbacks;

        // Need to query the specific type from the callbacks for these.
        if (this->varType == VariableType::NumberCB)
        {
            this->varType = this->optionalCallbacks.getVariableType();
        }
        else if (this->varType == VariableType::StringCB)
        {
            const auto stringType = this->optionalCallbacks.getVariableType();
            if (stringType == VariableType::Char)
            {
                this->varType = VariableType::CString;
            }
            else
            {
                this->varType = stringType; // StdString
            }
        }
        else if (this->varType == VariableType::ColorCB)
        {
            const auto colorStorageType = this->optionalCallbacks.getVariableType();
            switch (colorStorageType)
            {
            case VariableType::UInt32:
                this->varType = VariableType::ColorU32;
                break;
            case VariableType::Flt32:
                this->varType = VariableType::ColorF;
                break;
            case VariableType::UInt8:
                this->varType = VariableType::Color8B;
                break;
            default:
                NTB_ASSERT(false);
            }
        }
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

    std::uint32_t varWidgetFlags = 0;
    if (!this->readOnly)
    {
        if (varType == VariableType::Bool)
        {
            varWidgetFlags |= VarDisplayWidget::Flag_WithCheckmarkButton;

            if (this->varData != nullptr)
            {
                auto b = reinterpret_cast<bool *>(varData);
                checkmarkButton.setState(*b);
            }
            else
            {
                bool boolFromCallback = false;
                this->optionalCallbacks.callGetter(&boolFromCallback);
                checkmarkButton.setState(boolFromCallback);
            }
        }
        else if (isNumberVar())
        {
            varWidgetFlags |= VarDisplayWidget::Flag_WithValueEditButtons;
        }
        else if (isEditPopupVar())
        {
            varWidgetFlags |= VarDisplayWidget::Flag_WithEditPopupButton;
        }
    }

    VarDisplayWidget::init(panel->getGUI(), parentVarImpl, varRect, visible, window, myName, varWidgetFlags);
}

Variable * VariableImpl::setName(const char * newName)
{
    VarDisplayWidget::setVarName(newName);
    hashCode = hashString(newName);
    return this;
}

const char * VariableImpl::getName() const
{
    return VarDisplayWidget::getVarName().c_str();
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

VariableType VariableImpl::getType() const
{
    return varType;
}

bool VariableImpl::isReadOnly() const
{
    return readOnly;
}

bool VariableImpl::isNumberVar() const
{
    return varType >= VariableType::Ptr && varType <= VariableType::Flt64;
}

bool VariableImpl::isEditPopupVar() const
{
    return varType >= VariableType::Enum && varType <= VariableType::ColorU32;
}

void VariableImpl::drawVarValue(GeometryBatch & geoBatch) const
{
    if (!editField.isVisisble)
    {
        return;
    }

    void * valuePtr = nullptr;
    NTB_ALIGNED(char tempValueBuffer[kVarCallbackDataMaxSize], 16) = {};

    // HACK: Std string needs to be default constructed so a raw byte buffer is not enough...
    #if NEO_TWEAK_BAR_STD_STRING_INTEROP
    std::string tempStdString;
    #endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

    if (varData != nullptr)
    {
        valuePtr = varData;
    }
    else
    {
        #if NEO_TWEAK_BAR_STD_STRING_INTEROP
        if (varType == VariableType::StdString)
        {
            optionalCallbacks.callGetter(&tempStdString);
            valuePtr = &tempStdString;
        }
        else
        #endif // NEO_TWEAK_BAR_STD_STRING_INTEROP
        {
            optionalCallbacks.callGetter(tempValueBuffer);
            valuePtr = tempValueBuffer;
        }
    }

    // Convert value to string:
    SmallStr valueText;
    switch (varType)
    {
    case VariableType::Undefined:
    case VariableType::NumberCB:
    case VariableType::ColorCB:
    case VariableType::StringCB:
        {
            NTB_ASSERT(false && "Invalid variable type!");
            break;
        }
    case VariableType::Enum:
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
    case VariableType::VecF:
    case VariableType::DirVec3:
    case VariableType::Quat4:
    case VariableType::ColorF:
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
    case VariableType::Color8B:
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
    case VariableType::ColorU32:
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
    case VariableType::Bool:
        {
            auto b = reinterpret_cast<const bool *>(valuePtr);
            valueText = (*b ? kBoolTrueStr : kBoolFalseStr);
            break;
        }
    case VariableType::Ptr:
        {
            auto v = reinterpret_cast<const void * const *>(valuePtr);
            valueText = SmallStr::fromPointer(*v);
            break;
        }
    case VariableType::Int8:
        {
            auto i = reinterpret_cast<const std::int8_t *>(valuePtr);
            valueText = SmallStr::fromNumber(std::int64_t(*i));
            break;
        }
    case VariableType::UInt8:
        {
            auto i = reinterpret_cast<const std::uint8_t*>(valuePtr);
            valueText = SmallStr::fromNumber(std::uint64_t(*i));
            break;
        }
    case VariableType::Int16:
        {
            auto i = reinterpret_cast<const std::int16_t*>(valuePtr);
            valueText = SmallStr::fromNumber(std::int64_t(*i));
            break;
        }
    case VariableType::UInt16:
        {
            auto i = reinterpret_cast<const std::uint16_t*>(valuePtr);
            valueText = SmallStr::fromNumber(std::uint64_t(*i));
            break;
        }
    case VariableType::Int32:
        {
            auto i = reinterpret_cast<const std::int32_t *>(valuePtr);
            valueText = SmallStr::fromNumber(std::int64_t(*i));
            break;
        }
    case VariableType::UInt32:
        {
            auto i = reinterpret_cast<const std::uint32_t *>(valuePtr);
            valueText = SmallStr::fromNumber(std::uint64_t(*i));
            break;
        }
    case VariableType::Int64:
        {
            auto i = reinterpret_cast<const std::int64_t *>(valuePtr);
            valueText = SmallStr::fromNumber(*i);
            break;
        }
    case VariableType::UInt64:
        {
            auto i = reinterpret_cast<const std::uint64_t *>(valuePtr);
            valueText = SmallStr::fromNumber(*i);
            break;
        }
    case VariableType::Flt32:
        {
            auto f = reinterpret_cast<const Float32 *>(valuePtr);
            valueText = SmallStr::fromNumber(*f);
            break;
        }
    case VariableType::Flt64:
        {
            auto f = reinterpret_cast<const Float64 *>(valuePtr);
            valueText = SmallStr::fromNumber(*f);
            break;
        }
    case VariableType::Char:
        {
            auto c = reinterpret_cast<const char *>(valuePtr);
            valueText.append(*c);
            break;
        }
    case VariableType::CString:
        {
            auto s = reinterpret_cast<const char *>(valuePtr);
            valueText = s;
            break;
        }
    #if NEO_TWEAK_BAR_STD_STRING_INTEROP
    case VariableType::StdString:
        {
            auto s = reinterpret_cast<const std::string *>(valuePtr);
            valueText.append(s->c_str(), int(s->length()));
            break;
        }
    #endif // NEO_TWEAK_BAR_STD_STRING_INTEROP
    }

    const ColorScheme & myColors = getColors();
    const Color32 editBackground = lighthenRGB(myColors.box.bgTopLeft, 30);

    editField.drawSelf(geoBatch, dataDisplayRect, valueText.c_str(), valueText.getLength(),
                       myColors.text.normal, myColors.text.selection, myColors.text.cursor,
                       editBackground, getTextScaling(), getScaling());
}

struct VarOpIncrement
{
    template<typename T>
    static void Apply(void * valuePtr)
    {
        auto v = reinterpret_cast<T *>(valuePtr);
        (*v) += T(1);
    }
};

struct VarOpDecrement
{
    template<typename T>
    static void Apply(void * valuePtr)
    {
        auto v = reinterpret_cast<T *>(valuePtr);
        (*v) -= T(1);
    }
};

template<typename OP>
void VariableImpl::ApplyVarOp(OP op)
{
    NTB_ASSERT(isNumberVar());

    void* valuePtr = nullptr;
    std::uint64_t numberFromCallback = 0;

    if (varData != nullptr)
    {
        valuePtr = varData;
    }
    else
    {
        optionalCallbacks.callGetter(&numberFromCallback);
        valuePtr = &numberFromCallback;
    }

    switch (varType)
    {
    case VariableType::Ptr:
        op.Apply<std::uintptr_t>(valuePtr);
        break;

    case VariableType::Int8:
        op.Apply<std::int8_t>(valuePtr);
        break;

    case VariableType::UInt8:
        op.Apply<std::uint8_t>(valuePtr);
        break;

    case VariableType::Int16:
        op.Apply<std::int16_t>(valuePtr);
        break;

    case VariableType::UInt16:
        op.Apply<std::uint16_t>(valuePtr);
        break;

    case VariableType::Int32:
        op.Apply<std::int32_t>(valuePtr);
        break;

    case VariableType::UInt32:
        op.Apply<std::uint32_t>(valuePtr);
        break;

    case VariableType::Int64:
        op.Apply<std::int64_t>(valuePtr);
        break;

    case VariableType::UInt64:
        op.Apply<std::uint64_t>(valuePtr);
        break;

    case VariableType::Flt32:
        op.Apply<Float32>(valuePtr);
        break;

    case VariableType::Flt64:
        op.Apply<Float64>(valuePtr);
        break;

    default:
        NTB_ASSERT(false && "Invalid variable type!");
    }

    if (!optionalCallbacks.isNull())
    {
        optionalCallbacks.callSetter(valuePtr);
    }
}

void VariableImpl::onIncrementButton()
{
    ApplyVarOp(VarOpIncrement{});
}

void VariableImpl::onDecrementButton()
{
    ApplyVarOp(VarOpDecrement{});
}

void VariableImpl::onCheckmarkButton(bool state)
{
    NTB_ASSERT(varType  == VariableType::Bool);
    NTB_ASSERT(readOnly == false);

    if (!optionalCallbacks.isNull())
    {
        optionalCallbacks.callSetter(&state);
    }
    else
    {
        auto b = reinterpret_cast<bool *>(varData);
        *b = state;
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
    const auto scale = myGUI->getGlobalUIScaling();
    const Rectangle rect = { 0, 0, Widget::uiScaleBy(kPanelStartWidth, scale) + int(titleWidth), Widget::uiScaleBy(kPanelStartHeight, scale) };
    const bool visible = true;
    const bool resizeable = true;

    window.init(myGUI, nullptr, rect, visible, resizeable, name.c_str(),
                Widget::uiScaleBy(kPanelTitleBarHeight, scale), Widget::uiScaleBy(kPanelTitleBarBtnSize, scale),
                Widget::uiScaleBy(kPanelScrollBarWidth, scale), Widget::uiScaleBy(kPanelScrollBarBtnSize, scale));
}

Variable * PanelImpl::addVariableRO(VariableType type, Variable * parent, const char * name, const void * var,
                                    int elementCount, const EnumConstant * enumConstants)
{
    NTB_ASSERT(type != VariableType::Undefined);
    NTB_ASSERT(name != nullptr);
    NTB_ASSERT(var  != nullptr);

    VariableImpl * newVar = construct(implAllocT<VariableImpl>());

    const bool readOnly = true;
    newVar->init(this, parent, name, readOnly, type, const_cast<void *>(var), elementCount, enumConstants, nullptr);

    variables.pushBack(newVar);
    return newVar;
}

Variable * PanelImpl::addVariableRW(VariableType type, Variable * parent, const char * name, void * var,
                                    int elementCount, const EnumConstant * enumConstants)
{
    NTB_ASSERT(type != VariableType::Undefined);
    NTB_ASSERT(name != nullptr);
    NTB_ASSERT(var  != nullptr);

    VariableImpl * newVar = construct(implAllocT<VariableImpl>());

    const bool readOnly = false;
    newVar->init(this, parent, name, readOnly, type, var, elementCount, enumConstants, nullptr);

    variables.pushBack(newVar);
    return newVar;
}

Variable * PanelImpl::addVariableCB(VariableType type, Variable * parent, const char * name, const VarCallbacksAny & callbacks,
                                    VarAccess access, int elementCount, const EnumConstant * enumConstants)
{
    NTB_ASSERT(type != VariableType::Undefined);
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
    return this;
}

Panel * PanelImpl::setSize(int newWidth, int newHeight)
{
    const auto oldRect = window.getRect();
    window.setRect(Rectangle{
        oldRect.getX(), oldRect.getY(),
        oldRect.getX() + newWidth, oldRect.getY() + newHeight
    });
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
