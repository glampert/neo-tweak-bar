#pragma once
// ================================================================================================
// -*- C++ -*-
// File: ntb_impl.hpp
// Author: Guilherme R. Lampert
// Created on: 26/09/16
// Brief: Private implementations for the public NTB interfaces.
// ================================================================================================

#include "ntb.hpp"
#include "ntb_utils.hpp"
#include "ntb_widgets.hpp"

namespace ntb
{

class VariableImpl;
class PanelImpl;
class GUIImpl;

// ========================================================
// class VariableImpl:
// ========================================================

class VariableImpl final
    : public Variable, public VarDisplayWidget
{
public:

    VariableImpl();
    ~VariableImpl();

    void init(PanelImpl * myPanel, Variable * myParent, const char * myName, bool readOnly, VariableType varType,
              void * varData, int elementCount, const EnumConstant * enumConstants, const VarCallbacksAny * optionalCallbacks);

    VariableType getType() const override;
    bool isReadOnly() const override;

    Variable * setName(const char * newName) override;
    const char * getName() const override;
    std::uint32_t getHashCode() const override;

    const GUI * getGUI() const override;
    GUI * getGUI() override;

    const Panel * getPanel() const override;
    Panel * getPanel() override;

    Variable * collapseHierarchy() override;
    Variable * expandHierarchy() override;
    Variable * numberFormat(NumberFormat format) override;
    Variable * displayColorAsText(bool displayAsRgbaNumbers) override;
    Variable * valueRange(Float64 valueMin, Float64 valueMax, bool clamped) override;
    Variable * valueStep(Float64 step) override;

private:

    bool isNumberVar() const;
    bool isColorVar() const;
    bool isEditPopupVar() const;
    template<typename OP> void applyNumberVarOp(const OP & op);
    Color32 getVarColorValue() const;
    Vec3 getVarRotationAnglesValue() const;

    // Widget Delegates:
    void onListEntrySelected(const ListWidget * listWidget, int selectedEntry);
    void onColorPickerColorSelected(const ColorPickerWidget * colorPicker, Color32 selectedColor);
    void onColorPickerClosed(const ColorPickerWidget * colorPicker);
    void onView3DAnglesChanged(const View3DWidget * view3d, const Vec3 & rotationDegrees);
    void onView3DClosed(const View3DWidget * view3d);
    void onMultiEditWidgetGetFieldValueText(const MultiEditFieldWidget * multiEditWidget, int fieldIndex, SmallStr * outValueText);
    void onMultiEditWidgetClosed(const MultiEditFieldWidget * multiEditWidget);
    Float64 onValueSliderWidgetGetFloatValue(const FloatValueSliderWidget * sliderWidget);
    void onValueSliderWidgetClosed(const FloatValueSliderWidget * sliderWidget);

    // VarDisplayWidget overrides:
    bool onGetVarValueText(SmallStr & valueText) const override;
    void onSetVarValueText(const SmallStr & valueText) override;
    void onIncrementButton() override;
    void onDecrementButton() override;
    void onEditPopupButton(bool state) override;
    void onCheckboxButton(bool state) override;

private:

    PanelImpl *          panel{ nullptr };
    std::uint32_t        hashCode{ 0 }; // Hash of name for fast lookup.
    int                  elementCount{ 0 };
    VariableType         varType{ VariableType::Undefined };
    void               * varData{ nullptr };
    const EnumConstant * enumConstants{ nullptr };
    VarCallbacksAny      optionalCallbacks{};
    Float64              valueMin{ 0.0 };
    Float64              valueMax{ 1.0 };
    Float64              step{ 1.0 };
    NumberFormat         numberFmt{ NumberFormat::Decimal };
    bool                 clamped{ false }; // If true clamps to [valueMin,valueMax]
    bool                 readOnly{ false };
};

// ========================================================
// class PanelImpl:
// ========================================================

class PanelImpl final : public Panel
{
public:

    PanelImpl();
    ~PanelImpl();

    void init(GUIImpl * myGUI, const char * myName);

    bool onKeyPressed(KeyCode key, KeyModFlags modifiers);
    bool onMouseButton(MouseButton button, int clicks);
    bool onMouseMotion(int mx, int my);
    bool onMouseScroll(int yScroll);
    void onFrameRender(GeometryBatch & geoBatch, bool forceRefresh);

