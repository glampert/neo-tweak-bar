
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
// class VariableImpl:
// ========================================================

VariableImpl::VariableImpl()
{
}

VariableImpl::~VariableImpl()
{
}

void VariableImpl::init(GUI * myGUI, Panel * myPanel, const char * myName)
{
    gui   = myGUI;
    panel = myPanel;
    setName(myName);
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

void PanelImpl::init(GUI * myGUI, const char * myName)
{
    gui = myGUI;
    setName(myName);
}

VariableImpl * PanelImpl::newVariable(Variable * parent, const char * name)
{
    NTB_ASSERT(name != nullptr);
    VariableImpl * var = construct(implAllocT<VariableImpl>());
    var->init(gui, this, name);
    variables.pushBack(var);

    // TODO: Implement parenting
    (void)parent;

    return var;
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

} // namespace ntb {}
