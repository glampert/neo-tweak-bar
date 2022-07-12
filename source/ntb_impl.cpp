
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

constexpr int kVarHeight              = 30;
constexpr int kVarTopSpacing          = 55;
constexpr int kVarLeftSpacing         = 15;
constexpr int kVarRightSpacing        = 45;
constexpr int kVarInBetweenSpacing    = 4;
constexpr int kVarNestOffsetX         = 8;

constexpr int kPanelStartWidth        = 150;
constexpr int kPanelStartHeight       = 300;
constexpr int kPanelTitleBarHeight    = 40;
constexpr int kPanelTitleBarBtnSize   = 28;
constexpr int kPanelScrollBarWidth    = 40;
constexpr int kPanelScrollBarBtnSize  = 25;

constexpr const char * kBoolTrueStr   = "On";
constexpr const char * kBoolFalseStr  = "Off";

// ========================================================
// class VariableImpl:
// ========================================================

VariableImpl::VariableImpl()
{
}

VariableImpl::~VariableImpl()
{
    VarDisplayWidget::orphanAllChildren();
}

void VariableImpl::init(PanelImpl * myPanel, Variable * myParent, const char * myName, bool readOnly, VariableType varType,
                        void * varData, int elementCount, const EnumConstant * enumConstants, const VarCallbacksAny * optionalCallbacks)
{
    this->panel         = myPanel;
    this->hashCode      = hashString(myName);
    this->varType       = varType;
    this->varData       = varData;
    this->elementCount  = elementCount;
    this->enumConstants = enumConstants;
    this->readOnly      = readOnly;

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
                if (this->elementCount != 1)
                {
                    errorF("ColorU32 from callback must specify size = 1, got %i", this->elementCount);
                }
                NTB_ASSERT(this->elementCount == 1);
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

    // Hierarchy layout:
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
    bool checkboxInitialState = false;

    if (!this->readOnly)
    {
        if (varType == VariableType::Bool)
        {
            varWidgetFlags |= VarDisplayWidget::Flag_WithCheckboxButton;

            if (this->varData != nullptr)
            {
                auto b = reinterpret_cast<bool *>(varData);
                checkboxInitialState = *b;
            }
            else
            {
                this->optionalCallbacks.callGetter(&checkboxInitialState);
            }
        }
        else
        {
            if (isNumberVar())
            {
                varWidgetFlags |= VarDisplayWidget::Flag_WithValueEditButtons;
            }
            if (isEditPopupVar())
            {
                varWidgetFlags |= VarDisplayWidget::Flag_WithEditPopupButton;
            }
        }
    }

    VarDisplayWidget::init(panel->getGUI(), parentVarImpl, varRect, visible, window, myName, varWidgetFlags, checkboxInitialState);

    // This is the default for colors - display as a colored rectangle.
    displayColorAsText(false);
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

Variable * VariableImpl::collapseHierarchy()
{
    VarDisplayWidget::setExpandCollapseState(false);
    return this;
}

Variable * VariableImpl::expandHierarchy()
{
    VarDisplayWidget::setExpandCollapseState(true);
    return this;
}

Variable * VariableImpl::displayColorAsText(bool displayAsRgbaNumbers)
{
    if (isColorVar())
    {
        if (!displayAsRgbaNumbers) // Display as a colored rectangle?
        {
            const Color32 varColor = getVarColorValue();
            VarDisplayWidget::setEditFieldBackground(varColor);
            VarDisplayWidget::setFlag(VarDisplayWidget::Flag_ColorDisplayVar, true);
        }
        else
        {
            VarDisplayWidget::setEditFieldBackground(0);
            VarDisplayWidget::setFlag(VarDisplayWidget::Flag_ColorDisplayVar, false);
        }
    }
    return this;
}

Variable * VariableImpl::valueRange(Float64 valueMin, Float64 valueMax, bool clamped)
{
    this->valueMin = valueMin;
    this->valueMax = valueMax;
    this->clamped  = clamped;
    return this;
}

Variable * VariableImpl::valueStep(Float64 step)
{
    this->step = step;
    return this;
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

bool VariableImpl::isColorVar() const
{
    return varType >= VariableType::ColorF && varType <= VariableType::ColorU32;
}

bool VariableImpl::isEditPopupVar() const
{
    // Enums, colors, vectors and floats have an edit popup button.
    if (varType == VariableType::Flt32 || varType == VariableType::Flt64)
    {
        return true;
    }

    if (varType >= VariableType::Enum && varType <= VariableType::ColorU32)
    {
        return true;
    }

    return false;
}

bool VariableImpl::onGetVarValueText(SmallStr & valueText) const
{
    if (varData == nullptr && optionalCallbacks.isNull())
    {
        return false;
    }

    const void * valuePtr = nullptr;
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
    switch (varType)
    {
    case VariableType::Undefined:
    case VariableType::NumberCB:
    case VariableType::ColorCB:
    case VariableType::StringCB:
        {
            NTB_ASSERT(false && "Invalid variable type!");
            return false;
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
                valueText += SmallStr::fromNumber(v[i], 10, "%.3f");
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

    return true;
}

void VariableImpl::onSetVarValueText(const SmallStr & valueText)
{
    NTB_ASSERT(!readOnly);
    // TODO
}

Color32 VariableImpl::getVarColorValue() const
{
    NTB_ASSERT(isColorVar());

    Color32 tempColorBuffer[4] = {}; // Largest color value we need to handle.
    void * colorValuePtr = tempColorBuffer;

    if (varData != nullptr)
    {
        size_t colorElementSize = 0;
        switch (varType)
        {
        case VariableType::ColorF:
            NTB_ASSERT(elementCount == 3 || elementCount == 4); // RGB or RGBA
            colorElementSize = sizeof(Float32);
            break;

        case VariableType::Color8B:
            NTB_ASSERT(elementCount == 3 || elementCount == 4); // RGB or RGBA
            colorElementSize = sizeof(std::uint8_t);
            break;

        case VariableType::ColorU32:
            NTB_ASSERT(elementCount == 1); // Packed RGBA
            colorElementSize = sizeof(Color32);
            break;

        default:
            NTB_ASSERT(false);
            return Color32{ 0 };
        }

        const size_t colorValueSize = elementCount * colorElementSize;
        std::memcpy(colorValuePtr, varData, colorValueSize);
    }
    else
    {
        optionalCallbacks.callGetter(colorValuePtr);
    }

    // Convert to Color32:
    Color32 result{ 0 };
    switch (varType)
    {
    case VariableType::ColorF:
        {
            NTB_ASSERT(elementCount == 3 || elementCount == 4); // RGB or RGBA
            auto src = reinterpret_cast<const Float32 *>(colorValuePtr);
            const std::uint8_t r = floatToByte(src[0]);
            const std::uint8_t g = floatToByte(src[1]);
            const std::uint8_t b = floatToByte(src[2]);
            const std::uint8_t a = (elementCount == 4) ? floatToByte(src[3]) : 255;
            result = packColor(r, g, b, a);
        }
        break;

    case VariableType::Color8B:
        {
            NTB_ASSERT(elementCount == 3 || elementCount == 4); // RGB or RGBA
            auto src = reinterpret_cast<const std::uint8_t *>(colorValuePtr);
            result = packColor(src[0], src[1], src[2], (elementCount == 4) ? src[3] : 255);
        }
        break;

    case VariableType::ColorU32:
        {
            NTB_ASSERT(elementCount == 1); // Packed RGBA
            auto src = reinterpret_cast<const Color32 *>(colorValuePtr);
            result = *src;
        }
        break;

    default:
        NTB_ASSERT(false);
        return Color32{ 0 };
    }

    return result;
}

Vec3 VariableImpl::getVarRotationAnglesValue() const
{
    NTB_ASSERT(varType == VariableType::DirVec3 || varType == VariableType::Quat4);

    const void * valuePtr = nullptr;
    NTB_ALIGNED(Float32 tempValueBuffer[4], 16) = {}; // Vec3/Quat4

    if (varData != nullptr)
    {
        valuePtr = varData;
    }
    else
    {
        optionalCallbacks.callGetter(tempValueBuffer);
        valuePtr = tempValueBuffer;
    }

    if (varType == VariableType::DirVec3)
    {
        auto src = reinterpret_cast<const Vec3 *>(valuePtr);
        return *src;
    }
    else // varType == VariableType::Quat4
    {
        auto src = reinterpret_cast<const Quat *>(valuePtr);
        const Vec3 xyz = Quat::toAngles(*src);
        return xyz;
    }
}

struct VarOpIncrement
{
    Float64 valueMin;
    Float64 valueMax;
    Float64 step;
    bool    clamped; // If true clamps to [valueMin,valueMax]

    template<typename T>
    void apply(void * valuePtr) const
    {
        auto v = reinterpret_cast<T *>(valuePtr);
        if (clamped)
        {
            (*v) = clamp<T>((*v) + T(step), T(valueMin), T(valueMax));
        }
        else
        {
            (*v) += T(step);
        }
    }
};

struct VarOpDecrement
{
    Float64 valueMin;
    Float64 valueMax;
    Float64 step;
    bool    clamped; // If true clamps to [valueMin,valueMax]

    template<typename T>
    void apply(void * valuePtr) const
    {
        auto v = reinterpret_cast<T *>(valuePtr);
        if (clamped)
        {
            (*v) = clamp<T>((*v) - T(step), T(valueMin), T(valueMax));
        }
        else
        {
            (*v) -= T(step);
        }
    }
};

template<typename OP>
void VariableImpl::applyNumberVarOp(const OP & op)
{
    NTB_ASSERT(isNumberVar());
    NTB_ASSERT(!readOnly);

    void * valuePtr = nullptr;
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
        op.apply<std::uintptr_t>(valuePtr);
        break;

    case VariableType::Int8:
        op.apply<std::int8_t>(valuePtr);
        break;

    case VariableType::UInt8:
        op.apply<std::uint8_t>(valuePtr);
        break;

    case VariableType::Int16:
        op.apply<std::int16_t>(valuePtr);
        break;

    case VariableType::UInt16:
        op.apply<std::uint16_t>(valuePtr);
        break;

    case VariableType::Int32:
        op.apply<std::int32_t>(valuePtr);
        break;

    case VariableType::UInt32:
        op.apply<std::uint32_t>(valuePtr);
        break;

    case VariableType::Int64:
        op.apply<std::int64_t>(valuePtr);
        break;

    case VariableType::UInt64:
        op.apply<std::uint64_t>(valuePtr);
        break;

    case VariableType::Flt32:
        op.apply<Float32>(valuePtr);
        break;

    case VariableType::Flt64:
        op.apply<Float64>(valuePtr);
        break;

    default:
        NTB_ASSERT(false && "Invalid variable type!");
        return;
    }

    if (!optionalCallbacks.isNull())
    {
        optionalCallbacks.callSetter(valuePtr);
    }
}

void VariableImpl::onIncrementButton()
{
    applyNumberVarOp(VarOpIncrement{ valueMin, valueMax, step, clamped });
}

void VariableImpl::onDecrementButton()
{
    applyNumberVarOp(VarOpDecrement{ valueMin, valueMax, step, clamped });
}

void VariableImpl::onListEntrySelected(const ListWidget * listWidget, int selectedEntry)
{
    NTB_ASSERT(this == listWidget->getParent());
    NTB_ASSERT(varType == VariableType::Enum);
    NTB_ASSERT(!readOnly);
    NTB_ASSERT(enumConstants != nullptr);
    NTB_ASSERT(elementCount > 1);

    auto entryText = listWidget->getEntryText(selectedEntry);
    int enumConstantIndex = -1;

    for (int i = 1; i < elementCount; ++i)
    {
        if (entryText == enumConstants[i].name)
        {
            enumConstantIndex = i;
            break;
        }
    }

    NTB_ASSERT(enumConstantIndex >= 0);
    const std::int64_t enumVal = enumConstants[enumConstantIndex].value;

    if (varData != nullptr)
    {
        const EnumConstant & enumTypeSize = enumConstants[0];
        switch (enumTypeSize.value)
        {
        case sizeof(std::int8_t)  : *reinterpret_cast<std::int8_t  *>(varData) = static_cast<std::int8_t >(enumVal); break;
        case sizeof(std::int16_t) : *reinterpret_cast<std::int16_t *>(varData) = static_cast<std::int16_t>(enumVal); break;
        case sizeof(std::int32_t) : *reinterpret_cast<std::int32_t *>(varData) = static_cast<std::int32_t>(enumVal); break;
        case sizeof(std::int64_t) : *reinterpret_cast<std::int64_t *>(varData) = static_cast<std::int64_t>(enumVal); break;
        default : NTB_ASSERT(false);
        }
    }
    else
    {
        optionalCallbacks.callSetter(&enumVal);
    }
}

void VariableImpl::onColorPickerColorSelected(const ColorPickerWidget * colorPicker, Color32 selectedColor)
{
    NTB_ASSERT(this == colorPicker->getParent());
    NTB_ASSERT(isColorVar());
    NTB_ASSERT(!readOnly);

    Color32 tempColorBuffer[4] = {}; // Largest color value we need to handle.
    void * colorValuePtr = tempColorBuffer;
    size_t colorElementSize = 0;

    switch (varType)
    {
    case VariableType::ColorF:
        {
            NTB_ASSERT(elementCount == 3 || elementCount == 4); // RGB or RGBA
            colorElementSize = sizeof(Float32);

            std::uint8_t r, g, b, a;
            unpackColor(selectedColor, r, g, b, a);

            const Float32 fR = byteToFloat(r);
            const Float32 fG = byteToFloat(g);
            const Float32 fB = byteToFloat(b);
            const Float32 fA = byteToFloat(a);

            auto dest = reinterpret_cast<Float32 *>(colorValuePtr);
            dest[0] = fR;
            dest[1] = fG;
            dest[2] = fB;
            dest[3] = fA;
        }
        break;

    case VariableType::Color8B:
        {
            NTB_ASSERT(elementCount == 3 || elementCount == 4); // RGB or RGBA
            colorElementSize = sizeof(std::uint8_t);

            std::uint8_t r, g, b, a;
            unpackColor(selectedColor, r, g, b, a);

            auto dest = reinterpret_cast<std::uint8_t *>(colorValuePtr);
            dest[0] = r;
            dest[1] = g;
            dest[2] = b;
            dest[3] = a;
        }
        break;

    case VariableType::ColorU32:
        {
            NTB_ASSERT(elementCount == 1); // Packed RGBA
            colorElementSize = sizeof(Color32);

            auto dest = reinterpret_cast<Color32 *>(colorValuePtr);
            *dest = selectedColor;
        }
        break;

    default:
        NTB_ASSERT(false);
        return;
    }

    if (varData != nullptr)
    {
        const size_t colorValueSize = elementCount * colorElementSize;
        std::memcpy(varData, colorValuePtr, colorValueSize);
    }
    else
    {
        optionalCallbacks.callSetter(colorValuePtr);
    }

    if (VarDisplayWidget::testFlag(VarDisplayWidget::Flag_ColorDisplayVar))
    {
        VarDisplayWidget::setEditFieldBackground(selectedColor);
    }
}

void VariableImpl::onColorPickerClosed(const ColorPickerWidget * colorPicker)
{
    NTB_ASSERT(this == colorPicker->getParent());
    NTB_ASSERT(isColorVar());

    WindowWidget * window = panel->getWindow();
    window->destroyPopupWidget();

    getEditPopupButton().setState(false);
}

void VariableImpl::onView3DAnglesChanged(const View3DWidget * view3d, const Vec3 & rotationDegrees)
{
    NTB_ASSERT(this == view3d->getParent());
    NTB_ASSERT(varType == VariableType::DirVec3 || varType == VariableType::Quat4);
    NTB_ASSERT(!readOnly);

    if (varType == VariableType::DirVec3)
    {
        NTB_ALIGNED(const Vec3 src, 16) = rotationDegrees;

        if (varData != nullptr)
        {
            auto dest = reinterpret_cast<Vec3 *>(varData);
            *dest = src;
        }
        else
        {
            optionalCallbacks.callSetter(&src);
        }
    }
    else // varType == VariableType::Quat4
    {
        NTB_ALIGNED(const Quat src, 16) = Quat::fromAngles(rotationDegrees);

        if (varData != nullptr)
        {
            auto dest = reinterpret_cast<Quat *>(varData);
            *dest = src;
        }
        else
        {
            optionalCallbacks.callSetter(&src);
        }
    }
}

void VariableImpl::onView3DClosed(const View3DWidget * view3d)
{
    NTB_ASSERT(this == view3d->getParent());
    NTB_ASSERT(varType == VariableType::DirVec3 || varType == VariableType::Quat4);

    WindowWidget * window = panel->getWindow();
    window->destroyPopupWidget();

    getEditPopupButton().setState(false);
}

void VariableImpl::onMultiEditWidgetGetFieldValueText(const MultiEditFieldWidget * multiEditWidget, int fieldIndex, SmallStr * outValueText)
{
    NTB_ASSERT(outValueText != nullptr);
    NTB_ASSERT(this == multiEditWidget->getParent());
    NTB_ASSERT(varType == VariableType::VecF);
    NTB_ASSERT(fieldIndex >= 0 && fieldIndex < elementCount);

    const void * valuePtr = nullptr;
    NTB_ALIGNED(Float32 tempValueBuffer[4], 16) = {}; // Vec3/Vec4

    if (varData != nullptr)
    {
        valuePtr = varData;
    }
    else
    {
        optionalCallbacks.callGetter(tempValueBuffer);
        valuePtr = tempValueBuffer;
    }

    auto vec = reinterpret_cast<const Float32 *>(valuePtr);
    const Float32 element = vec[fieldIndex];

    (*outValueText) = SmallStr::fromNumber(element, 10, "%.3f");
}

void VariableImpl::onMultiEditWidgetClosed(const MultiEditFieldWidget * multiEditWidget)
{
    NTB_ASSERT(this == multiEditWidget->getParent());
    NTB_ASSERT(varType == VariableType::VecF);

    WindowWidget * window = panel->getWindow();
    window->destroyPopupWidget();

    getEditPopupButton().setState(false);
}

Float64 VariableImpl::onValueSliderWidgetGetFloatValue(const FloatValueSliderWidget * sliderWidget)
{
    NTB_ASSERT(this == sliderWidget->getParent());
    NTB_ASSERT(varType == VariableType::Flt32 || varType == VariableType::Flt64);

    Float64 valueOut = 0.0;

    switch (varType)
    {
    case VariableType::Flt32:
        {
            if (varData != nullptr)
            {
                auto f32 = reinterpret_cast<const Float32 *>(varData);
                valueOut = *f32;
            }
            else
            {
                Float32 f32 = 0.0f;
                optionalCallbacks.callGetter(&f32);
                valueOut = f32;
            }
        }
        break;

    case VariableType::Flt64:
        {
            if (varData != nullptr)
            {
                auto f64 = reinterpret_cast<const Float64 *>(varData);
                valueOut = *f64;
            }
            else
            {
                Float64 f64 = 0.0f;
                optionalCallbacks.callGetter(&f64);
                valueOut = f64;
            }
        }
        break;
    }

    return valueOut;
}

void VariableImpl::onValueSliderWidgetClosed(const FloatValueSliderWidget * sliderWidget)
{
    NTB_ASSERT(this == sliderWidget->getParent());
    NTB_ASSERT(varType == VariableType::Flt32 || varType == VariableType::Flt64);

    WindowWidget * window = panel->getWindow();
    window->destroyPopupWidget();

    getEditPopupButton().setState(false);
}

void VariableImpl::onEditPopupButton(bool state)
{
    NTB_ASSERT(!readOnly);

    WindowWidget * window = panel->getWindow();

    // In case we already have a popup open, close it first.
    window->destroyPopupWidget();

    // Reset all other edit buttons in the hierarchy:
    panel->enumerateAllVariables([](Variable * var, void * userData)
    {
        auto varImpl = static_cast<VariableImpl *>(var);
        auto thisVar = static_cast<VariableImpl *>(userData);

        if (varImpl != thisVar)
        {
            varImpl->getEditPopupButton().setState(false);
        }
        return true;
    }, this);

    if (state) // New popup opened
    {
        switch (varType)
        {
        case VariableType::Enum:
            if (elementCount > 1)
            {
                Rectangle listRect = getDataDisplayRect();
                listRect.xMaxs -= Widget::uiScaled(2);
                listRect.moveBy(0, listRect.getHeight());

                auto onEntrySelected = ListWidget::OnEntrySelectedDelegate::fromClassMethod<VariableImpl, &VariableImpl::onListEntrySelected>(this);

                auto listWidget = construct(implAllocT<ListWidget>());
                listWidget->init(gui, this, listRect, true, onEntrySelected);

                listWidget->allocEntries(elementCount - 1);
                for (int i = 1; i < elementCount; ++i)
                {
                    listWidget->addEntryText(i - 1, enumConstants[i].name);
                }

                window->setPopupWidget(listWidget);
            }
            break;

        case VariableType::ColorF:
        case VariableType::Color8B:
        case VariableType::ColorU32:
            {
                const int colorPickerWidth  = Widget::uiScaled(360);
                const int colorPickerHeight = Widget::uiScaled(500);
                const int colorPickerXStart = getEditPopupButton().getRect().xMins + Widget::uiScaled(20);
                const int colorPickerYStart = getEditPopupButton().getRect().yMins;

                const Rectangle colorPickerRect = {
                    colorPickerXStart,
                    colorPickerYStart,
                    colorPickerXStart + colorPickerWidth,
                    colorPickerYStart + colorPickerHeight
                };

                auto onColorSelected = ColorPickerWidget::OnColorSelectedDelegate::fromClassMethod<VariableImpl, &VariableImpl::onColorPickerColorSelected>(this);
                auto onClosed = ColorPickerWidget::OnClosedDelegate::fromClassMethod<VariableImpl, &VariableImpl::onColorPickerClosed>(this);

                auto colorPicker = construct(implAllocT<ColorPickerWidget>());

                colorPicker->init(gui, this, colorPickerRect, true,
                                  Widget::uiScaled(30), Widget::uiScaled(18),
                                  Widget::uiScaled(40), Widget::uiScaled(25),
                                  Widget::uiScaled(40), onColorSelected, onClosed);

                window->setPopupWidget(colorPicker);
            }
            break;

        case VariableType::DirVec3:
        case VariableType::Quat4:
            {
                const int view3dWidth  = Widget::uiScaled(300);
                const int view3dHeight = Widget::uiScaled(350);
                const int view3dXStart = getEditPopupButton().getRect().xMins + Widget::uiScaled(20);
                const int view3dYStart = getEditPopupButton().getRect().yMins;

                View3DWidget::ProjectionParameters projParams = {};
                projParams.fovYRadians      = degToRad(60.0f);
                projParams.aspectRatio      = 0.0f; // auto computed
                projParams.zNear            = 0.5f;
                projParams.zFar             = 100.0f;
                projParams.autoAdjustAspect = true;

                const Rectangle view3dRect = {
                    view3dXStart,
                    view3dYStart,
                    view3dXStart + view3dWidth,
                    view3dYStart + view3dHeight
                };

                const View3DWidget::ObjectType type =
                    (varType == VariableType::DirVec3) ?
                    View3DWidget::ObjectType::Arrow :
                    View3DWidget::ObjectType::Sphere;

                auto onAnglesChanged = View3DWidget::OnAnglesChangedDelegate::fromClassMethod<VariableImpl, &VariableImpl::onView3DAnglesChanged>(this);
                auto onClosed = View3DWidget::OnClosedDelegate::fromClassMethod<VariableImpl, &VariableImpl::onView3DClosed>(this);

                auto view3d = construct(implAllocT<View3DWidget>());

                view3d->init(gui, this, view3dRect, true, getVarName().c_str(),
                             Widget::uiScaled(30), Widget::uiScaled(18), Widget::uiScaled(10),
                             projParams, type, onAnglesChanged, onClosed);

                view3d->setRotationDegrees(getVarRotationAnglesValue());

                window->setPopupWidget(view3d);
            }
            break;

        case VariableType::VecF:
            {
                static const char * const fieldLabels[] = { "X:", "Y:", "Z:", "W:" };

                const int multiEditWidth  = Widget::uiScaled(200);
                const int multiEditHeight = Widget::uiScaled(300);
                const int multiEditXStart = getEditPopupButton().getRect().xMins + Widget::uiScaled(20);
                const int multiEditYStart = getEditPopupButton().getRect().yMins;

                const Rectangle multiEditRect = {
                    multiEditXStart,
                    multiEditYStart,
                    multiEditXStart + multiEditWidth,
                    multiEditYStart + multiEditHeight
                };

                auto onGetFieldValueText = MultiEditFieldWidget::OnGetFieldValueTextDelegate::fromClassMethod<VariableImpl, &VariableImpl::onMultiEditWidgetGetFieldValueText>(this);
                auto onClosed = MultiEditFieldWidget::OnClosedDelegate::fromClassMethod<VariableImpl, &VariableImpl::onMultiEditWidgetClosed>(this);

                auto multiEditWidget = construct(implAllocT<MultiEditFieldWidget>());

                multiEditWidget->init(gui, this, multiEditRect, true, getVarName().c_str(),
                                      Widget::uiScaled(30), Widget::uiScaled(18), onGetFieldValueText, onClosed);

                multiEditWidget->allocFields(elementCount);
                for (int i = 0; i < elementCount; ++i)
                {
                    multiEditWidget->addFieldLabel(i, fieldLabels[i]);
                }

                window->setPopupWidget(multiEditWidget);
            }
            break;

        case VariableType::Flt32:
        case VariableType::Flt64:
            {
                const int sliderWidth  = Widget::uiScaled(250);
                const int sliderHeight = Widget::uiScaled(70);
                const int sliderXStart = getEditPopupButton().getRect().xMins + Widget::uiScaled(20);
                const int sliderYStart = getEditPopupButton().getRect().yMins;

                const Rectangle sliderRect = {
                    sliderXStart,
                    sliderYStart,
                    sliderXStart + sliderWidth,
                    sliderYStart + sliderHeight
                };

                auto onGetFloatValue = FloatValueSliderWidget::OnGetFloatValueDelegate::fromClassMethod<VariableImpl, &VariableImpl::onValueSliderWidgetGetFloatValue>(this);
                auto onClosed = FloatValueSliderWidget::OnClosedDelegate::fromClassMethod<VariableImpl, &VariableImpl::onValueSliderWidgetClosed>(this);

                auto sliderWidget = construct(implAllocT<FloatValueSliderWidget>());

                sliderWidget->init(gui, this, sliderRect, true, getVarName().c_str(),
                                   Widget::uiScaled(30), Widget::uiScaled(18), onGetFloatValue, onClosed);

                sliderWidget->setRange(valueMin, valueMax);

                window->setPopupWidget(sliderWidget);
            }
            break;

        default:
            break;
        }
    }
}

void VariableImpl::onCheckboxButton(bool state)
{
    NTB_ASSERT(varType == VariableType::Bool);
    NTB_ASSERT(!readOnly);

    if (varData != nullptr)
    {
        auto b = reinterpret_cast<bool *>(varData);
        *b = state;
    }
    else
    {
        optionalCallbacks.callSetter(&state);
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
    window.orphanAllChildren();
    PanelImpl::destroyAllVariables();
}

void PanelImpl::init(GUIImpl * myGUI, const char * myName)
{
    hashCode = hashString(myName);

    // Guess our window size based on the title length:
    const Float32 titleWidth = GeometryBatch::calcTextWidth(myName, lengthOfString(myName), myGUI->getGlobalTextScaling());

    // TODO: Make these configurable?
    const auto scale = myGUI->getGlobalUIScaling();
    const Rectangle rect = { 0, 0, Widget::uiScaleBy(kPanelStartWidth, scale) + int(titleWidth), Widget::uiScaleBy(kPanelStartHeight, scale) };
    const bool visible = true;
    const bool resizeable = true;

    window.init(myGUI, nullptr, rect, visible, resizeable, myName,
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

Panel * PanelImpl::setName(const char * newName)
{
    window.setTitle(newName);
    hashCode = hashString(newName);
    return this;
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
    // We could potentially keep track if any widget in the UI has changed
    // and if not just re-submit the same GeometryBatch from the previous frame.
    (void)forceRefresh;

    window.onDraw(geoBatch);
}

Variable * PanelImpl::addHierarchyParent(const char * name)
{
    return addHierarchyParent(nullptr, name);
}

Variable * PanelImpl::addHierarchyParent(Variable * parent, const char * name)
{
    NTB_ASSERT(name != nullptr);

    VariableImpl * newVar = construct(implAllocT<VariableImpl>());
    newVar->init(this, parent, name, true, VariableType::Undefined, nullptr, 0, nullptr, nullptr);
    variables.pushBack(newVar);

    return newVar;
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
    GUIImpl::destroyAllPanels();
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