    void setMinimized(bool minimized) { window.setMinimized(minimized); }
    void setVisible(bool visible)     { window.setVisible(visible); }

    void setUIScaling(Float32 s)   { window.setScaling(s); }
    void setTextScaling(Float32 s) { window.setTextScaling(s); }

    WindowWidget * getWindow() { return &window; }

    Variable * addVariableRO(VariableType type, Variable * parent, const char * name, const void * var,
                             int elementCount = 1, const EnumConstant * enumConstants = nullptr) override;

    Variable * addVariableRW(VariableType type, Variable * parent, const char * name, void * var,
                             int elementCount = 1, const EnumConstant * enumConstants = nullptr) override;

    Variable * addVariableCB(VariableType type, Variable * parent, const char * name, const VarCallbacksAny & callbacks,
                             VarAccess access, int elementCount = 1, const EnumConstant * enumConstants = nullptr) override;

    Variable * addHierarchyParent(const char * name) override;
    Variable * addHierarchyParent(Variable * parent, const char * name) override;

    int getPositionX() const override { return window.getRect().getX(); }
    int getPositionY() const override { return window.getRect().getY(); }

    int getWidth()  const override { return window.getRect().getWidth(); }
    int getHeight() const override { return window.getRect().getHeight(); }

    Panel * setPosition(int newPosX, int newPosY) override;
    Panel * setSize(int newWidth, int newHeight) override;

    Variable * findVariable(const char * varName) const override;
    Variable * findVariable(std::uint32_t varNameHashCode) const override;
    bool destroyVariable(Variable * variable) override;
    void destroyAllVariables() override;
    int getVariablesCount() const override;
    void enumerateAllVariables(VariableEnumerateCallback enumCallback, void * userContext) override;

    const GUI * getGUI() const override { return window.getGUI(); }
    GUI * getGUI() override { return window.getGUI(); }

    Panel * setName(const char * newName) override;
    const char * getName() const override { return window.getTitle(); }
    std::uint32_t getHashCode() const override { return hashCode; }

private:

    std::uint32_t hashCode{ 0 }; // Hash of name/window title for fast lookup.
    PODArray      variables{ sizeof(VariableImpl *) };
    WindowWidget  window{};
};

// ========================================================
// class GUIImpl:
// ========================================================

class GUIImpl final : public GUI
{
public:

    GUIImpl();
    ~GUIImpl();

    void init(const char * myName);

    Panel * findPanel(const char * panelName) const override;
    Panel * findPanel(std::uint32_t panelNameHashCode) const override;
    Panel * createPanel(const char * panelName) override;
    bool destroyPanel(Panel * panel) override;
    void destroyAllPanels() override;
    int getPanelCount() const override;
    void enumerateAllPanels(PanelEnumerateCallback enumCallback, void * userContext) override;

    bool onKeyPressed(KeyCode key, KeyModFlags modifiers) override;
    bool onMouseButton(MouseButton button, int clicks) override;
    bool onMouseMotion(int mx, int my) override;
    bool onMouseScroll(int yScroll) override;
    void onFrameRender(bool forceRefresh = false) override;

    void minimizeAllPanels() override;
    void maximizeAllPanels() override;
    void hideAllPanels() override;
    void showAllPanels() override;

    void setGlobalUIScaling(Float32 scaling) override;
    void setGlobalTextScaling(Float32 scaling) override;
    Float32 getGlobalUIScaling() const override { return globalUIScaling; }
    Float32 getGlobalTextScaling() const override { return globalTextScaling; }

    void setName(const char * newName) { name = newName; hashCode = hashString(newName); }
    const char * getName() const override { return name.c_str(); }
    std::uint32_t getHashCode() const override { return hashCode; }

private:

    std::uint32_t hashCode{ 0 }; // Hash of name for fast lookup.
    SmallStr      name{};
    PODArray      panels{ sizeof(PanelImpl *) };
    GeometryBatch geoBatch{};
    Float32       globalUIScaling{ 1.0f };
    Float32       globalTextScaling{ 1.0f };
};

} // namespace ntb {}
