
// ================================================================================================
// -*- C++ -*-
// File: ntb_impl.hpp
// Author: Guilherme R. Lampert
// Created on: 26/09/16
// Brief: Private implementations for the public NTB interfaces.
// ================================================================================================

#ifndef NTB_IMPL_HPP
#define NTB_IMPL_HPP

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

    void init(PanelImpl * myPanel, Variable * myParent, const char * myName, bool readOnly, Variable::Type varType,
              void * varData, int elementCount, const EnumConstant * enumConstants, const VarCallbacksAny * optionalCallbacks);

    Type getType() const override;
    bool isReadOnly() const override;

    Variable * setName(const char * newName) override;
    const char * getName() const override;
    std::uint32_t getHashCode() const override;

    const GUI * getGUI() const override;
    GUI * getGUI() override;

    const Panel * getPanel() const override;
    Panel * getPanel() override;

protected:

    // VarDisplayWidget overrides:
    void drawVarValue(GeometryBatch & geoBatch) const override;

private:

    PanelImpl *          panel{ nullptr };
    std::uint32_t        hashCode{ 0 };
    SmallStr             name;
    bool                 readOnly{ false };
    int                  titleWidth{ 0 };
    int                  elementCount{ 0 };
    Variable::Type       varType{ Variable::Type::Undefined };
    void               * varData{ nullptr };
    const EnumConstant * enumConstants{ nullptr };
    VarCallbacksAny      optionalCallbacks;
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

    Variable * addVariableRO(VarType type, Variable * parent, const char * name, const void * var,
                             int elementCount = 1, const EnumConstant * enumConstants = nullptr) override;

    Variable * addVariableRW(VarType type, Variable * parent, const char * name, void * var,
                             int elementCount = 1, const EnumConstant * enumConstants = nullptr) override;

    Variable * addVariableCB(VarType type, Variable * parent, const char * name, const VarCallbacksAny & callbacks,
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

    Panel * setName(const char * newName) override { name = newName; hashCode = hashString(newName); return this; }
    const char * getName() const override { return name.c_str(); }
    std::uint32_t getHashCode() const override { return hashCode; }

private:

    std::uint32_t hashCode{ 0 };
    SmallStr      name;
    PODArray      variables{ sizeof(VariableImpl *) };
    WindowWidget  window;
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

    std::uint32_t hashCode{ 0 };
    SmallStr      name;
    PODArray      panels{ sizeof(PanelImpl *) };
    GeometryBatch geoBatch;
    Float32       globalUIScaling{ 1.0f };
    Float32       globalTextScaling{ 1.0f };
};

} // namespace ntb {}

#endif // NTB_IMPL_HPP
