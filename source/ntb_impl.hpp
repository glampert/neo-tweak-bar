
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

// ========================================================
// class VariableImpl:
// ========================================================

class VariableImpl final : public Variable
{
public:

    VariableImpl();
    ~VariableImpl();

    void init(GUI * myGUI, Panel * myPanel, const char * myName);

    Variable * setName(const char * newName) override { name = newName; hashCode = hashString(newName); return this; }
    const char * getName() const override { return name.c_str(); }
    std::uint32_t getHashCode() const override { return hashCode; }

    const GUI * getGUI() const override { return gui; }
    GUI * getGUI() override { return gui; }

    const Panel * getPanel() const override { return panel; }
    Panel * getPanel() override { return panel; }

private:

    GUI   *       gui{ nullptr };
    Panel *       panel{ nullptr };
    std::uint32_t hashCode{ 0 };
    SmallStr      name;
};

// ========================================================
// class PanelImpl:
// ========================================================

class PanelImpl final : public Panel
{
public:

    PanelImpl();
    ~PanelImpl();

    void init(GUI * myGUI, const char * myName);

    // Boolean variables:
    Variable * addBoolRO(const char * name, const bool * var) override { return newVariable(nullptr, name); }
    Variable * addBoolRO(Variable * parent, const char * name, const bool * var) override { return newVariable(parent, name); }
    Variable * addBoolRW(const char * name, bool * var) override { return newVariable(nullptr, name); }
    Variable * addBoolRW(Variable * parent, const char * name, bool * var) override { return newVariable(parent, name); }
    Variable * addBoolRO(const char * name, const VarCallbacksAny & callbacks) override { return newVariable(nullptr, name); }
    Variable * addBoolRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks) override { return newVariable(parent, name); }
    Variable * addBoolRW(const char * name, const VarCallbacksAny & callbacks) override { return newVariable(nullptr, name); }
    Variable * addBoolRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks) override { return newVariable(parent, name); }

    // Char variables:
    Variable * addCharRO(const char * name, const char * var) override { return newVariable(nullptr, name); }
    Variable * addCharRO(Variable * parent, const char * name, const char * var) override { return newVariable(parent, name); }
    Variable * addCharRW(const char * name, char * var) override { return newVariable(nullptr, name); }
    Variable * addCharRW(Variable * parent, const char * name, char * var) override { return newVariable(parent, name); }
    Variable * addCharRO(const char * name, const VarCallbacksAny & callbacks) override { return newVariable(nullptr, name); }
    Variable * addCharRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks) override { return newVariable(parent, name); }
    Variable * addCharRW(const char * name, const VarCallbacksAny & callbacks) override { return newVariable(nullptr, name); }
    Variable * addCharRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks) override { return newVariable(parent, name); }

    // Integer/float number variables:
    Variable * addNumberRO(const char * name, const std::int8_t * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRO(Variable * parent, const char * name, const std::int8_t * var) override { return newVariable(parent, name); }
    Variable * addNumberRW(const char * name, std::int8_t * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRW(Variable * parent, const char * name, std::int8_t * var) override { return newVariable(parent, name); }
    Variable * addNumberRO(const char * name, const std::uint8_t * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRO(Variable * parent, const char * name, const std::uint8_t * var) override { return newVariable(parent, name); }
    Variable * addNumberRW(const char * name, std::uint8_t * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRW(Variable * parent, const char * name, std::uint8_t * var) override { return newVariable(parent, name); }
    Variable * addNumberRO(const char * name, const std::int16_t * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRO(Variable * parent, const char * name, const std::int16_t * var) override { return newVariable(parent, name); }
    Variable * addNumberRW(const char * name, std::int16_t * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRW(Variable * parent, const char * name, std::int16_t * var) override { return newVariable(parent, name); }
    Variable * addNumberRO(const char * name, const std::uint16_t * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRO(Variable * parent, const char * name, const std::uint16_t * var) override { return newVariable(parent, name); }
    Variable * addNumberRW(const char * name, std::uint16_t * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRW(Variable * parent, const char * name, std::uint16_t * var) override { return newVariable(parent, name); }
    Variable * addNumberRO(const char * name, const std::int32_t * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRO(Variable * parent, const char * name, const std::int32_t * var) override { return newVariable(parent, name); }
    Variable * addNumberRW(const char * name, std::int32_t * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRW(Variable * parent, const char * name, std::int32_t * var) override { return newVariable(parent, name); }
    Variable * addNumberRO(const char * name, const std::uint32_t * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRO(Variable * parent, const char * name, const std::uint32_t * var) override { return newVariable(parent, name); }
    Variable * addNumberRW(const char * name, std::uint32_t * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRW(Variable * parent, const char * name, std::uint32_t * var) override { return newVariable(parent, name); }
    Variable * addNumberRO(const char * name, const std::int64_t * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRO(Variable * parent, const char * name, const std::int64_t * var) override { return newVariable(parent, name); }
    Variable * addNumberRW(const char * name, std::int64_t * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRW(Variable * parent, const char * name, std::int64_t * var) override { return newVariable(parent, name); }
    Variable * addNumberRO(const char * name, const std::uint64_t * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRO(Variable * parent, const char * name, const std::uint64_t * var) override { return newVariable(parent, name); }
    Variable * addNumberRW(const char * name, std::uint64_t * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRW(Variable * parent, const char * name, std::uint64_t * var) override { return newVariable(parent, name); }
    Variable * addNumberRO(const char * name, const Float32 * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRO(Variable * parent, const char * name, const Float32 * var) override { return newVariable(parent, name); }
    Variable * addNumberRW(const char * name, Float32 * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRW(Variable * parent, const char * name, Float32 * var) override { return newVariable(parent, name); }
    Variable * addNumberRO(const char * name, const Float64 * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRO(Variable * parent, const char * name, const Float64 * var) override { return newVariable(parent, name); }
    Variable * addNumberRW(const char * name, Float64 * var) override { return newVariable(nullptr, name); }
    Variable * addNumberRW(Variable * parent, const char * name, Float64 * var) override { return newVariable(parent, name); }
    Variable * addNumberRO(const char * name, const VarCallbacksAny & callbacks) override { return newVariable(nullptr, name); }
    Variable * addNumberRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks) override { return newVariable(parent, name); }
    Variable * addNumberRW(const char * name, const VarCallbacksAny & callbacks) override { return newVariable(nullptr, name); }
    Variable * addNumberRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks) override { return newVariable(parent, name); }

    // Hexadecimal pointer value:
    Variable * addPointerRO(const char * name, void * const * ptr) override { return newVariable(nullptr, name); }
    Variable * addPointerRO(Variable * parent, const char * name, void * const * ptr) override { return newVariable(parent, name); }
    Variable * addPointerRW(const char * name, void ** ptr) override { return newVariable(nullptr, name); }
    Variable * addPointerRW(Variable * parent, const char * name, void ** ptr) override { return newVariable(parent, name); }
    Variable * addPointerRO(const char * name, const VarCallbacksAny & callbacks) override { return newVariable(nullptr, name); }
    Variable * addPointerRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks) override { return newVariable(parent, name); }
    Variable * addPointerRW(const char * name, const VarCallbacksAny & callbacks) override { return newVariable(nullptr, name); }
    Variable * addPointerRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks) override { return newVariable(parent, name); }

    // Generic float vectors (2,3,4 elements):
    Variable * addFloatVecRO(const char * name, const Float32 * vec, int size) override { return newVariable(nullptr, name); }
    Variable * addFloatVecRO(Variable * parent, const char * name, const Float32 * vec, int size) override { return newVariable(parent, name); }
    Variable * addFloatVecRW(const char * name, Float32 * vec, int size) override { return newVariable(nullptr, name); }
    Variable * addFloatVecRW(Variable * parent, const char * name, Float32 * vec, int size) override { return newVariable(parent, name); }
    Variable * addFloatVecRO(const char * name, const VarCallbacksAny & callbacks, int size) override { return newVariable(nullptr, name); }
    Variable * addFloatVecRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks, int size) override { return newVariable(parent, name); }
    Variable * addFloatVecRW(const char * name, const VarCallbacksAny & callbacks, int size) override { return newVariable(nullptr, name); }
    Variable * addFloatVecRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks, int size) override { return newVariable(parent, name); }

    // Direction vector (3 elements):
    Variable * addDirectionVecRO(const char * name, const Float32 * vec) override { return newVariable(nullptr, name); }
    Variable * addDirectionVecRO(Variable * parent, const char * name, const Float32 * vec) override { return newVariable(parent, name); }
    Variable * addDirectionVecRW(const char * name, Float32 * vec) override { return newVariable(nullptr, name); }
    Variable * addDirectionVecRW(Variable * parent, const char * name, Float32 * vec) override { return newVariable(parent, name); }
    Variable * addDirectionVecRO(const char * name, const VarCallbacksAny & callbacks) override { return newVariable(nullptr, name); }
    Variable * addDirectionVecRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks) override { return newVariable(parent, name); }
    Variable * addDirectionVecRW(const char * name, const VarCallbacksAny & callbacks) override { return newVariable(nullptr, name); }
    Variable * addDirectionVecRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks) override { return newVariable(parent, name); }

    // Rotation quaternions (4 elements):
    Variable * addRotationQuatRO(const char * name, const Float32 * quat) override { return newVariable(nullptr, name); }
    Variable * addRotationQuatRO(Variable * parent, const char * name, const Float32 * quat) override { return newVariable(parent, name); }
    Variable * addRotationQuatRW(const char * name, Float32 * quat) override { return newVariable(nullptr, name); }
    Variable * addRotationQuatRW(Variable * parent, const char * name, Float32 * quat) override { return newVariable(parent, name); }
    Variable * addRotationQuatRO(const char * name, const VarCallbacksAny & callbacks) override { return newVariable(nullptr, name); }
    Variable * addRotationQuatRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks) override { return newVariable(parent, name); }
    Variable * addRotationQuatRW(const char * name, const VarCallbacksAny & callbacks) override { return newVariable(nullptr, name); }
    Variable * addRotationQuatRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks) override { return newVariable(parent, name); }

    // Byte-sized color components [0,255]:
    Variable * addColorRO(const char * name, const std::uint8_t * color, int size) override { return newVariable(nullptr, name); }
    Variable * addColorRO(Variable * parent, const char * name, const std::uint8_t * color, int size) override { return newVariable(parent, name); }
    Variable * addColorRW(const char * name, std::uint8_t * color, int size) override { return newVariable(nullptr, name); }
    Variable * addColorRW(Variable * parent, const char * name, std::uint8_t * color, int size) override { return newVariable(parent, name); }

    // Floating-point color components [0,1]:
    Variable * addColorRO(const char * name, const Float32 * color, int size) override { return newVariable(nullptr, name); }
    Variable * addColorRO(Variable * parent, const char * name, const Float32 * color, int size) override { return newVariable(parent, name); }
    Variable * addColorRW(const char * name, Float32 * color, int size) override { return newVariable(nullptr, name); }
    Variable * addColorRW(Variable * parent, const char * name, Float32 * color, int size) override { return newVariable(parent, name); }

    // Integer-packed 32 bits ARGB color:
    Variable * addColorRO(const char * name, const Color32 * color) override { return newVariable(nullptr, name); }
    Variable * addColorRO(Variable * parent, const char * name, const Color32 * color) override { return newVariable(parent, name); }
    Variable * addColorRW(const char * name, Color32 * color) override { return newVariable(nullptr, name); }
    Variable * addColorRW(Variable * parent, const char * name, Color32 * color) override { return newVariable(parent, name); }

    // Colors from callbacks:
    Variable * addColorRO(const char * name, const VarCallbacksAny & callbacks, int size) override { return newVariable(nullptr, name); }
    Variable * addColorRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks, int size) override { return newVariable(parent, name); }
    Variable * addColorRW(const char * name, const VarCallbacksAny & callbacks, int size) override { return newVariable(nullptr, name); }
    Variable * addColorRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks, int size) override { return newVariable(parent, name); }

    // String variables:
    Variable * addStringRO(const char * name, const char * str) override { return newVariable(nullptr, name); }
    Variable * addStringRO(Variable * parent, const char * name, const char * str) override { return newVariable(parent, name); }
    Variable * addStringRW(const char * name, char * buffer, int bufferSize) override { return newVariable(nullptr, name); }
    Variable * addStringRW(Variable * parent, const char * name, char * buffer, int bufferSize) override { return newVariable(parent, name); }

    // By pointer to std::string (optional interface):
    #if NEO_TWEAK_BAR_STD_STRING_INTEROP
    Variable * addStringRO(const char * name, const std::string * str) override { return newVariable(nullptr, name); }
    Variable * addStringRO(Variable * parent, const char * name, const std::string * str) override { return newVariable(parent, name); }
    Variable * addStringRW(const char * name, std::string * str) override { return newVariable(nullptr, name); }
    Variable * addStringRW(Variable * parent, const char * name, std::string * str) override { return newVariable(parent, name); }
    #endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

    // Strings from callbacks:
    Variable * addStringRO(const char * name, const VarCallbacksAny & callbacks) override { return newVariable(nullptr, name); }
    Variable * addStringRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks) override { return newVariable(parent, name); }
    Variable * addStringRW(const char * name, const VarCallbacksAny & callbacks) override { return newVariable(nullptr, name); }
    Variable * addStringRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks) override { return newVariable(parent, name); }

    // User-defined enums (constants are not copied):
    Variable * addEnumRO(const char * name, const void * var, const EnumConstant * constants, int numOfConstants) override { return newVariable(nullptr, name); }
    Variable * addEnumRO(Variable * parent, const char * name, const void * var, const EnumConstant * constants, int numOfConstants) override { return newVariable(parent, name); }
    Variable * addEnumRW(const char * name, void * var, const EnumConstant * constants, int numOfConstants) override { return newVariable(nullptr, name); }
    Variable * addEnumRW(Variable * parent, const char * name, void * var, const EnumConstant * constants, int numOfConstants) override { return newVariable(parent, name); }
    Variable * addEnumRO(const char * name, const VarCallbacksAny & callbacks, const EnumConstant * constants, int numOfConstants) override { return newVariable(nullptr, name); }
    Variable * addEnumRO(Variable * parent, const char * name, const VarCallbacksAny & callbacks, const EnumConstant * constants, int numOfConstants) override { return newVariable(parent, name); }
    Variable * addEnumRW(const char * name, const VarCallbacksAny & callbacks, const EnumConstant * constants, int numOfConstants) override { return newVariable(nullptr, name); }
    Variable * addEnumRW(Variable * parent, const char * name, const VarCallbacksAny & callbacks, const EnumConstant * constants, int numOfConstants) override { return newVariable(parent, name); }

    // TODO: Implement
    Variable * addHierarchyParent(const char * name) override { return nullptr; }
    Variable * addHierarchyParent(Variable * parent, const char * name) override { return nullptr; }

    // TODO: Implement
    int getPositionX() const override { return 0; }
    int getPositionY() const override { return 0; }
    int getWidth() const override { return 0; }
    int getHeight() const override { return 0; }
    Panel * setPosition(int newPosX, int newPosY) override { return this; }
    Panel * setSize(int newWidth, int newHeight) override { return this; }

    Variable * findVariable(const char * varName) const override;
    Variable * findVariable(std::uint32_t varNameHashCode) const override;
    bool destroyVariable(Variable * variable) override;
    void destroyAllVariables() override;
    int getVariablesCount() const override;
    void enumerateAllVariables(VariableEnumerateCallback enumCallback, void * userContext) override;

    const GUI * getGUI() const override { return gui; }
    GUI * getGUI() override { return gui; }

    Panel * setName(const char * newName) override { name = newName; hashCode = hashString(newName); return this; }
    const char * getName() const override { return name.c_str(); }
    std::uint32_t getHashCode() const override { return hashCode; }

private:

    VariableImpl * newVariable(Variable * parent, const char * name);

    GUI *         gui{ nullptr };
    std::uint32_t hashCode{ 0 };
    SmallStr      name;
    PODArray      variables{ sizeof(VariableImpl *) };
};

// ========================================================
// class GUIImpl:
// ========================================================

class GUIImpl final : public GUI
{
public:

    GUIImpl();
    ~GUIImpl();

    Panel * findPanel(const char * panelName) const override;
    Panel * findPanel(std::uint32_t panelNameHashCode) const override;
    Panel * createPanel(const char * panelName) override;
    bool destroyPanel(Panel * panel) override;
    void destroyAllPanels() override;
    int getPanelCount() const override;
    void enumerateAllPanels(PanelEnumerateCallback enumCallback, void * userContext) override;

    // TODO: Implement
    bool onKeyPressed(KeyCode key, KeyModFlags modifiers) override { return false; }
    bool onMouseButton(MouseButton button, int clicks) override { return false; }
    bool onMouseMotion(int mx, int my) override { return false; }
    bool onMouseScroll(int yScroll) override { return false; }
    void onFrameRender(bool forceRefresh = false) override {}

    // TODO: Implement
    void minimizeAllPanels() override {}
    void maximizeAllPanels() override {}
    void hideAllPanels() override {}
    void showAllPanels() override {}

    // TODO: Implement
    void setGlobalUIScaling(Float32 scaling) override {}
    void setGlobalTextScaling(Float32 scaling) override {}
    Float32 getGlobalUIScaling() const override { return 1.0f; }
    Float32 getGlobalTextScaling() const override { return 1.0f; }

    void setName(const char * newName) { name = newName; hashCode = hashString(newName); }
    const char * getName() const override { return name.c_str(); }
    std::uint32_t getHashCode() const override { return hashCode; }

private:

    std::uint32_t hashCode{ 0 };
    SmallStr      name;
    PODArray      panels{ sizeof(PanelImpl *) };
};

} // namespace ntb {}

#endif // NTB_IMPL_HPP
