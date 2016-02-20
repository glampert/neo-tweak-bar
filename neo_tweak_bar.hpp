
// ================================================================================================
// -*- C++ -*-
// File: neo_tweak_bar.hpp
// Author: Guilherme R. Lampert
// Created on: 07/01/16
// Brief: Neo Tweak Bar - a lightweight and intuitive C++ GUI library for graphics applications.
// ================================================================================================

#ifndef NEO_TWEAK_BAR_HPP
#define NEO_TWEAK_BAR_HPP

#include "ntb_utils.hpp"
#include "ntb_render_interface.hpp"
#include "ntb_geometry_batch.hpp"
#include "ntb_widgets.hpp"

//TEMP placeholder
struct StackAllocator { };

namespace ntb
{

// ================================================================================================
//
//                                   Main public NTB interface
//
// ================================================================================================

class Variable;
class Panel;
class GUI;

// ========================================================
// Miscellaneous helper structures:
// ========================================================

// Constant value + name string pair for Panel::addEnum().
// The string is not copied, so be sure to use a literal
// const char* string for 'name'.
template<class T>
struct EnumConstant
{
    const char * name;
    const T value;
};

// Numerical bases for Variable::setNumberFormatting().
struct NumFormat
{
    enum Enum
    {
        Binary      = 2,
        Octal       = 8,
        Decimal     = 10,
        Hexadecimal = 16
    };
};

// ========================================================
// class Variable:
//
// Variables can be added to Panels to display user
// data that can be tweaked in the UI. A variable can
// also be display-only.
// ========================================================

class Variable
    : public ListNode, protected VarDisplayWidget
{
    // Panel is the Variable factory. No direct instantiation is allowed.
    friend Panel;

public:

    // The Panel methods ending in 'RO' create ReadOnly Variables
    // while the ones ending in 'RW' create ReadWrite Variables.
    enum Access { ReadOnly, ReadWrite };

    // Accessors:
    virtual Access getAccessMode() const = 0;
    const SmallStr & getName() const;

    // Styling methods:
    virtual Variable * setNumberFormatting(int numericBase) = 0;
    virtual Variable * setMaxStringSize(int maxSizeIncludingNulTerminator) = 0;

    // Since we use protected inheritance from VarDisplayWidget,
    // a cast can only be done from within Variable.
    const VarDisplayWidget * getVarDisplayWidget() const;
    VarDisplayWidget * getVarDisplayWidget();

protected:

    Variable(Panel * owner, Variable * parent, const char * name);
    virtual ~Variable();

    // Variable impl methods:
    virtual void onUpdateCachedValue() const = 0;
    virtual void onUpdateUserValue()   const = 0;
    virtual void onLinkedToPanel(Panel & owner) = 0;

    // VarDisplayWidget methods:
    virtual void onDraw(GeometryBatch & geoBatch) const NTB_OVERRIDE = 0;
};

// ========================================================
// class Panel:
//
// The Panel class is the chief Neo Tweak Bar interface.
// Use a Panel to add your tweakable variables to the UI.
//
// Variables can be added from pointers to user data
// or callbacks to get/set the variable value.
// ========================================================

class Panel NTB_FINAL_CLASS
    : public ListNode, protected WindowWidget
{
    NTB_DISABLE_COPY_ASSIGN(Panel);

    // GUI is the Panel factory. No direct instantiation is allowed.
    friend GUI;

public:

    // ------------------------------------------
    // Booleans
    // ------------------------------------------

    Variable * addBoolRO(const char * name, const bool * var);
    Variable * addBoolRO(Variable * parent, const char * name, const bool * var);
    Variable * addBoolRW(const char * name, bool * var);
    Variable * addBoolRW(Variable * parent, const char * name, bool * var);

    template<class OT, class CBT>
    Variable * addBoolRO(const char * name, const OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addBoolRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addBoolRW(const char * name, OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addBoolRW(Variable * parent, const char * name, OT * obj, const CBT & cbs);

    // ------------------------------------------
    // Single character
    // ------------------------------------------

    Variable * addCharRO(const char * name, const char * var);
    Variable * addCharRO(Variable * parent, const char * name, const char * var);
    Variable * addCharRW(const char * name, char * var);
    Variable * addCharRW(Variable * parent, const char * name, char * var);

    template<class OT, class CBT>
    Variable * addCharRO(const char * name, const OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addCharRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addCharRW(const char * name, OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addCharRW(Variable * parent, const char * name, OT * obj, const CBT & cbs);

    // ------------------------------------------
    // Numeric types
    // Integer and floating-point
    // ------------------------------------------

    Variable * addNumberRO(const char * name, const Int8 * var);
    Variable * addNumberRO(Variable * parent, const char * name, const Int8 * var);
    Variable * addNumberRW(const char * name, Int8 * var);
    Variable * addNumberRW(Variable * parent, const char * name, Int8 * var);

    Variable * addNumberRO(const char * name, const UInt8 * var);
    Variable * addNumberRO(Variable * parent, const char * name, const UInt8 * var);
    Variable * addNumberRW(const char * name, UInt8 * var);
    Variable * addNumberRW(Variable * parent, const char * name, UInt8 * var);

    Variable * addNumberRO(const char * name, const Int16 * var);
    Variable * addNumberRO(Variable * parent, const char * name, const Int16 * var);
    Variable * addNumberRW(const char * name, Int16 * var);
    Variable * addNumberRW(Variable * parent, const char * name, Int16 * var);

    Variable * addNumberRO(const char * name, const UInt16 * var);
    Variable * addNumberRO(Variable * parent, const char * name, const UInt16 * var);
    Variable * addNumberRW(const char * name, UInt16 * var);
    Variable * addNumberRW(Variable * parent, const char * name, UInt16 * var);

    Variable * addNumberRO(const char * name, const Int32 * var);
    Variable * addNumberRO(Variable * parent, const char * name, const Int32 * var);
    Variable * addNumberRW(const char * name, Int32 * var);
    Variable * addNumberRW(Variable * parent, const char * name, Int32 * var);

    Variable * addNumberRO(const char * name, const UInt32 * var);
    Variable * addNumberRO(Variable * parent, const char * name, const UInt32 * var);
    Variable * addNumberRW(const char * name, UInt32 * var);
    Variable * addNumberRW(Variable * parent, const char * name, UInt32 * var);

    Variable * addNumberRO(const char * name, const Int64 * var);
    Variable * addNumberRO(Variable * parent, const char * name, const Int64 * var);
    Variable * addNumberRW(const char * name, Int64 * var);
    Variable * addNumberRW(Variable * parent, const char * name, Int64 * var);

    Variable * addNumberRO(const char * name, const UInt64 * var);
    Variable * addNumberRO(Variable * parent, const char * name, const UInt64 * var);
    Variable * addNumberRW(const char * name, UInt64 * var);
    Variable * addNumberRW(Variable * parent, const char * name, UInt64 * var);

    Variable * addNumberRO(const char * name, const float * var);
    Variable * addNumberRO(Variable * parent, const char * name, const float * var);
    Variable * addNumberRW(const char * name, float * var);
    Variable * addNumberRW(Variable * parent, const char * name, float * var);

    Variable * addNumberRO(const char * name, const double * var);
    Variable * addNumberRO(Variable * parent, const char * name, const double * var);
    Variable * addNumberRW(const char * name, double * var);
    Variable * addNumberRW(Variable * parent, const char * name, double * var);

    template<class OT, class CBT>
    Variable * addNumberRO(const char * name, const OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addNumberRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addNumberRW(const char * name, OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addNumberRW(Variable * parent, const char * name, OT * obj, const CBT & cbs);

    // ------------------------------------------
    // Generic vectors of floats
    // Size = 2,3,4 elements
    //  V={X,Y}
    //  V={X,Y,Z}
    //  V={X,Y,Z,W}
    // ------------------------------------------

    template<int Size>
    Variable * addFloatVecRO(const char * name, const float * vec);
    template<int Size>
    Variable * addFloatVecRO(Variable * parent, const char * name, const float * vec);
    template<int Size>
    Variable * addFloatVecRW(const char * name, float * vec);
    template<int Size>
    Variable * addFloatVecRW(Variable * parent, const char * name, float * vec);

    template<int Size, class OT, class CBT>
    Variable * addFloatVecRO(const char * name, const OT * obj, const CBT & cbs);
    template<int Size, class OT, class CBT>
    Variable * addFloatVecRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs);
    template<int Size, class OT, class CBT>
    Variable * addFloatVecRW(const char * name, OT * obj, const CBT & cbs);
    template<int Size, class OT, class CBT>
    Variable * addFloatVecRW(Variable * parent, const char * name, OT * obj, const CBT & cbs);

    // ------------------------------------------
    // Direction vectors - 3 floats
    //  D={X,Y,Z}
    // ------------------------------------------

    Variable * addDirectionVecRO(const char * name, const float * vec);
    Variable * addDirectionVecRO(Variable * parent, const char * name, const float * vec);
    Variable * addDirectionVecRW(const char * name, float * vec);
    Variable * addDirectionVecRW(Variable * parent, const char * name, float * vec);

    template<class OT, class CBT>
    Variable * addDirectionVecRO(const char * name, const OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addDirectionVecRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addDirectionVecRW(const char * name, OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addDirectionVecRW(Variable * parent, const char * name, OT * obj, const CBT & cbs);

    // ------------------------------------------
    // Rotation quaternions - 4 floats
    //  Q={X,Y,Z,W}
    // ------------------------------------------

    Variable * addRotationQuatRO(const char * name, const float * quat);
    Variable * addRotationQuatRO(Variable * parent, const char * name, const float * quat);
    Variable * addRotationQuatRW(const char * name, float * quat);
    Variable * addRotationQuatRW(Variable * parent, const char * name, float * quat);

    template<class OT, class CBT>
    Variable * addRotationQuatRO(const char * name, const OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addRotationQuatRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addRotationQuatRW(const char * name, OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addRotationQuatRW(Variable * parent, const char * name, OT * obj, const CBT & cbs);

    // ------------------------------------------
    // Color values
    // Size  = 3 or 4 components
    // Range = [0,255] or [0.0,1.0]
    //  C={R,G,B}
    //  C={R,G,B,A}
    // ------------------------------------------

    // Byte-sized color components:
    template<int Size>
    Variable * addColorRO(const char * name, const UByte * clr);
    template<int Size>
    Variable * addColorRO(Variable * parent, const char * name, const UByte * clr);
    template<int Size>
    Variable * addColorRW(const char * name, UByte * clr);
    template<int Size>
    Variable * addColorRW(Variable * parent, const char * name, UByte * clr);

    // Floating-point color components:
    template<int Size>
    Variable * addColorRO(const char * name, const float * clr);
    template<int Size>
    Variable * addColorRO(Variable * parent, const char * name, const float * clr);
    template<int Size>
    Variable * addColorRW(const char * name, float * clr);
    template<int Size>
    Variable * addColorRW(Variable * parent, const char * name, float * clr);

    // NTB Color32 packed 32-bits color:
    Variable * addColorRO(const char * name, const Color32 * clr);
    Variable * addColorRO(Variable * parent, const char * name, const Color32 * clr);
    Variable * addColorRW(const char * name, Color32 * clr);
    Variable * addColorRW(Variable * parent, const char * name, Color32 * clr);

    // Colors from callbacks:
    template<int Size, class OT, class CBT>
    Variable * addColorRO(const char * name, const OT * obj, const CBT & cbs);
    template<int Size, class OT, class CBT>
    Variable * addColorRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs);
    template<int Size, class OT, class CBT>
    Variable * addColorRW(const char * name, OT * obj, const CBT & cbs);
    template<int Size, class OT, class CBT>
    Variable * addColorRW(Variable * parent, const char * name, OT * obj, const CBT & cbs);

    // ------------------------------------------
    // Text strings
    //  - char[] / const char[]
    //  - std::string (optional)
    //  - NTB SmallStr type
    // ------------------------------------------

    // The C-style string overloads of addString() are limited
    // to this many characters (counting the NUL-terminator).
    // Text fields in the panel are small, so we don't need
    // a large value in here anyways.
    enum { CStringMaxSize = 256 };

    // By pointer to char buffer (read-only):
    Variable * addStringRO(const char * name, const char * str);
    Variable * addStringRO(Variable * parent, const char * name, const char * str);

    // Pointer to fixed-size char buffer (read-write):
    template<int Size> Variable * addStringRW(const char * name, char * str);
    template<int Size> Variable * addStringRW(Variable * parent, const char * name, char * str);

    // By pointer to std::string:
    #if NEO_TWEAK_BAR_STD_STRING_INTEROP
    Variable * addStringRO(const char * name, const std::string * str);
    Variable * addStringRO(Variable * parent, const char * name, const std::string * str);
    Variable * addStringRW(const char * name, std::string * str);
    Variable * addStringRW(Variable * parent, const char * name, std::string * str);
    #endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

    // By pointer to NTB SmallStr:
    Variable * addStringRO(const char * name, const SmallStr * str);
    Variable * addStringRO(Variable * parent, const char * name, const SmallStr * str);
    Variable * addStringRW(const char * name, SmallStr * str);
    Variable * addStringRW(Variable * parent, const char * name, SmallStr * str);

    // Strings from callbacks:
    template<class OT, class CBT>
    Variable * addStringRO(const char * name, const OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addStringRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addStringRW(const char * name, OT * obj, const CBT & cbs);
    template<class OT, class CBT>
    Variable * addStringRW(Variable * parent, const char * name, OT * obj, const CBT & cbs);

    // ------------------------------------------
    // Extended/structured types
    //  - Raw pointer values
    //  - Named enum constants
    //  - User defined structures/hierarchies
    // ------------------------------------------

    // Pointer values are always displayed as hexadecimal: (Note: no callbacks variant provided)
    Variable * addPointerRO(const char * name, void * const * ptr);
    Variable * addPointerRO(Variable * parent, const char * name, void * const * ptr);
    Variable * addPointerRW(const char * name, void ** ptr);
    Variable * addPointerRW(Variable * parent, const char * name, void ** ptr);

    // Enum var from pointers: (Note: constants are not copied)
    template<class VT>
    Variable * addEnumRO(const char * name, const VT * var, const EnumConstant<VT> * constants, int numOfConstants);
    template<class VT>
    Variable * addEnumRO(Variable * parent, const char * name, const VT * var, const EnumConstant<VT> * constants, int numOfConstants);
    template<class VT>
    Variable * addEnumRW(const char * name, VT * var, const EnumConstant<VT> * constants, int numOfConstants);
    template<class VT>
    Variable * addEnumRW(Variable * parent, const char * name, VT * var, const EnumConstant<VT> * constants, int numOfConstants);

    // Enum var from callbacks: (Note: constants are not copied)
    template<class OT, class CBT, class VT>
    Variable * addEnumRO(const char * name, const OT * obj, const CBT & cbs, const EnumConstant<VT> * constants, int numOfConstants);
    template<class OT, class CBT, class VT>
    Variable * addEnumRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs, const EnumConstant<VT> * constants, int numOfConstants);
    template<class OT, class CBT, class VT>
    Variable * addEnumRW(const char * name, OT * obj, const CBT & cbs, const EnumConstant<VT> * constants, int numOfConstants);
    template<class OT, class CBT, class VT>
    Variable * addEnumRW(Variable * parent, const char * name, OT * obj, const CBT & cbs, const EnumConstant<VT> * constants, int numOfConstants);

    // User-defined hierarchy parent. Can be used to define structs/objects:
    Variable * addHierarchyParent(const char * name);
    Variable * addHierarchyParent(Variable * parent, const char * name);

    // ------------------------------------------
    // Panel management methods:
    // ------------------------------------------

    // Find existing var by name. Returns null if not found.
    // If more than one var with the same name exits, the
    // first one found is returned.
    Variable * findVariable(const char * varName) const;

    // Removes the give variable and destroys the object, invalidating the pointer.
    // If the variable is not a member of this Panel, false is returned and nothing is done.
    bool destroyVariable(Variable * var);

    // Destroys all variables from this Panel. Be aware: Calling this will
    // invalidate all Variable pointers from this Panel that you might
    // be holding on to!
    void destroyAllVariables();

    // Passes every variable attached to the Panel to user Func.
    template<class Func> void enumerateAllVariables(Func fn);
    template<class Func> void enumerateAllVariables(Func fn) const;

    // Accessors:
    int getVariablesCount() const;
    const SmallStr & getName() const;

    // Since we use protected inheritance from WindowWidget,
    // a cast can only be done from within Panel.
    const WindowWidget * getWindowWidget() const;
    WindowWidget * getWindowWidget();

    // Access the GUI this Panel belongs to.
    const GUI * getGUI() const;
    GUI * getGUI();

    // Recursive debug printing of the Panel/var hierarchy.
    #if NEO_TWEAK_BAR_DEBUG
    void printHierarchy(std::ostream & out = std::cout, const SmallStr & indent = "") const NTB_OVERRIDE;
    #endif // NEO_TWEAK_BAR_DEBUG

private:

    // Only ntb::GUI can create Panels.
    Panel(GUI * myGUI, const char * name);
    ~Panel();

    // This actually links the variable to the list.
    Variable * addVarImpl(Variable * var);

    // WindowWidget overrides:
    void onAdjustLayout() NTB_OVERRIDE;
    void onScrollContentUp() NTB_OVERRIDE;
    void onScrollContentDown() NTB_OVERRIDE;
    bool onMouseScroll(int yScroll) NTB_OVERRIDE;
    void onDraw(GeometryBatch & geoBatch) const NTB_OVERRIDE;

    IntrusiveList  variables;
    const SmallStr panelName;
};

// ========================================================
// class GUI:
//
// A Neo Tweak Bar GUI is a container that owns a set
// of UI elements like Panels and other Widgets. The GUI
// retains ownership of every UI element it creates,
// destroying all of them when it is itself finalized.
// ========================================================

class GUI NTB_FINAL_CLASS
    : public ListNode
{
    NTB_DISABLE_COPY_ASSIGN(GUI);

    // Global GUI factory functions:
    friend GUI * createGUI(const char * guiName);
    friend GUI * findGUI(const char * guiName);
    friend bool destroyGUI(GUI * gui);

public:

    // ------------------------------------------
    // Panel creation and management:
    // ------------------------------------------

    // Add new Panel or remove existing.
    Panel * createPanel(const char * panelName);
    bool destroyPanel(Panel * panel);

    // Find existing Panel by name. Returns null if Panel not found.
    // If more than one Panel with the same name exits, the first
    // one found is returned.
    Panel * findPanel(const char * panelName) const;

    // Destroys all Panels from this GUI. Be aware: Calling this
    // will invalidate all Panel pointers that you might be holding on to!
    void destroyAllPanels();

    // Number of Panels currently in the GUI.
    int getPanelCount() const;

    // Passes every Panel attached to the GUI to user Func.
    template<class Func> void enumerateAllPanels(Func fn);
    template<class Func> void enumerateAllPanels(Func fn) const;

    // ------------------------------------------
    // GUI events / callbacks:
    // ------------------------------------------

    //TODO candidate methods:
    // - minimizeAllPanels() => minimize/maximize all from the side bar
    // - maximizeAllPanels()    user can maximize them with a button
    //
    // - hideAllPanels() => completely hide the ntb UI, as if inexistent
    // - showAllPanels()    user cannot bring them back via UI.

    void onFrameRender(bool forceRefresh = false);

    // true returned if the event was consumed.

    // clicks <= 0 for button released.
    // clicks >  0 click count, e.g.: 1 single click, 2 for double click, etc.
    bool onMouseButton(MouseButton::Enum button, int clicks);

    bool onMouseMotion(int mx, int my);

    // only vertical scroll for scroll bars.
    // +Y=forward, -Y=back
    bool onMouseScroll(int yScroll);

    // ------------------------------------------
    // Miscellaneous:
    // ------------------------------------------

    // Color scheme:
    const ColorScheme * getNormalColors() const;
    const ColorScheme * getHighlightedColors() const;

    // Name of this GUI set on creation.
    const SmallStr & getName() const;

private:

    // Only createGUI() can allocate new GUIs.
    explicit GUI(const char * name);
    ~GUI();

    IntrusiveList  panels;
    GeometryBatch  geoBatch;
    const SmallStr guiName;
};

// ========================================================
// Library initialization/shutdown and GUI allocation:
// ========================================================

// Initialize Neo Tweak Bar. You must call this function once
// before you can create any ntb::GUI instances. Call shutdown()
// when you are done with the library to automatically destroy
// all remaining GUI instances.
//
// A RenderInterface and a ShellInterface implementation must
// be provided. If you pass null pointers, initialization will
// not succeed.
//
// The StackAllocator is optional. If provided, all GUIs and
// other UI element like Panels and Variables will be allocated
// from this global stack, ensuring minimal memory fragmentation.
// When not provided, we source all allocations from the callbacks
// provided by the ShellInterface.
bool initialize(RenderInterface * renderer, ShellInterface * shell,
                StackAllocator * stackAlloc = NTB_NULL);

// Performs global library shutdown.
// This will also destroy any remaining GUI instances
// that might still be active, so be sure to dispose
// any GUI pointers you might be holding on to after
// shutdown() is called.
void shutdown();

// Create a new GUI instance. Name doesn't have to be unique,
// but it must not be null nor an empty string.
GUI * createGUI(const char * guiName);

// GUIs don't have to be explicitly destroyed.
// Calling ntb::shutdown() will destroy all remaining GUIs.
bool destroyGUI(GUI * gui);

// Find existing GUI by name. Returns null if not found.
// If more than one GUI with the same name exits, the
// first one found is returned.
GUI * findGUI(const char * guiName);

// Access to the global list of GUI instances.
// Normally this should not be called by the user
// code, but we provided it anyways for debugging
// and testing, if necessary.
IntrusiveList & getGUIList();

// Retrieve the pointers set on initialization.
// The StackAllocator pointer may be null.
RenderInterface * getRenderInterface();
ShellInterface  * getShellInterface();
StackAllocator  * getStackAllocator();

// ================================================================================================
//
//                         Implementation details / internal support code
//
// ================================================================================================

namespace detail
{

//
// FIXME?
//
// I'm not really super happy with all these templates below.
// Maybe I'll just switch back to the tagged union after all...
//

#pragma pack(push, 1)

// TODO
// We always convert to->from string for displaying,
// so perhaps store a cached SmallStr of the number?
// That would trade speed for memory (SmallStr = 48 bytes)
//
// for UInt64/Int64 and double
class NumberEx NTB_FINAL_CLASS
{
public:

    enum Type
    {
        Undefined,
        SignedInt,
        UnsignedInt,
        Double,
        Pointer
    };

    // Value gets interpreted according to the type tag.
    union
    {
        Int64  asI64;
        UInt64 asU64;
        double asDouble;
        void * asPointer;
    };

    UInt8 type;   // NumberEx::Type  (as U8 to have an explicit size)
    UInt8 format; // NumFormat::Enum (as U8 to have an explicit size)

    NumberEx()
        : type(Undefined)
        , format(NumFormat::Decimal)
    {
        asU64 = 0;
    }

    SmallStr toString() const
    {
        SmallStr str;

        switch (type)
        {
        case SignedInt :
            str = SmallStr::fromNumber(asI64, format);
            break;
        case UnsignedInt :
            str = SmallStr::fromNumber(asU64, format);
            break;
        case Double :
            str = SmallStr::fromNumber(asDouble, format);
            break;
        case Pointer :
            str = SmallStr::fromPointer(asPointer, format);
            break;
        default :
            NTB_ERROR("Invalid NumberEx type!");
            str = "???";
            break;
        } // switch (type)

        return str;
    }
};

// for bool only (also holds the true/false strings)
class BoolEx NTB_FINAL_CLASS
{
public:

    enum Display { CheckMark, String };

    // Default is "true/false", but user can set to whatever (yes/no|on/off...)
    // Used when displayMode == String. NOTE: These are not copied! Use str literals only.
    const char * trueString;
    const char * falseString;

    // Value and flag stored as bytes to save as much space as possible.
    // Display mode controls how the boolean is drawn in the UI.
    // It can either be shown as a check mark or a text string.
    UInt8 displayMode;
    UInt8 value;

    BoolEx()
        : trueString("true")
        , falseString("false")
        , displayMode(CheckMark)
        , value(false)
    { }

    bool isSet() const { return static_cast<bool>(value); }
    SmallStr toString() const { return value ? trueString : falseString; }
};

// ========================================================
// class Float4Ex:
// ========================================================

// TODO Make non-inline
class Float4Ex NTB_FINAL_CLASS
{
public:

    enum Type
    {
        Undefined,
        Vec2,
        Vec3,
        Vec4,
        Dir3,
        Quat4,
        Color
    };

    float  data[4]; // X,Y,Z,W
    UInt16 type;    // For UI displaying

    Float4Ex(const Type t = Undefined)
        : type(t)
    {
        data[0] = 0.0f;
        data[1] = 0.0f;
        data[2] = 0.0f;
        data[3] = 0.0f;
    }

    void setTypeFromSize(const int size)
    {
        switch (size)
        {
        case 2  : type = Vec2; break;
        case 3  : type = Vec3; break;
        case 4  : type = Vec4; break;
        default : NTB_ERROR("Invalid Float4Ex vector size!");
        } // switch (size)
    }

    int getSize() const
    {
        switch (type)
        {
        case Vec2  : return 2;
        case Vec3  : return 3;
        case Vec4  : return 4;
        case Dir3  : return 3;
        case Quat4 : return 4;
        case Color : return 4;
        default    : return 0;
        } // switch (type)
    }

    void getFloats(float * v) const
    {
        const int size = getSize();
        for (int i = 0; i < size; ++i)
        {
            v[i] = data[i];
        }
    }

    void setFloats(const float * v)
    {
        const int size = getSize();
        for (int i = 0; i < size; ++i)
        {
            data[i] = v[i];
        }
    }

    SmallStr toString() const
    {
        SmallStr str;

        switch (type)
        {
        case Vec2 :
            str = SmallStr::fromFloatVec(data, 2, "V=");
            break;
        case Vec3 :
            str = SmallStr::fromFloatVec(data, 3, "V=");
            break;
        case Vec4 :
            str = SmallStr::fromFloatVec(data, 4, "V=");
            break;
        case Dir3 :
            str = SmallStr::fromFloatVec(data, 3, "D=");
            break;
        case Quat4 :
            str = SmallStr::fromFloatVec(data, 4, "Q=");
            break;
        case Color :
            str = SmallStr::fromFloatVec(data, 4, "C=");
            break;
        default :
            NTB_ERROR("Invalid Float4Ex type!");
            str = "???";
            break;
        } // switch (type)

        return str;
    }
};

// ========================================================
// class ColorEx:
// ========================================================

// TODO Make non-inline
class ColorEx NTB_FINAL_CLASS
{
public:

    // HLS = Hue Lightness Saturation
    enum Mode    { RGB,   HLS    };
    enum Display { CByte, CFloat };

    Float4Ex rgbaF4;
    UInt8 colorMode;      // RGB/HLS
    UInt8 displayMode;    // Display as [0,255] or [0,1]
    UInt8 numChannels;    // 3 or 4 channels
    UByte bR, bG, bB, bA; // rgbaF4 broken into components

    ColorEx()
        : rgbaF4(Float4Ex::Color)
        , colorMode(RGB)
        , displayMode(CByte)
        , numChannels(4)
    {
        // Alpha channel = opaque
        rgbaF4.data[3] = 1.0f;
        bR = bG = bB = 0;
        bA = 255;
    }

    void getColor32(Color32 & c) const
    {
        c = packColor(bR, bG, bB, bA);
    }

    void setColor32(const Color32 c)
    {
        unpackColor(c, bR, bG, bB, bA);
        rgbaF4.data[0] = byteToFloat(bR);
        rgbaF4.data[1] = byteToFloat(bG);
        rgbaF4.data[2] = byteToFloat(bB);
        rgbaF4.data[3] = byteToFloat(bA);
    }

    void getColorF(float * c) const
    {
        for (UInt32 i = 0; i < numChannels; ++i)
        {
            c[i] = rgbaF4.data[i];
        }
    }

    void setColorF(const float * c)
    {
        for (UInt32 i = 0; i < numChannels; ++i)
        {
            rgbaF4.data[i] = c[i];
        }
        bR = floatToByte(rgbaF4.data[0]);
        bG = floatToByte(rgbaF4.data[1]);
        bB = floatToByte(rgbaF4.data[2]);
        bA = floatToByte(rgbaF4.data[3]);
    }

    void getColorB(UByte * c) const
    {
        c[0] = bR;
        c[1] = bG;
        c[2] = bB;
        if (numChannels == 4) { c[3] = bA; }
    }

    void setColorB(const UByte * c)
    {
        for (UInt32 i = 0; i < numChannels; ++i)
        {
            rgbaF4.data[i] = byteToFloat(c[i]);
        }
        bR = c[0];
        bG = c[1];
        bB = c[2];
        if (numChannels == 4) { bA = c[3]; }
    }

    bool hasTransparency() const { return bA < 255; }
};

#pragma pack(pop)

//TODO put in the cpp
static const EnumConstant<UInt8> colorModeEnum[] =
{
    { "RGB", ColorEx::RGB },
    { "HLS", ColorEx::HLS }
};

static const EnumConstant<UInt8> colorDisplayEnum[] =
{
    { "[0,255]", ColorEx::CByte  },
    { "[0,1]",   ColorEx::CFloat }
};

static const char * colorModeChannelNames[] =
{
    "Red", "Green", "Blue", "Alpha",
    "Hue", "Lightness", "Saturation", "Alpha",
    NTB_NULL
};

// ========================================================
// class EnumValEx / EnumValExImpl:
// ========================================================

// This interface allows defining a single non-template
// drawVarValue() method in VarDisplayWidget.
class EnumValEx
{
public:

    virtual int getEnumValue() const = 0;
    virtual int getNumOfConsts() const = 0;
    virtual int getConstValue(int index) const = 0;
    virtual const char * getConstName(int index) const = 0;
    virtual ~EnumValEx();
};

template<class T>
class EnumValExImpl NTB_FINAL_CLASS
    : public EnumValEx
{
public:

    T value;

    EnumValExImpl()
        : value(static_cast<T>(0))
        , numOfConstants(0)
        , constants(NTB_NULL)
    { }

    void setConsts(const EnumConstant<T> * consts, const int numConsts)
    {
        NTB_ASSERT(consts != NTB_NULL);
        NTB_ASSERT(numConsts > 0);
        constants = consts;
        numOfConstants = numConsts;
    }

    int getConstValue(const int index) const NTB_OVERRIDE
    {
        NTB_ASSERT(constants != NTB_NULL);
        NTB_ASSERT(index >= 0 && index < numOfConstants);
        return static_cast<int>(constants[index].value);
    }

    const char * getConstName(const int index) const NTB_OVERRIDE
    {
        NTB_ASSERT(constants != NTB_NULL);
        NTB_ASSERT(index >= 0 && index < numOfConstants);
        return constants[index].name;
    }

    int getEnumValue()   const NTB_OVERRIDE { return static_cast<int>(value); }
    int getNumOfConsts() const NTB_OVERRIDE { return numOfConstants; }

private:

    int numOfConstants;
    const EnumConstant<T> * constants;
};

// ========================================================
// struct StripPtrRefConst:
// ========================================================

template<class T> struct StripPtrRefConst            { typedef T Type; };
template<class T> struct StripPtrRefConst<T &>       { typedef T Type; };
template<class T> struct StripPtrRefConst<T *>       { typedef T Type; };
template<class T> struct StripPtrRefConst<const T &> { typedef T Type; };
template<class T> struct StripPtrRefConst<const T *> { typedef T Type; };

// These are needed for Panel::addPointer(). We must preserve the pointer qualifier
// for the special case of a var storing the raw pointer value of a void*.
template<> struct StripPtrRefConst<      void *> { typedef       void * Type; };
template<> struct StripPtrRefConst<const void *> { typedef const void * Type; };

// ========================================================
// class VarCallbacksMemFuncByValOrRef:
// ========================================================

template<class OT, class VT>
class VarCallbacksMemFuncByValOrRef NTB_FINAL_CLASS
{
public:

    typedef typename StripPtrRefConst<OT>::Type ObjType;
    typedef typename StripPtrRefConst<VT>::Type VarType;

    typedef VT (OT::*GetCBType)() const;
    typedef void (OT::*SetCBType)(VT);

    VarCallbacksMemFuncByValOrRef(GetCBType getCb, SetCBType setCb)
        : getter(getCb)
        , setter(setCb)
    { }

    void callGetter(const ObjType * obj, VarType & valueOut) const
    {
        NTB_ASSERT(obj    != NTB_NULL);
        NTB_ASSERT(getter != NTB_NULL);
        valueOut = (obj->*getter)();
    }
    void callSetter(ObjType * obj, const VarType & valueIn) const
    {
        NTB_ASSERT(obj    != NTB_NULL);
        NTB_ASSERT(setter != NTB_NULL);
        (obj->*setter)(valueIn);
    }

    bool hasSetter() const { return setter != NTB_NULL; }

private:

    GetCBType getter;
    SetCBType setter;
};

// ========================================================
// class VarCallbacksMemFuncByPointer:
// ========================================================

template<class OT, class VT>
class VarCallbacksMemFuncByPointer NTB_FINAL_CLASS
{
public:

    typedef typename StripPtrRefConst<OT>::Type ObjType;
    typedef typename StripPtrRefConst<VT>::Type VarType;

    typedef void (OT::*GetCBType)(VarType *) const;
    typedef void (OT::*SetCBType)(const VarType *);

    VarCallbacksMemFuncByPointer(GetCBType getCb, SetCBType setCb)
        : getter(getCb)
        , setter(setCb)
    { }

    void callGetter(const ObjType * obj, VarType * valueOut) const
    {
        NTB_ASSERT(obj    != NTB_NULL);
        NTB_ASSERT(getter != NTB_NULL);
        (obj->*getter)(valueOut);
    }
    void callGetter(const ObjType * obj, VarType & valueOut) const
    {
        NTB_ASSERT(obj    != NTB_NULL);
        NTB_ASSERT(getter != NTB_NULL);
        (obj->*getter)(&valueOut);
    }

    void callSetter(ObjType * obj, const VarType * valueIn) const
    {
        NTB_ASSERT(obj    != NTB_NULL);
        NTB_ASSERT(setter != NTB_NULL);
        (obj->*setter)(valueIn);
    }
    void callSetter(ObjType * obj, const VarType & valueIn) const
    {
        NTB_ASSERT(obj    != NTB_NULL);
        NTB_ASSERT(setter != NTB_NULL);
        (obj->*setter)(&valueIn);
    }

    bool hasSetter() const { return setter != NTB_NULL; }

private:

    GetCBType getter;
    SetCBType setter;
};

// ========================================================
// class VarCallbacksCFuncPtr:
// ========================================================

template<class OT, class VT>
class VarCallbacksCFuncPtr NTB_FINAL_CLASS
{
public:

    typedef typename StripPtrRefConst<OT>::Type ObjType;
    typedef typename StripPtrRefConst<VT>::Type VarType;

    typedef void (*GetCBType)(const ObjType *, VarType *);
    typedef void (*SetCBType)(ObjType *, const VarType *);

    VarCallbacksCFuncPtr(GetCBType getCb, SetCBType setCb)
        : getter(getCb)
        , setter(setCb)
    { }

    void callGetter(const ObjType * obj, VarType * valueOut) const
    {
        NTB_ASSERT(getter != NTB_NULL);
        getter(obj, valueOut);
    }
    void callGetter(const ObjType * obj, VarType & valueOut) const
    {
        NTB_ASSERT(getter != NTB_NULL);
        getter(obj, &valueOut);
    }

    void callSetter(ObjType * obj, const VarType * valueIn) const
    {
        NTB_ASSERT(setter != NTB_NULL);
        setter(obj, valueIn);
    }
    void callSetter(ObjType * obj, const VarType & valueIn) const
    {
        NTB_ASSERT(setter != NTB_NULL);
        setter(obj, &valueIn);
    }

    // Used exclusively by Panel::addPointer()
    void callGetter(void ** obj, void ** valueOut) const
    {
        NTB_ASSERT(getter != NTB_NULL);
        getter(obj, valueOut);
    }
    void callSetter(void ** obj, void ** valueIn) const
    {
        NTB_ASSERT(setter != NTB_NULL);
        setter(obj, valueIn);
    }

    bool hasSetter() const { return setter != NTB_NULL; }

private:

    GetCBType getter;
    SetCBType setter;
};

// ========================================================
// convert() overloads:
// ========================================================

//TODO put these into the extended types themselves
//

//
// Booleans - bool is the common type. No conversion needed.
//
inline void convert(const BoolEx & from, bool & to) { to = static_cast<bool>(from.value);  }
inline void convert(const bool from, BoolEx & to)   { to.value = static_cast<UInt8>(from); }

inline void convert(const NumberEx & from, Int8  & to) { to = static_cast<Int8>(from.asI64);  }
inline void convert(const NumberEx & from, Int16 & to) { to = static_cast<Int16>(from.asI64); }
inline void convert(const NumberEx & from, Int32 & to) { to = static_cast<Int32>(from.asI64); }
inline void convert(const NumberEx & from, Int64 & to) { to = from.asI64; }

inline void convert(const NumberEx & from, UInt8  & to) { to = static_cast<UInt8>(from.asU64);  }
inline void convert(const NumberEx & from, UInt16 & to) { to = static_cast<UInt16>(from.asU64); }
inline void convert(const NumberEx & from, UInt32 & to) { to = static_cast<UInt32>(from.asU64); }
inline void convert(const NumberEx & from, UInt64 & to) { to = from.asU64; }

inline void convert(const NumberEx & from, float  & to) { to = static_cast<float>(from.asDouble); }
inline void convert(const NumberEx & from, double & to) { to = from.asDouble; }


inline void convert(const Int8  from, NumberEx & to) { to.asI64 = from; }
inline void convert(const Int16 from, NumberEx & to) { to.asI64 = from; }
inline void convert(const Int32 from, NumberEx & to) { to.asI64 = from; }
inline void convert(const Int64 from, NumberEx & to) { to.asI64 = from; }

inline void convert(const UInt8  from, NumberEx & to) { to.asU64 = from; }
inline void convert(const UInt16 from, NumberEx & to) { to.asU64 = from; }
inline void convert(const UInt32 from, NumberEx & to) { to.asU64 = from; }
inline void convert(const UInt64 from, NumberEx & to) { to.asU64 = from; }

inline void convert(const float  from, NumberEx & to) { to.asDouble = from; }
inline void convert(const double from, NumberEx & to) { to.asDouble = from; }

//
// Void pointers (we care about the actual pointer value).
//
inline void convert(void * from, NumberEx & to)
{
    to.asPointer = from;
}
inline void convert(const NumberEx & from, void *& to)
{
    to = from.asPointer;
}

//
// Colors - ColorEx is the common type.
//
inline void convert(const float   * from, ColorEx & to) { to.setColorF(from);  }
inline void convert(const ColorEx & from, float   * to) { from.getColorF(to);  }
inline void convert(const UByte   * from, ColorEx & to) { to.setColorB(from);  }
inline void convert(const ColorEx & from, UByte   * to) { from.getColorB(to);  }
inline void convert(const Color32 & from, ColorEx & to) { to.setColor32(from); }
inline void convert(const ColorEx & from, Color32 & to) { from.getColor32(to); }

//
// Vectors and Quaternion - Float4Ex is the common type.
//
inline void convert(const float * from, Float4Ex & to) { to.setFloats(from); }
inline void convert(const Float4Ex & from, float * to) { from.getFloats(to); }

//
// Strings (single char also) - SmallStr is the common type.
//
inline void convert(const SmallStr & from, SmallStr & to) { to = from;    }
inline void convert(const SmallStr & from, char & to)     { to = from[0]; }
inline void convert(const char from, SmallStr   & to)     { to[0] = from; }

// Here's where SmallStr::maxSize comes into play:
inline void convert(const SmallStr & from, char * to)
{
    // If maxSize isn't specified assume the char buffer is just big enough to hold the string...
    const int maxChars = (from.getMaxSize() > 0) ? from.getMaxSize() : (from.getLength() + 1);
    copyString(to, maxChars, from.getCString());
}
inline void convert(const char * from, SmallStr & to)
{
    to.setCString(from);
}

//
// std::string, stored as SmallStr:
//
#if NEO_TWEAK_BAR_STD_STRING_INTEROP
inline void convert(const std::string & from, SmallStr & to)
{
    to.setCString(from.c_str(), static_cast<int>(from.length()));
}
inline void convert(const SmallStr & from, std::string & to)
{
    to.assign(from.getCString(), from.getLength());
}
#endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

//
// C++ enums - EnumValExImpl is the common type.
//
template<class EnumType>
inline void convert(const EnumType from, EnumValExImpl<EnumType> & to)
{
    to.value = from;
}
template<class EnumType>
inline void convert(const EnumValExImpl<EnumType> & from, EnumType & to)
{
    to = from.value;
}

// ========================================================
// Template class VarImpl:
//
// The supported variable types will narrow down to
// the following common types that the UI supports:
//
// VarImpl<SmallStr>  -> All strings (including single a char)
// VarImpl<NumberEx>  -> All numbers, integer or floating-point
// VarImpl<BoolEx>    -> Booleans only
// VarImpl<Float4Ex>  -> All vectors, quaternions and directions
// VarImpl<ColorEx>   -> All colors (float or byte)
// VarImpl<EnumValEx> -> User defined enums
//
// NOTE: Pointer vars (void*) will use either UInt32 or UInt64,
// depending on the platform's pointer size, so they also fall
// into the NumberEx category.
//
// The convert() functions take care of converting
// from the internal types to the user types.
// ========================================================

template<class OT, class VT, class DT, class CBT>
class VarImpl NTB_FINAL_CLASS
    : public Variable
{
    NTB_DISABLE_COPY_ASSIGN(VarImpl);

public:

    typedef VT  VarType;
    typedef DT  DispType;
    typedef CBT CallbacksType;
    typedef typename StripPtrRefConst<OT>::Type ObjType;

    VarImpl(Panel * owner, Variable * parent, const char * name,
            const ObjType * obj, const CallbacksType & cbs, const Access access)
        : Variable(owner, parent, name)
        , userPtr(const_cast<ObjType *>(obj))
        , callbacks(cbs)
        , displayValue()
        , cachedValue()
        , accessMode(access)
    {
        NTB_ASSERT(owner != NTB_NULL);

        if (accessMode == Variable::ReadWrite && !callbacks.hasSetter())
        {
            NTB_ERROR("Read-write variable missing a setter callback!");
        }
        else if (accessMode == Variable::ReadOnly && callbacks.hasSetter())
        {
            NTB_ERROR("Read-only variable should not have a setter callback!");
        }
    }

    void onUpdateCachedValue() const NTB_OVERRIDE
    {
        // A getter must always be provided.
        callbacks.callGetter(userPtr, cachedValue);
        convert(cachedValue, displayValue);
    }

    void onUpdateUserValue() const NTB_OVERRIDE
    {
        // Read-Only variables won't have a setter method.
        if (accessMode == Variable::ReadWrite && callbacks.hasSetter())
        {
            convert(displayValue, cachedValue);
            callbacks.callSetter(userPtr, cachedValue);
        }
    }

    void onLinkedToPanel(Panel & owner) NTB_OVERRIDE
    {
        setUpVarValueDisplay(owner, displayValue);
    }

    void onDraw(GeometryBatch & geoBatch) const NTB_OVERRIDE
    {
        VarDisplayWidget::onDraw(geoBatch);
        if (!isVisible())
        {
            return;
        }

        drawVarName(geoBatch);                // Var name drawing (left side)
        drawVarValue(geoBatch, displayValue); // Displayed value (right side)
        drawValueEditButtons(geoBatch);       // [+],[-] buttons (if enabled)
    }

    Variable * setNumberFormatting(const int numericBase) NTB_OVERRIDE
    {
        //TODO
        (void)numericBase;
        return this;
    }

    Variable * setMaxStringSize(const int maxSizeIncludingNulTerminator) NTB_OVERRIDE
    {
        //TODO this is only valid for SmallStr display types...
        (void)maxSizeIncludingNulTerminator;
        return this;
    }

    Access getAccessMode() const NTB_OVERRIDE
    {
        return accessMode;
    }

    DispType & getDisplayValue()
    {
        return displayValue;
    }

private:

    // Pointer to either a user variable or object that owns the get/set callbacks.
    // We rip away const from it so we can combine both RO and RW variables under the
    // same VarImpl. We promise to be good and never try to set the variable if it is RO ;)
    ObjType * userPtr;

    // The user pointer is never touched directly, but instead via
    // the callbacks. This way we can combine accessing direct pointers
    // to variables and access through functions/methods under the same interface.
    CallbacksType callbacks;

    // Cached value of a common type displayable in the NTB UI. This
    // is generally not the same type as the user value. Before doing
    // the get/set we convert to/from the cachedValue if needed.
    mutable DispType displayValue;

    // Cached value of the same type as the user's variable.
    mutable VarType cachedValue;

    // Access mode set on construction: Variable::ReadOnly or Variable::ReadWrite.
    const Access accessMode;
};

// ========================================================
// class VarHierarchyParent:
//
// This specialized Variable class can be used to open
// a hierarchy handle that has only a name and no value
// associated. A hierarchy can then be used to define
// a tree of values or a user structure/object.
//
// Note that most styling methods are unsupported,
// so they will be no-ops if called.
// ========================================================

class VarHierarchyParent NTB_FINAL_CLASS
    : public Variable
{
    NTB_DISABLE_COPY_ASSIGN(VarHierarchyParent);

public:

    VarHierarchyParent(Panel * owner, Variable * parent, const char * name);
    ~VarHierarchyParent();

    Access getAccessMode() const NTB_OVERRIDE;
    void onDraw(GeometryBatch & geoBatch) const NTB_OVERRIDE;

    // Unsupported stuff:
    Variable * setNumberFormatting(int numericBase) NTB_OVERRIDE;
    Variable * setMaxStringSize(int maxSizeIncludingNulTerminator) NTB_OVERRIDE;
    void onUpdateCachedValue() const NTB_OVERRIDE;
    void onUpdateUserValue() const NTB_OVERRIDE;
    void onLinkedToPanel(Panel & owner) NTB_OVERRIDE;
};

// ========================================================
// Built-in get/set helpers for Panel:
// ========================================================

template<class T> inline void defaultGetter(const T * src, T * dest) { *dest = *src; }
template<class T> inline void defaultSetter(T * dest, const T * src) { *dest = *src; }

template<class T, int Size>
inline void defaultGetterArray(const T * src, T * dest)
{
    for (int i = 0; i < Size; ++i)
    {
        dest[i] = src[i];
    }
}
template<class T, int Size>
inline void defaultSetterArray(T * dest, const T * src)
{
    for (int i = 0; i < Size; ++i)
    {
        dest[i] = src[i];
    }
}

template<int Size>
inline void defaultGetterCZStr(const char * src, char * dest)
{
    #if NEO_TWEAK_BAR_CXX11_SUPPORTED
    static_assert(Size <= Panel::CStringMaxSize, "Max static C-string length exceeded!");
    #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

    copyString(dest, Size, src);
}
template<int Size>
inline void defaultSetterCZStr(char * dest, const char * src)
{
    #if NEO_TWEAK_BAR_CXX11_SUPPORTED
    static_assert(Size <= Panel::CStringMaxSize, "Max static C-string length exceeded!");
    #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

    copyString(dest, Size, src);
}

// ========================================================
// Type switches to convert the supported signed/unsigned
// integer types plus floats to the underlaying common type
// used by NTB for displaying & storage.
// All signeds expand to Int64 and all unsigneds
// expand to UInt64. Float expands to double.
// ========================================================

// TODO Move this closer to NumberEx?
template<class T> struct NumTS  { /* unsupported */ };
template<> struct NumTS<Int8>   { static const NumberEx::Type Tag = NumberEx::SignedInt;   };
template<> struct NumTS<Int16>  { static const NumberEx::Type Tag = NumberEx::SignedInt;   };
template<> struct NumTS<Int32>  { static const NumberEx::Type Tag = NumberEx::SignedInt;   };
template<> struct NumTS<Int64>  { static const NumberEx::Type Tag = NumberEx::SignedInt;   };
template<> struct NumTS<UInt8>  { static const NumberEx::Type Tag = NumberEx::UnsignedInt; };
template<> struct NumTS<UInt16> { static const NumberEx::Type Tag = NumberEx::UnsignedInt; };
template<> struct NumTS<UInt32> { static const NumberEx::Type Tag = NumberEx::UnsignedInt; };
template<> struct NumTS<UInt64> { static const NumberEx::Type Tag = NumberEx::UnsignedInt; };
template<> struct NumTS<float>  { static const NumberEx::Type Tag = NumberEx::Double;      };
template<> struct NumTS<double> { static const NumberEx::Type Tag = NumberEx::Double;      };

// ========================================================
// Type switches for Panel::addColor().
// The Color32 overloads store a single
// object, while the float[] and UByte[]
// ones must store a small array.
// ========================================================

template<class T, int Size>
struct ColorTS
{
    typedef T Type[Size];
};
template<>
struct ColorTS<Color32, 4>
{
    typedef Color32 Type;
};

// ========================================================
// Type switches for Panel::addString(char *).
// We'll limit the size of C-strings to a fixed constant.
// ========================================================

template<class T>
struct StrTS
{
    /* unsupported */
};
template<>
struct StrTS<char>
{
    typedef char Type[Panel::CStringMaxSize];
};
template<>
struct StrTS<SmallStr>
{
    typedef SmallStr Type;
};
#if NEO_TWEAK_BAR_STD_STRING_INTEROP
template<>
struct StrTS<std::string>
{
    typedef std::string Type;
};
#endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

} // namespace detail {}

// ================================================================================================
//
//                                 NTB callbacks() helpers
//
// ================================================================================================

// ========================================================
// Callbacks from member functions,
// dealing with references or values:
//
// void VT OT::getCallback() const;
// void OT::setCallback(VT valueIn);
//
// VT can be a reference, const reference or value.
// ========================================================

template<class OT, class VT>
inline detail::VarCallbacksMemFuncByValOrRef<OT, VT> callbacks(VT (OT::*getCb)() const)
{
    // The member function by val or ref callbacks
    // won't accept a pointer type, but the error
    // message generated is less than clear (it fails
    // inside the VarCallbacksMemFuncByValOrRef class).
    // If we are building for C++11 or above, this check
    // will provide a clean error message right away,
    // which should help the user find out the offending
    // method more easily. If C++11 is not available,
    // we still get a compiler error, but it might be a
    // little harder to figure out where is the wrong method...
    #if NEO_TWEAK_BAR_CXX11_SUPPORTED
    static_assert(!std::is_pointer<VT>::value, "Var cannot be a pointer for this type of callback!");
    #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

    return detail::VarCallbacksMemFuncByValOrRef<OT, VT>(getCb, NTB_NULL);
}

template<class OT, class VT>
inline detail::VarCallbacksMemFuncByValOrRef<OT, VT> callbacks(VT (OT::*getCb)() const, void (OT::*setCb)(VT))
{
    // No pointers allowed here!
    // See the comment above in the other function.
    #if NEO_TWEAK_BAR_CXX11_SUPPORTED
    static_assert(!std::is_pointer<VT>::value, "Var cannot be a pointer for this type of callbacks!");
    #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

    return detail::VarCallbacksMemFuncByValOrRef<OT, VT>(getCb, setCb);
}

// ========================================================
// Callbacks from member functions, dealing with pointers:
//
// void OT::getCallback(VT * valueOut) const;
// void OT::setCallback(const VT * valueIn);
//
// ========================================================

template<class OT, class VT>
inline detail::VarCallbacksMemFuncByPointer<OT, VT> callbacks(void (OT::*getCb)(VT *) const)
{
    return detail::VarCallbacksMemFuncByPointer<OT, VT>(getCb, NTB_NULL);
}

template<class OT, class VT>
inline detail::VarCallbacksMemFuncByPointer<OT, VT> callbacks(void (OT::*getCb)(VT *) const, void (OT::*setCb)(const VT *))
{
    return detail::VarCallbacksMemFuncByPointer<OT, VT>(getCb, setCb);
}

// ========================================================
// Callbacks from C-style function pointers:
//
// void getCallback(const OT * obj, VT * valueOut);
// void setCallback(OT * obj, const VT * valueIn);
//
// ========================================================

template<class OT, class VT>
inline detail::VarCallbacksCFuncPtr<OT, VT> callbacks(void (*getCb)(const OT *, VT *))
{
    return detail::VarCallbacksCFuncPtr<OT, VT>(getCb, NTB_NULL);
}

template<class OT, class VT>
inline detail::VarCallbacksCFuncPtr<OT, VT> callbacks(void (*getCb)(const OT *, VT *), void (*setCb)(OT *, const VT *))
{
    return detail::VarCallbacksCFuncPtr<OT, VT>(getCb, setCb);
}

// ================================================================================================
//
//                                  Panel class inline methods
//
// ================================================================================================

// ========================================================
// Panel => Boolean vars:
// ========================================================

// Pointers:
inline Variable * Panel::addBoolRO(const char * name, const bool * var)
{
    return addBoolRO(NTB_NULL, name, var);
}
inline Variable * Panel::addBoolRO(Variable * parent, const char * name, const bool * var)
{
    return addBoolRO(parent, name, var, callbacks(detail::defaultGetter<bool>));
}
inline Variable * Panel::addBoolRW(const char * name, bool * var)
{
    return addBoolRW(NTB_NULL, name, var);
}
inline Variable * Panel::addBoolRW(Variable * parent, const char * name, bool * var)
{
    return addBoolRW(parent, name, var, callbacks(detail::defaultGetter<bool>, detail::defaultSetter<bool>));
}

// Callbacks:
template<class OT, class CBT>
inline Variable * Panel::addBoolRO(const char * name, const OT * obj, const CBT & cbs)
{
    return addBoolRO(NTB_NULL, name, obj, cbs);
}
template<class OT, class CBT>
inline Variable * Panel::addBoolRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs)
{
    return addVarImpl(NTB_NEW detail::VarImpl<OT, typename CBT::VarType, detail::BoolEx, CBT>(this, parent, name, obj, cbs, Variable::ReadOnly));
}
template<class OT, class CBT>
inline Variable * Panel::addBoolRW(const char * name, OT * obj, const CBT & cbs)
{
    return addBoolRW(NTB_NULL, name, obj, cbs);
}
template<class OT, class CBT>
inline Variable * Panel::addBoolRW(Variable * parent, const char * name, OT * obj, const CBT & cbs)
{
    return addVarImpl(NTB_NEW detail::VarImpl<OT, typename CBT::VarType, detail::BoolEx, CBT>(this, parent, name, obj, cbs, Variable::ReadWrite));
}

// ========================================================
// Panel => Single char vars:
// ========================================================

// Pointers:
inline Variable * Panel::addCharRO(const char * name, const char * var)
{
    return addCharRO(NTB_NULL, name, var);
}
inline Variable * Panel::addCharRO(Variable * parent, const char * name, const char * var)
{
    return addCharRO(parent, name, var, callbacks(detail::defaultGetter<char>));
}
inline Variable * Panel::addCharRW(const char * name, char * var)
{
    return addCharRW(NTB_NULL, name, var);
}
inline Variable * Panel::addCharRW(Variable * parent, const char * name, char * var)
{
    return addCharRW(parent, name, var, callbacks(detail::defaultGetter<char>, detail::defaultSetter<char>));
}

// Callbacks:
template<class OT, class CBT>
inline Variable * Panel::addCharRO(const char * name, const OT * obj, const CBT & cbs)
{
    return addCharRO(NTB_NULL, name, obj, cbs);
}
template<class OT, class CBT>
inline Variable * Panel::addCharRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs)
{
    detail::VarImpl<OT, typename CBT::VarType, SmallStr, CBT> * var =
        NTB_NEW detail::VarImpl<OT, typename CBT::VarType, SmallStr, CBT>(this, parent, name, obj, cbs, Variable::ReadOnly);

    var->getDisplayValue().setMaxSize(2);      // 1 for the char + a NUL terminator to make it a valid C-string.
    var->getDisplayValue().setCString("?", 1); // Set length to 1 char.
    return addVarImpl(var);
}
template<class OT, class CBT>
inline Variable * Panel::addCharRW(const char * name, OT * obj, const CBT & cbs)
{
    return addCharRW(NTB_NULL, name, obj, cbs);
}
template<class OT, class CBT>
inline Variable * Panel::addCharRW(Variable * parent, const char * name, OT * obj, const CBT & cbs)
{
    detail::VarImpl<OT, typename CBT::VarType, SmallStr, CBT> * var =
        NTB_NEW detail::VarImpl<OT, typename CBT::VarType, SmallStr, CBT>(this, parent, name, obj, cbs, Variable::ReadWrite);

    var->getDisplayValue().setMaxSize(2);      // 1 for the char + a NUL terminator to make it a valid C-string.
    var->getDisplayValue().setCString("?", 1); // Set length to 1 char.
    return addVarImpl(var);
}

// ========================================================
// Panel => Number vars:
// ========================================================

// Int8:
inline Variable * Panel::addNumberRO(const char * name, const Int8 * var)
{
    return addNumberRO(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRO(Variable * parent, const char * name, const Int8 * var)
{
    return addNumberRO(parent, name, var, callbacks(detail::defaultGetter<Int8>));
}
inline Variable * Panel::addNumberRW(const char * name, Int8 * var)
{
    return addNumberRW(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRW(Variable * parent, const char * name, Int8 * var)
{
    return addNumberRW(parent, name, var, callbacks(detail::defaultGetter<Int8>, detail::defaultSetter<Int8>));
}

// UInt8:
inline Variable * Panel::addNumberRO(const char * name, const UInt8 * var)
{
    return addNumberRO(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRO(Variable * parent, const char * name, const UInt8 * var)
{
    return addNumberRO(parent, name, var, callbacks(detail::defaultGetter<UInt8>));
}
inline Variable * Panel::addNumberRW(const char * name, UInt8 * var)
{
    return addNumberRW(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRW(Variable * parent, const char * name, UInt8 * var)
{
    return addNumberRW(parent, name, var, callbacks(detail::defaultGetter<UInt8>, detail::defaultSetter<UInt8>));
}

// Int16:
inline Variable * Panel::addNumberRO(const char * name, const Int16 * var)
{
    return addNumberRO(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRO(Variable * parent, const char * name, const Int16 * var)
{
    return addNumberRO(parent, name, var, callbacks(detail::defaultGetter<Int16>));
}
inline Variable * Panel::addNumberRW(const char * name, Int16 * var)
{
    return addNumberRW(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRW(Variable * parent, const char * name, Int16 * var)
{
    return addNumberRW(parent, name, var, callbacks(detail::defaultGetter<Int16>, detail::defaultSetter<Int16>));
}

// UInt16:
inline Variable * Panel::addNumberRO(const char * name, const UInt16 * var)
{
    return addNumberRO(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRO(Variable * parent, const char * name, const UInt16 * var)
{
    return addNumberRO(parent, name, var, callbacks(detail::defaultGetter<UInt16>));
}
inline Variable * Panel::addNumberRW(const char * name, UInt16 * var)
{
    return addNumberRW(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRW(Variable * parent, const char * name, UInt16 * var)
{
    return addNumberRW(parent, name, var, callbacks(detail::defaultGetter<UInt16>, detail::defaultSetter<UInt16>));
}

// Int32:
inline Variable * Panel::addNumberRO(const char * name, const Int32 * var)
{
    return addNumberRO(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRO(Variable * parent, const char * name, const Int32 * var)
{
    return addNumberRO(parent, name, var, callbacks(detail::defaultGetter<Int32>));
}
inline Variable * Panel::addNumberRW(const char * name, Int32 * var)
{
    return addNumberRW(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRW(Variable * parent, const char * name, Int32 * var)
{
    return addNumberRW(parent, name, var, callbacks(detail::defaultGetter<Int32>, detail::defaultSetter<Int32>));
}

// UInt32:
inline Variable * Panel::addNumberRO(const char * name, const UInt32 * var)
{
    return addNumberRO(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRO(Variable * parent, const char * name, const UInt32 * var)
{
    return addNumberRO(parent, name, var, callbacks(detail::defaultGetter<UInt32>));
}
inline Variable * Panel::addNumberRW(const char * name, UInt32 * var)
{
    return addNumberRW(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRW(Variable * parent, const char * name, UInt32 * var)
{
    return addNumberRW(parent, name, var, callbacks(detail::defaultGetter<UInt32>, detail::defaultSetter<UInt32>));
}

// Int64:
inline Variable * Panel::addNumberRO(const char * name, const Int64 * var)
{
    return addNumberRO(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRO(Variable * parent, const char * name, const Int64 * var)
{
    return addNumberRO(parent, name, var, callbacks(detail::defaultGetter<Int64>));
}
inline Variable * Panel::addNumberRW(const char * name, Int64 * var)
{
    return addNumberRW(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRW(Variable * parent, const char * name, Int64 * var)
{
    return addNumberRW(parent, name, var, callbacks(detail::defaultGetter<Int64>, detail::defaultSetter<Int64>));
}

// UInt64:
inline Variable * Panel::addNumberRO(const char * name, const UInt64 * var)
{
    return addNumberRO(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRO(Variable * parent, const char * name, const UInt64 * var)
{
    return addNumberRO(parent, name, var, callbacks(detail::defaultGetter<UInt64>));
}
inline Variable * Panel::addNumberRW(const char * name, UInt64 * var)
{
    return addNumberRW(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRW(Variable * parent, const char * name, UInt64 * var)
{
    return addNumberRW(parent, name, var, callbacks(detail::defaultGetter<UInt64>, detail::defaultSetter<UInt64>));
}

// float:
inline Variable * Panel::addNumberRO(const char * name, const float * var)
{
    return addNumberRO(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRO(Variable * parent, const char * name, const float * var)
{
    return addNumberRO(parent, name, var, callbacks(detail::defaultGetter<float>));
}
inline Variable * Panel::addNumberRW(const char * name, float * var)
{
    return addNumberRW(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRW(Variable * parent, const char * name, float * var)
{
    return addNumberRW(parent, name, var, callbacks(detail::defaultGetter<float>, detail::defaultSetter<float>));
}

// double:
inline Variable * Panel::addNumberRO(const char * name, const double * var)
{
    return addNumberRO(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRO(Variable * parent, const char * name, const double * var)
{
    return addNumberRO(parent, name, var, callbacks(detail::defaultGetter<double>));
}
inline Variable * Panel::addNumberRW(const char * name, double * var)
{
    return addNumberRW(NTB_NULL, name, var);
}
inline Variable * Panel::addNumberRW(Variable * parent, const char * name, double * var)
{
    return addNumberRW(parent, name, var, callbacks(detail::defaultGetter<double>, detail::defaultSetter<double>));
}

// Number var callbacks:
template<class OT, class CBT>
inline Variable * Panel::addNumberRO(const char * name, const OT * obj, const CBT & cbs)
{
    return addNumberRO(NTB_NULL, name, obj, cbs);
}
template<class OT, class CBT>
inline Variable * Panel::addNumberRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs)
{
    typedef typename CBT::VarType VarType;
    typedef detail::NumTS<VarType> NumType;

    detail::VarImpl<OT, VarType, detail::NumberEx, CBT> * var =
        NTB_NEW detail::VarImpl<OT, VarType, detail::NumberEx, CBT>(this, parent, name, obj, cbs, Variable::ReadOnly);

    var->getDisplayValue().type = NumType::Tag;
    return addVarImpl(var);
}
template<class OT, class CBT>
inline Variable * Panel::addNumberRW(const char * name, OT * obj, const CBT & cbs)
{
    return addNumberRW(NTB_NULL, name, obj, cbs);
}
template<class OT, class CBT>
inline Variable * Panel::addNumberRW(Variable * parent, const char * name, OT * obj, const CBT & cbs)
{
    typedef typename CBT::VarType VarType;
    typedef detail::NumTS<VarType> NumType;

    detail::VarImpl<OT, VarType, detail::NumberEx, CBT> * var =
        NTB_NEW detail::VarImpl<OT, VarType, detail::NumberEx, CBT>(this, parent, name, obj, cbs, Variable::ReadWrite);

    var->getDisplayValue().type = NumType::Tag;
    return addVarImpl(var);
}

// ========================================================
// Panel => Float vectors:
// ========================================================

// Pointers:
template<int Size>
inline Variable * Panel::addFloatVecRO(const char * name, const float * vec)
{
    return addFloatVecRO<Size>(NTB_NULL, name, vec, callbacks(detail::defaultGetterArray<float, Size>));
}
template<int Size>
inline Variable * Panel::addFloatVecRO(Variable * parent, const char * name, const float * vec)
{
    return addFloatVecRO<Size>(parent, name, vec, callbacks(detail::defaultGetterArray<float, Size>));
}
template<int Size>
inline Variable * Panel::addFloatVecRW(const char * name, float * vec)
{
    return addFloatVecRW<Size>(NTB_NULL, name, vec,
            callbacks(detail::defaultGetterArray<float, Size>, detail::defaultSetterArray<float, Size>));
}
template<int Size>
inline Variable * Panel::addFloatVecRW(Variable * parent, const char * name, float * vec)
{
    return addFloatVecRW<Size>(parent, name, vec,
            callbacks(detail::defaultGetterArray<float, Size>, detail::defaultSetterArray<float, Size>));
}

// Callbacks:
template<int Size, class OT, class CBT>
inline Variable * Panel::addFloatVecRO(const char * name, const OT * obj, const CBT & cbs)
{
    return addFloatVecRO<Size>(NTB_NULL, name, obj, cbs);
}
template<int Size, class OT, class CBT>
inline Variable * Panel::addFloatVecRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs)
{
    #if NEO_TWEAK_BAR_CXX11_SUPPORTED
    static_assert(Size == 2 || Size == 3 || Size == 4, "Vectors must have 2, 3 or 4 components!");
    #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

    typedef float FVecType[Size];

    detail::VarImpl<OT, FVecType, detail::Float4Ex, CBT> * var =
        NTB_NEW detail::VarImpl<OT, FVecType, detail::Float4Ex, CBT>(this, parent, name, obj, cbs, Variable::ReadOnly);

    var->getDisplayValue().setTypeFromSize(Size);
    return addVarImpl(var);
}
template<int Size, class OT, class CBT>
inline Variable * Panel::addFloatVecRW(const char * name, OT * obj, const CBT & cbs)
{
    return addFloatVecRW<Size>(NTB_NULL, name, obj, cbs);
}
template<int Size, class OT, class CBT>
inline Variable * Panel::addFloatVecRW(Variable * parent, const char * name, OT * obj, const CBT & cbs)
{
    #if NEO_TWEAK_BAR_CXX11_SUPPORTED
    static_assert(Size == 2 || Size == 3 || Size == 4, "Vectors must have 2, 3 or 4 components!");
    #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

    typedef float FVecType[Size];

    detail::VarImpl<OT, FVecType, detail::Float4Ex, CBT> * var =
        NTB_NEW detail::VarImpl<OT, FVecType, detail::Float4Ex, CBT>(this, parent, name, obj, cbs, Variable::ReadWrite);

    var->getDisplayValue().setTypeFromSize(Size);
    return addVarImpl(var);
}

// ========================================================
// Panel => Direction vectors (float3):
// ========================================================

// Pointers:
inline Variable * Panel::addDirectionVecRO(const char * name, const float * vec)
{
    return addDirectionVecRO(NTB_NULL, name, vec);
}
inline Variable * Panel::addDirectionVecRO(Variable * parent, const char * name, const float * vec)
{
    return addDirectionVecRO(parent, name, vec, callbacks(detail::defaultGetterArray<float, 3>));
}
inline Variable * Panel::addDirectionVecRW(const char * name, float * vec)
{
    return addDirectionVecRW(NTB_NULL, name, vec);
}
inline Variable * Panel::addDirectionVecRW(Variable * parent, const char * name, float * vec)
{
    return addDirectionVecRW(parent, name, vec,
            callbacks(detail::defaultGetterArray<float, 3>, detail::defaultSetterArray<float, 3>));
}

// Callbacks:
template<class OT, class CBT>
inline Variable * Panel::addDirectionVecRO(const char * name, const OT * obj, const CBT & cbs)
{
    return addDirectionVecRO(NTB_NULL, name, obj, cbs);
}
template<class OT, class CBT>
inline Variable * Panel::addDirectionVecRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs)
{
    typedef float DirVecType[3];

    detail::VarImpl<OT, DirVecType, detail::Float4Ex, CBT> * var =
        NTB_NEW detail::VarImpl<OT, DirVecType, detail::Float4Ex, CBT>(this, parent, name, obj, cbs, Variable::ReadOnly);

    var->getDisplayValue().type = detail::Float4Ex::Dir3;
    return addVarImpl(var);
}
template<class OT, class CBT>
inline Variable * Panel::addDirectionVecRW(const char * name, OT * obj, const CBT & cbs)
{
    return addDirectionVecRW(NTB_NULL, name, obj, cbs);
}
template<class OT, class CBT>
inline Variable * Panel::addDirectionVecRW(Variable * parent, const char * name, OT * obj, const CBT & cbs)
{
    typedef float DirVecType[3];

    detail::VarImpl<OT, DirVecType, detail::Float4Ex, CBT> * var =
        NTB_NEW detail::VarImpl<OT, DirVecType, detail::Float4Ex, CBT>(this, parent, name, obj, cbs, Variable::ReadWrite);

    var->getDisplayValue().type = detail::Float4Ex::Dir3;
    return addVarImpl(var);
}

// ========================================================
// Panel => Rotation quaternions (float4):
// ========================================================

// Pointers:
inline Variable * Panel::addRotationQuatRO(const char * name, const float * quat)
{
    return addRotationQuatRO(NTB_NULL, name, quat);
}
inline Variable * Panel::addRotationQuatRO(Variable * parent, const char * name, const float * quat)
{
    return addRotationQuatRO(parent, name, quat, callbacks(detail::defaultGetterArray<float, 4>));
}
inline Variable * Panel::addRotationQuatRW(const char * name, float * quat)
{
    return addRotationQuatRW(NTB_NULL, name, quat);
}
inline Variable * Panel::addRotationQuatRW(Variable * parent, const char * name, float * quat)
{
    return addRotationQuatRW(parent, name, quat,
            callbacks(detail::defaultGetterArray<float, 4>, detail::defaultSetterArray<float, 4>));
}

// Callbacks:
template<class OT, class CBT>
inline Variable * Panel::addRotationQuatRO(const char * name, const OT * obj, const CBT & cbs)
{
    return addRotationQuatRO(NTB_NULL, name, obj, cbs);
}
template<class OT, class CBT>
inline Variable * Panel::addRotationQuatRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs)
{
    typedef float QuatType[4];

    detail::VarImpl<OT, QuatType, detail::Float4Ex, CBT> * var =
        NTB_NEW detail::VarImpl<OT, QuatType, detail::Float4Ex, CBT>(this, parent, name, obj, cbs, Variable::ReadOnly);

    var->getDisplayValue().type = detail::Float4Ex::Quat4;
    return addVarImpl(var);
}
template<class OT, class CBT>
inline Variable * Panel::addRotationQuatRW(const char * name, OT * obj, const CBT & cbs)
{
    return addRotationQuatRW(NTB_NULL, name, obj, cbs);
}
template<class OT, class CBT>
inline Variable * Panel::addRotationQuatRW(Variable * parent, const char * name, OT * obj, const CBT & cbs)
{
    typedef float QuatType[4];

    detail::VarImpl<OT, QuatType, detail::Float4Ex, CBT> * var =
        NTB_NEW detail::VarImpl<OT, QuatType, detail::Float4Ex, CBT>(this, parent, name, obj, cbs, Variable::ReadWrite);

    var->getDisplayValue().type = detail::Float4Ex::Quat4;
    return addVarImpl(var);
}

// ========================================================
// Panel => Color values:
// ========================================================

// Pointers to UByte color vector:
template<int Size>
inline Variable * Panel::addColorRO(const char * name, const UByte * clr)
{
    return addColorRO<Size>(NTB_NULL, name, clr, callbacks(detail::defaultGetterArray<UByte, Size>));
}
template<int Size>
inline Variable * Panel::addColorRO(Variable * parent, const char * name, const UByte * clr)
{
    return addColorRO<Size>(parent, name, clr, callbacks(detail::defaultGetterArray<UByte, Size>));
}
template<int Size>
inline Variable * Panel::addColorRW(const char * name, UByte * clr)
{
    return addColorRW<Size>(NTB_NULL, name, clr,
            callbacks(detail::defaultGetterArray<UByte, Size>, detail::defaultSetterArray<UByte, Size>));
}
template<int Size>
inline Variable * Panel::addColorRW(Variable * parent, const char * name, UByte * clr)
{
    return addColorRW<Size>(parent, name, clr,
            callbacks(detail::defaultGetterArray<UByte, Size>, detail::defaultSetterArray<UByte, Size>));
}

// Pointers to float color vector:
template<int Size>
inline Variable * Panel::addColorRO(const char * name, const float * clr)
{
    return addColorRO<Size>(NTB_NULL, name, clr, callbacks(detail::defaultGetterArray<float, Size>));
}
template<int Size>
inline Variable * Panel::addColorRO(Variable * parent, const char * name, const float * clr)
{
    return addColorRO<Size>(parent, name, clr, callbacks(detail::defaultGetterArray<float, Size>));
}
template<int Size>
inline Variable * Panel::addColorRW(const char * name, float * clr)
{
    return addColorRW<Size>(NTB_NULL, name, clr,
            callbacks(detail::defaultGetterArray<float, Size>, detail::defaultSetterArray<float, Size>));
}
template<int Size>
inline Variable * Panel::addColorRW(Variable * parent, const char * name, float * clr)
{
    return addColorRW<Size>(parent, name, clr,
            callbacks(detail::defaultGetterArray<float, Size>, detail::defaultSetterArray<float, Size>));
}

// Pointers to Color32:
inline Variable * Panel::addColorRO(const char * name, const Color32 * clr)
{
    return addColorRO(NTB_NULL, name, clr);
}
inline Variable * Panel::addColorRO(Variable * parent, const char * name, const Color32 * clr)
{
    return addColorRO<4>(parent, name, clr, callbacks(detail::defaultGetter<Color32>));
}
inline Variable * Panel::addColorRW(const char * name, Color32 * clr)
{
    return addColorRW(NTB_NULL, name, clr);
}
inline Variable * Panel::addColorRW(Variable * parent, const char * name, Color32 * clr)
{
    return addColorRW<4>(parent, name, clr,
            callbacks(detail::defaultGetter<Color32>, detail::defaultSetter<Color32>));
}

// Color var callbacks:
template<int Size, class OT, class CBT>
inline Variable * Panel::addColorRO(const char * name, const OT * obj, const CBT & cbs)
{
    return addColorRO<Size>(NTB_NULL, name, obj, cbs);
}
template<int Size, class OT, class CBT>
inline Variable * Panel::addColorRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs)
{
    #if NEO_TWEAK_BAR_CXX11_SUPPORTED
    static_assert(Size == 3 || Size == 4, "Color can have 3 or 4 components only!");
    #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

    typedef typename CBT::VarType VarType;
    typedef typename detail::ColorTS<VarType, Size>::Type ColorType;

    detail::VarImpl<OT, ColorType, detail::ColorEx, CBT> * var =
        NTB_NEW detail::VarImpl<OT, ColorType, detail::ColorEx, CBT>(this, parent, name, obj, cbs, Variable::ReadOnly);

    var->getDisplayValue().numChannels = Size;
    return addVarImpl(var);
}
template<int Size, class OT, class CBT>
inline Variable * Panel::addColorRW(const char * name, OT * obj, const CBT & cbs)
{
    return addColorRW<Size>(NTB_NULL, name, obj, cbs);
}
template<int Size, class OT, class CBT>
inline Variable * Panel::addColorRW(Variable * parent, const char * name, OT * obj, const CBT & cbs)
{
    #if NEO_TWEAK_BAR_CXX11_SUPPORTED
    static_assert(Size == 3 || Size == 4, "Color can have 3 or 4 components only!");
    #endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

    typedef typename CBT::VarType VarType;
    typedef typename detail::ColorTS<VarType, Size>::Type ColorType;

    detail::VarImpl<OT, ColorType, detail::ColorEx, CBT> * var =
        NTB_NEW detail::VarImpl<OT, ColorType, detail::ColorEx, CBT>(this, parent, name, obj, cbs, Variable::ReadWrite);

    var->getDisplayValue().numChannels = Size;
    return addVarImpl(var);
}

// ========================================================
// Panel => String vars:
// ========================================================

// By pointer to char buffer:
inline Variable * Panel::addStringRO(const char * name, const char * str)
{
    return addStringRO(NTB_NULL, name, str);
}
inline Variable * Panel::addStringRO(Variable * parent, const char * name, const char * str)
{
    return addStringRO(parent, name, str, callbacks(detail::defaultGetterCZStr<Panel::CStringMaxSize>));
}
template<int Size>
inline Variable * Panel::addStringRW(const char * name, char * str)
{
    Variable * var = addStringRW(NTB_NULL, name, str,
            callbacks(detail::defaultGetterCZStr<Size>, detail::defaultSetterCZStr<Size>));
    return var->setMaxStringSize(Size);
}
template<int Size>
inline Variable * Panel::addStringRW(Variable * parent, const char * name, char * str)
{
    Variable * var = addStringRW(parent, name, str,
            callbacks(detail::defaultGetterCZStr<Size>, detail::defaultSetterCZStr<Size>));
    return var->setMaxStringSize(Size);
}

// By pointer to std::string:
#if NEO_TWEAK_BAR_STD_STRING_INTEROP
inline Variable * Panel::addStringRO(const char * name, const std::string * str)
{
    return addStringRO(NTB_NULL, name, str);
}
inline Variable * Panel::addStringRO(Variable * parent, const char * name, const std::string * str)
{
    return addStringRO(parent, name, str, callbacks(detail::defaultGetter<std::string>));
}
inline Variable * Panel::addStringRW(const char * name, std::string * str)
{
    return addStringRW(NTB_NULL, name, str);
}
inline Variable * Panel::addStringRW(Variable * parent, const char * name, std::string * str)
{
    return addStringRW(parent, name, str,
            callbacks(detail::defaultGetter<std::string>, detail::defaultSetter<std::string>));
}
#endif // NEO_TWEAK_BAR_STD_STRING_INTEROP

// By pointer to native NTB SmallStr type:
inline Variable * Panel::addStringRO(const char * name, const SmallStr * str)
{
    return addStringRO(NTB_NULL, name, str);
}
inline Variable * Panel::addStringRO(Variable * parent, const char * name, const SmallStr * str)
{
    return addStringRO(parent, name, str, callbacks(detail::defaultGetter<SmallStr>));
}
inline Variable * Panel::addStringRW(const char * name, SmallStr * str)
{
    return addStringRW(NTB_NULL, name, str);
}
inline Variable * Panel::addStringRW(Variable * parent, const char * name, SmallStr * str)
{
    return addStringRW(parent, name, str,
            callbacks(detail::defaultGetter<SmallStr>, detail::defaultSetter<SmallStr>));
}

// String var callbacks:
template<class OT, class CBT>
inline Variable * Panel::addStringRO(const char * name, const OT * obj, const CBT & cbs)
{
    return addStringRO(NTB_NULL, name, obj, cbs);
}
template<class OT, class CBT>
inline Variable * Panel::addStringRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs)
{
    typedef typename CBT::VarType VarType;
    typedef typename detail::StrTS<VarType>::Type StrType;
    return addVarImpl(NTB_NEW detail::VarImpl<OT, StrType, SmallStr, CBT>(this, parent, name, obj, cbs, Variable::ReadOnly));
}
template<class OT, class CBT>
inline Variable * Panel::addStringRW(const char * name, OT * obj, const CBT & cbs)
{
    return addStringRW(NTB_NULL, name, obj, cbs);
}
template<class OT, class CBT>
inline Variable * Panel::addStringRW(Variable * parent, const char * name, OT * obj, const CBT & cbs)
{
    typedef typename CBT::VarType VarType;
    typedef typename detail::StrTS<VarType>::Type StrType;
    return addVarImpl(NTB_NEW detail::VarImpl<OT, StrType, SmallStr, CBT>(this, parent, name, obj, cbs, Variable::ReadWrite));
}

// ========================================================
// Panel => Raw pointer values:
// ========================================================

inline Variable * Panel::addPointerRO(const char * name, void * const * ptr)
{
    return addPointerRO(NTB_NULL, name, ptr);
}
inline Variable * Panel::addPointerRO(Variable * parent, const char * name, void * const * ptr)
{
    typedef detail::VarCallbacksCFuncPtr<void *, void *> CBT;
    const CBT cbs = callbacks(detail::defaultGetter<void *>);

    detail::VarImpl<void *, void *, detail::NumberEx, CBT> * var =
        NTB_NEW detail::VarImpl<void *, void *, detail::NumberEx, CBT>(this, parent, name, ptr, cbs, Variable::ReadOnly);

    var->getDisplayValue().type = detail::NumberEx::Pointer;
    var->getDisplayValue().format = NumFormat::Hexadecimal;
    return addVarImpl(var);
}
inline Variable * Panel::addPointerRW(const char * name, void ** ptr)
{
    return addPointerRW(NTB_NULL, name, ptr);
}
inline Variable * Panel::addPointerRW(Variable * parent, const char * name, void ** ptr)
{
    typedef detail::VarCallbacksCFuncPtr<void *, void *> CBT;
    const CBT cbs = callbacks(detail::defaultGetter<void *>);

    detail::VarImpl<void *, void *, detail::NumberEx, CBT> * var =
        NTB_NEW detail::VarImpl<void *, void *, detail::NumberEx, CBT>(this, parent, name, ptr, cbs, Variable::ReadWrite);

    var->getDisplayValue().type = detail::NumberEx::Pointer;
    var->getDisplayValue().format = NumFormat::Hexadecimal;
    return addVarImpl(var);
}

// ========================================================
// Panel => Enums:
// ========================================================

// Pointers:
template<class VT>
inline Variable * Panel::addEnumRO(const char * name, const VT * var, const EnumConstant<VT> * constants, const int numOfConstants)
{
    return addEnumRO(NTB_NULL, name, var, callbacks(detail::defaultGetter<VT>), constants, numOfConstants);
}
template<class VT>
inline Variable * Panel::addEnumRO(Variable * parent, const char * name, const VT * var, const EnumConstant<VT> * constants, const int numOfConstants)
{
    return addEnumRO(parent, name, var, callbacks(detail::defaultGetter<VT>), constants, numOfConstants);
}
template<class VT>
inline Variable * Panel::addEnumRW(const char * name, VT * var, const EnumConstant<VT> * constants, const int numOfConstants)
{
    return addEnumRW(NTB_NULL, name, var, callbacks(detail::defaultGetter<VT>, detail::defaultSetter<VT>), constants, numOfConstants);
}
template<class VT>
inline Variable * Panel::addEnumRW(Variable * parent, const char * name, VT * var, const EnumConstant<VT> * constants, const int numOfConstants)
{
    return addEnumRW(parent, name, var, callbacks(detail::defaultGetter<VT>, detail::defaultSetter<VT>), constants, numOfConstants);
}

// Callbacks:
template<class OT, class CBT, class VT>
inline Variable * Panel::addEnumRO(const char * name, const OT * obj, const CBT & cbs, const EnumConstant<VT> * constants, const int numOfConstants)
{
    return addEnumRO(NTB_NULL, name, obj, cbs, constants, numOfConstants);
}
template<class OT, class CBT, class VT>
inline Variable * Panel::addEnumRO(Variable * parent, const char * name, const OT * obj, const CBT & cbs, const EnumConstant<VT> * constants, const int numOfConstants)
{
    detail::VarImpl<OT, VT, detail::EnumValExImpl<VT>, CBT> * var =
        NTB_NEW detail::VarImpl<OT, VT, detail::EnumValExImpl<VT>, CBT>(this, parent, name, obj, cbs, Variable::ReadOnly);

    var->getDisplayValue().setConsts(constants, numOfConstants);
    return addVarImpl(var);
}
template<class OT, class CBT, class VT>
inline Variable * Panel::addEnumRW(const char * name, OT * obj, const CBT & cbs, const EnumConstant<VT> * constants, const int numOfConstants)
{
    return addEnumRW(NTB_NULL, name, obj, cbs, constants, numOfConstants);
}
template<class OT, class CBT, class VT>
inline Variable * Panel::addEnumRW(Variable * parent, const char * name, OT * obj, const CBT & cbs, const EnumConstant<VT> * constants, const int numOfConstants)
{
    detail::VarImpl<OT, VT, detail::EnumValExImpl<VT>, CBT> * var =
        NTB_NEW detail::VarImpl<OT, VT, detail::EnumValExImpl<VT>, CBT>(this, parent, name, obj, cbs, Variable::ReadWrite);

    var->getDisplayValue().setConsts(constants, numOfConstants);
    return addVarImpl(var);
}

// ========================================================
// Panel => User structures / hierarchy:
// ========================================================

inline Variable * Panel::addHierarchyParent(const char * name)
{
    return addVarImpl(NTB_NEW detail::VarHierarchyParent(this, NTB_NULL, name));
}
inline Variable * Panel::addHierarchyParent(Variable * parent, const char * name)
{
    return addVarImpl(NTB_NEW detail::VarHierarchyParent(this, parent, name));
}

// ========================================================
// Panel management helpers:
// ========================================================

template<class Func>
inline void Panel::enumerateAllVariables(Func fn)
{
    Variable * pVar = variables.getFirst<Variable>();
    for (int count = variables.getSize(); count--; pVar = pVar->getNext<Variable>())
    {
        fn(*pVar);
    }
}
template<class Func>
inline void Panel::enumerateAllVariables(Func fn) const
{
    const Variable * pVar = variables.getFirst<Variable>();
    for (int count = variables.getSize(); count--; pVar = pVar->getNext<Variable>())
    {
        fn(*pVar);
    }
}
inline int Panel::getVariablesCount() const
{
    return variables.getSize();
}
inline const SmallStr & Panel::getName() const
{
    return panelName;
}

// ================================================================================================
//
//                                   GUI class inline methods
//
// ================================================================================================

template<class Func>
inline void GUI::enumerateAllPanels(Func fn)
{
    Panel * pPanel = panels.getFirst<Panel>();
    for (int count = panels.getSize(); count--; pPanel = pPanel->getNext<Panel>())
    {
        fn(*pPanel);
    }
}
template<class Func>
inline void GUI::enumerateAllPanels(Func fn) const
{
    const Panel * pPanel = panels.getFirst<Panel>();
    for (int count = panels.getSize(); count--; pPanel = pPanel->getNext<Panel>())
    {
        fn(*pPanel);
    }
}
inline int GUI::getPanelCount() const
{
    return panels.getSize();
}
inline const SmallStr & GUI::getName() const
{
    return guiName;
}

} // namespace ntb {}

#endif // NEO_TWEAK_BAR_HPP
