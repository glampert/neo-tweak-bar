
// ================================================================================================
// -*- C++ -*-
// File: ntb_utils.hpp
// Author: Guilherme R. Lampert
// Created on: 18/12/15
// Brief: Common internal use functions, types and structures of the NeoTweakBar library.
// ================================================================================================

#ifndef NEO_TWEAK_BAR_UTILS_HPP
#define NEO_TWEAK_BAR_UTILS_HPP

#include <cstddef>   // For std::size_t, basic types, etc
#include <cstring>   // For std::memcpy, std::strlen, etc
#include <utility>   // For std::swap, std::move
#include <algorithm> // For std::sort, std::min, std::max

#include <cmath> // tan
#include <cstdlib> // abs/malloc/free
#include <cstdio> // snprintf

//
// Overridable assert() macro for NTB:
//
#ifndef NTB_ASSERT
    #include <cassert>
    #define NTB_ASSERT assert
#endif // NTB_ASSERT

// ========================================================
// TODO
// ========================================================

#define NEO_TWEAK_BAR_CXX11_SUPPORTED 1
#define NEO_TWEAK_BAR_STD_STRING_INTEROP 1

// *** TEMP ***
#include <iostream>
#include <string>

#if NEO_TWEAK_BAR_CXX11_SUPPORTED
    // For std::is_pod, std::is_pointer, etc.
    #include <type_traits>
#endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

//TODO c++98 compat!
#define NTB_DISABLE_COPY_ASSIGN(className) \
  private:                                 \
    className(const className &) = delete; \
    className & operator = (const className &) = delete

#define NTB_FINAL_CLASS    final
#define NTB_OVERRIDE       override
#define NTB_NULL           nullptr
#define NTB_CONSTEXPR_FUNC constexpr

//TODO TIDY; TEMP
#include <cstdio> // for the printf below
#define NTB_ERROR(message) std::printf("NTB ERROR: %s\n", message)

#if NEO_TWEAK_BAR_CXX11_SUPPORTED
    #define NTB_ALIGNED(expr, alignment) alignas(alignment) expr
#else // !C++11
    #if defined(__GNUC__) || defined(__clang__) // Clang & GCC
        #define NTB_ALIGNED(expr, alignment) expr __attribute__((aligned(alignment)))
    #elif defined(_MSC_VER) // Visual Studio
        #define NTB_ALIGNED(expr, alignment) __declspec(align(alignment)) expr
    #else // Unknown compiler
        #define NTB_ALIGNED(expr, alignment) expr /* hope for the best? */
    #endif // Compiler id switch
#endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

//TODO need a custom allocator...
//
// Preferably, should get rid of these macros.
// Probably define class-level new/delete for the classes that get dynamically allocated.
//
#define NTB_NEW    new
#define NTB_DELETE delete

//FIXME stdint is not so widely available...
#include <cstdint>
#include <cstdlib>

namespace ntb
{

typedef std::uint8_t  UByte;
typedef std::int8_t   Int8;
typedef std::uint8_t  UInt8;

typedef std::int16_t  Int16;
typedef std::uint16_t UInt16;

typedef std::int32_t  Int32;
typedef std::uint32_t UInt32;

typedef std::int64_t  Int64;
typedef std::uint64_t UInt64;

typedef float         Float32;
typedef double        Float64;

#if NEO_TWEAK_BAR_CXX11_SUPPORTED
    static_assert(sizeof(UByte)   == 1, "Expected 8-bits  integer!");
    static_assert(sizeof(Int8)    == 1, "Expected 8-bits  integer!");
    static_assert(sizeof(UInt8)   == 1, "Expected 8-bits  integer!");
    static_assert(sizeof(Int16)   == 2, "Expected 16-bits integer!");
    static_assert(sizeof(UInt16)  == 2, "Expected 16-bits integer!");
    static_assert(sizeof(Int32)   == 4, "Expected 32-bits integer!");
    static_assert(sizeof(UInt32)  == 4, "Expected 32-bits integer!");
    static_assert(sizeof(Int64)   == 8, "Expected 64-bits integer!");
    static_assert(sizeof(UInt64)  == 8, "Expected 64-bits integer!");
    static_assert(sizeof(Float32) == 4, "Expected 32-bits float!");
    static_assert(sizeof(Float64) == 8, "Expected 64-bits float!");
#endif // NEO_TWEAK_BAR_CXX11_SUPPORTED

//TODO replace with user supplied callbacks!
template<typename T>
inline T * memAlloc(const std::size_t countInItems) //NOTE: should just crash if out of mem! we don't check for null return!!!
{
    NTB_ASSERT(countInItems != 0);
    return static_cast<T *>(std::malloc(countInItems * sizeof(T)));
}
inline void memFree(void * ptr)
{
    if (ptr != NTB_NULL)
    {
        std::free(ptr);
    }
}

inline bool stringsEqual(const char * a, const char * b)
{
    NTB_ASSERT(a != NTB_NULL && b != NTB_NULL);
    return std::strcmp(a, b) == 0;
}

//TODO make non-inline
inline int copyString(char * dest, int destSizeInChars, const char * source)
{
    NTB_ASSERT(dest != NTB_NULL);
    NTB_ASSERT(source != NTB_NULL);
    NTB_ASSERT(destSizeInChars > 0);

    // Copy until the end of source or until we run out of space in dest:
    char * ptr = dest;
    while ((*ptr++ = *source++) && --destSizeInChars > 0) { }

    // Truncate on overflow:
    if (destSizeInChars == 0)
    {
        *(--ptr) = '\0';
        NTB_ERROR("Overflow in copyString()! Output was truncated.");
        return static_cast<int>(ptr - dest);
    }

    // Return the number of chars written to dest (not counting the null terminator).
    return static_cast<int>(ptr - dest - 1);
}

//MAKE NON-INLINE
inline bool intToString(UInt64 number, char * dest, const int destSizeInChars, const int numBase, const bool isNegative)
{
    NTB_ASSERT(dest != NTB_NULL);
    NTB_ASSERT(destSizeInChars > 3); // - or 0x and a '\0'

    // Supports binary, octal, decimal and hexadecimal.
    const bool goodBase = (numBase == 2  || numBase == 8 ||
                           numBase == 10 || numBase == 16);
    if (!goodBase)
    {
        NTB_ERROR("Bad numeric base in intToString()!");
        dest[0] = '\0';
        return false;
    }

    char * ptr = dest;
    int length = 0;

    if (numBase == 16)
    {
        // Add an "0x" in front of hexadecimal values:
        *ptr++ = '0';
        *ptr++ = 'x';
        length += 2;
    }
    else
    {
        if (isNegative && numBase == 10)
        {
            // Negative decimal, so output '-' and negate:
            length++;
            *ptr++ = '-';
            number = static_cast<UInt64>(-static_cast<Int64>(number));
        }
    }

    // Save pointer to the first digit so we can reverse the string later.
    char * firstDigit = ptr;

    // Main conversion loop:
    do
    {
        const int digitVal = number % numBase;
        number /= numBase;

        // Convert to ASCII and store:
        if (digitVal > 9)
        {
            *ptr++ = static_cast<char>((digitVal - 10) + 'A'); // A letter (hexadecimal)
        }
        else
        {
            *ptr++ = static_cast<char>(digitVal + '0'); // A digit
        }
        ++length;
    }
    while (number > 0 && length < destSizeInChars);

    // Check for buffer overflow. Return an empty string in such case.
    if (length >= destSizeInChars)
    {
        NTB_ERROR("Buffer overflow in integer => string conversion!");
        dest[0] = '\0';
        return false;
    }

    *ptr-- = '\0';

    // We now have the digits of the number in the buffer,
    // but in reverse order. So reverse the string now.
    do
    {
        const char tmp = *ptr;
        *ptr = *firstDigit;
        *firstDigit = tmp;

        --ptr;
        ++firstDigit;
    }
    while (firstDigit < ptr);

    // Converted successfully.
    return true;
}

NTB_CONSTEXPR_FUNC inline Float32 degToRad(const Float32 degrees)
{
    return degrees * (3.1415926535897931f / 180.0f);
}

NTB_CONSTEXPR_FUNC inline Float32 radToDeg(const Float32 radians)
{
    return radians * (180.0f / 3.1415926535897931f);
}

// Don't care about 32-64bit truncation. Our strings are not nearly that long.
inline int lengthOf(const char * str)
{
    NTB_ASSERT(str != NTB_NULL);
    return static_cast<int>(std::strlen(str));
}

template<typename T, int Size>
NTB_CONSTEXPR_FUNC inline int lengthOf(const T (&)[Size])
{
    return Size;
}

} // ntb

// ------------------------------------

namespace ntb
{

// ========================================================
// Color space conversion helpers:
// ========================================================

typedef UInt32 Color32;

// Remaps the value 'x' from one arbitrary min,max range to another.
template<typename T>
NTB_CONSTEXPR_FUNC inline T remap(const T x, const T inMin, const T inMax, const T outMin, const T outMax)
{
    return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

// Clamp 'x' between min/max bounds.
template<typename T>
NTB_CONSTEXPR_FUNC inline T clamp(const T x, const T minimum, const T maximum)
{
    return (x < minimum) ? minimum : (x > maximum) ? maximum : x;
}

// Byte in [0,255] range to Float32 in [0,1] range.
// Used for color space conversions.
NTB_CONSTEXPR_FUNC inline Float32 byteToFloat(const UByte b)
{
    return static_cast<Float32>(b) * (1.0f / 255.0f);
}

// Float in [0,1] range to byte in [0,255] range.
// Used for color space conversions. Note that 'f' is not clamped!
NTB_CONSTEXPR_FUNC inline UByte floatToByte(const Float32 f)
{
    return static_cast<UByte>(f * 255.0f);
}

// Pack each byte into an integer:
// 0x00-00-00-00
//   aa-rr-gg-bb
// Order will be ARGB, but APIs like OpenGL read it right-to-left as BGRA (GL_BGRA).
NTB_CONSTEXPR_FUNC inline Color32 packColor(const UByte r, const UByte g, const UByte b, const UByte a = 255)
{
    return static_cast<Color32>((a << 24) | (r << 16) | (g << 8) | b);
}

// Undo the work of packColor():
void unpackColor(Color32 color, UByte & r, UByte & g, UByte & b, UByte & a);

// Lightens/darkens the given color by a percentage. Alpha channel remains unaltered.
// NOTE: The algorithm used is not very accurate!
Color32 lighthenRGB(Color32 color, Float32 percent);
Color32 darkenRGB(Color32 color, Float32 percent);

// Very simple blending of Float32 RGBA [0,1] range colors by a given percentage.
Color32 blendColors(const Float32 color1[], const Float32 color2[], Float32 percent);
Color32 blendColors(Color32 color1, Color32 color2, Float32 percent);

// Hue Lightens Saturation <=> Red Green Blue conversions:
void RGBToHLS(Float32 fR, Float32 fG, Float32 fB, Float32 & hue, Float32 & light, Float32 & saturation);
void HLSToRGB(Float32 hue, Float32 light, Float32 saturation, Float32 & fR, Float32 & fG, Float32 & fB);

// ========================================================
// class PODArray:
//
// Dynamically growable sequential array similar to
// std::vector, but not copyable, so no returning it
// by value. This was intentional to prevent expensive
// accidental copies.
//
// Each reallocation will add some extra slots to the array.
// pushBack() reallocations will always double the current
// capacity plus add a few extra slots. Check 'allocExtra'
// in the implementation for details.
//
// NOTE: This class supports Plain Old Data (POD)
// types only! No constructor or destructor is called for
// the stored type. It also uses std::memcpy internally.
// ========================================================

//TODO make non-inline
class PODArray NTB_FINAL_CLASS
{
    NTB_DISABLE_COPY_ASSIGN(PODArray);

public:

    explicit PODArray(const int itemSizeBytes)
    {
        // Default constructor creates an empty array.
        // First insertion will init with default capacity.
        // No memory is allocated until them.
        initInternal(itemSizeBytes);
    }

    PODArray(const int itemSizeBytes, const int sizeInItems)
    {
        initInternal(itemSizeBytes);
        resize(sizeInItems);
        // New items are left uninitialized.
    }

    template<typename T>
    PODArray(const int itemSizeBytes, const int sizeInItems, const T & fillWith)
    {
        initInternal(itemSizeBytes);
        resize(sizeInItems);
        for (int i = 0; i < getSize(); ++i)
        {
            get<T>(i) = fillWith;
        }
    }

    ~PODArray()
    {
        deallocate();
    }

    void zeroFill()
    {
        if (!isAllocated()) { return; }
        std::memset(basePtr, 0, getCapacity() * getItemSize());
    }

    // Explicitly allocate storage or expand current. Size not changed.
    // No-op when new capacity is less than or equal the current.
    void allocate()
    {
        if (isAllocated()) { return; }

        // Default to 2 initial slots plus allocation
        // extra added to all reallocations.
        allocate(2);
    }

    // Allocates some extra trailing slots for future array growths.
    void allocate(const int capacityHint)
    {
        if (capacityHint <= getCapacity())
        {
            return; // Doesn't shrink
        }

        // Extra elements per allocation (powers of 2):
        const int itemSize   = getItemSize();
        const int allocExtra = (itemSize <= 1) ? 64 :
                               (itemSize <= 2) ? 32 :
                               (itemSize <= 4) ? 16 :
                               (itemSize <= 8) ?  8 : 4;

        const int newCapacity = capacityHint + allocExtra;
        UByte * newMemory = memAlloc<UByte>(newCapacity * itemSize);

        // Preserve old data, if any:
        if (getSize() > 0)
        {
            std::memcpy(newMemory, basePtr, getSize() * itemSize);
        }

        setCapacity(newCapacity);
        setNewStorage(newMemory);
    }

    // This one allocates the exact amount requested, without reserving
    // some extra to prevent new allocations on future array growths.
    // Use this method when you are sure the array will only be allocated once.
    void allocateExact(const int capacityWanted)
    {
        if (capacityWanted <= getCapacity())
        {
            return; // Doesn't shrink
        }

        const int itemSize = getItemSize();
        UByte * newMemory  = memAlloc<UByte>(capacityWanted * itemSize);
        if (getSize() > 0) // Preserve old data, if any:
        {
            std::memcpy(newMemory, basePtr, getSize() * itemSize);
        }

        setCapacity(capacityWanted);
        setNewStorage(newMemory);
    }

    // Frees all memory and sets size & capacity to zero.
    void deallocate()
    {
        if (!isAllocated()) { return; }
        setNewStorage(NTB_NULL);
        setCapacity(0);
        setSize(0);
    }

    // Ensure space is allocated and sets size to 'newSizeInItems'.
    // Newly allocated items are uninitialized. No-op if new size <= current size.
    void resize(const int newSizeInItems)
    {
        if (newSizeInItems <= getSize())
        {
            return; // Doesn't shrink
        }
        allocate(newSizeInItems);
        setSize(newSizeInItems);
    }

    // Append one element, possibly reallocating to make room.
    template<typename T>
    void pushBack(const T & item)
    {
        const int currSize = getSize();
        if (currSize == getCapacity())
        {
            // Double size on push back when depleted.
            // Fresh allocations start with 2 items.
            allocate(currSize > 0 ? (currSize * 2) : 2);
        }

        NTB_ASSERT(sizeof(T) == getItemSize());
        *reinterpret_cast<T *>(basePtr + (currSize * sizeof(T))) = item;
        setSize(currSize + 1);
    }

    // Decrement size by one, removing element at the end of the array.
    void popBack()
    {
        if (!isEmpty())
        {
            setSize(getSize() - 1);
        }
    }

    //TODO test this!
    //
    // Insert at index, possibly growing the array and shifting to the right to make room.
    template<typename T>
    void insert(const int index, const T & item)
    {
        // Inserting at the start of an empty array is permitted.
        if (isEmpty() && index == 0)
        {
            pushBack<T>(item);
            return;
        }

        // Otherwise, index must be valid.
        NTB_ASSERT(index >= 0 && index < getSize());
        NTB_ASSERT(sizeof(T) == getItemSize());

        const int currSize = getSize();
        allocate(currSize + 1);

        if (index < currSize)
        {
            std::memmove(basePtr + ((index + 1) * sizeof(T)),
                         basePtr + (index * sizeof(T)),
                         (currSize - index) * sizeof(T));
        }

        *reinterpret_cast<T *>(basePtr + (index * sizeof(T))) = item;
        setSize(currSize + 1);
    }

    //TODO test this!
    //
    // Removes at index, shifting the array by one.
    void erase(const int index)
    {
        NTB_ASSERT(index >= 0 && index < getSize());

        const int newSize = getSize() - 1;
        if (newSize > 0) // If it wasn't the last item, we shift the remaining:
        {
            const int remaining = newSize - (index + 1);
            if (remaining > 0)
            {
                const int itemSize = getItemSize();
                std::memmove(basePtr + (index * itemSize),
                             basePtr + ((index + 1) * itemSize),
                             remaining * itemSize);
            }
        }
        setSize(newSize);
    }

    //TODO test this!
    //
    // Swap the last element of the array into the given index.
    // Unlike erase() this is constant time.
    void eraseSwap(const int index)
    {
        NTB_ASSERT(index >= 0 && index < getSize());

        const int newSize = getSize() - 1;
        if (index != newSize)
        {
            const int itemSize = getItemSize();
            std::memmove(basePtr + (index * itemSize),
                         basePtr + (newSize * itemSize),
                         itemSize);
        }
        setSize(newSize);
    }

    //
    // Access item with cast and bounds checking:
    //
    template<typename T>
    const T & get(const int index) const
    {
        NTB_ASSERT(isAllocated());
        NTB_ASSERT(sizeof(T) == getItemSize());
        NTB_ASSERT(index >= 0 && index < getSize());
        return *reinterpret_cast<const T *>(basePtr + (index * sizeof(T)));
    }
    template<typename T>
    T & get(const int index)
    {
        NTB_ASSERT(isAllocated());
        NTB_ASSERT(sizeof(T) == getItemSize());
        NTB_ASSERT(index >= 0 && index < getSize());
        return *reinterpret_cast<T *>(basePtr + (index * sizeof(T)));
    }

    //
    // Pointer to base address:
    // (T doesn't have to match itemSize in this case)
    //
    template<typename T>
    const T * getData() const
    {
        return reinterpret_cast<const T *>(basePtr);
    }
    template<typename T>
    T * getData()
    {
        return reinterpret_cast<T *>(basePtr);
    }

    //
    // Misc queries:
    //
    bool isAllocated() const { return basePtr != NTB_NULL; }
    bool isEmpty()     const { return ctrl.used == 0; }
    int  getSize()     const { return ctrl.used;      }
    int  getCapacity() const { return ctrl.capacity;  }
    int  getItemSize() const { return ctrl.itemSize;  }
    void clear()             { setSize(0);            }

private:

    void initInternal(const int itemBytes)
    {
        // Size we can fit in the 16 bits of ctrl.itemSize.
        NTB_ASSERT(itemBytes <= 65536);
        ctrl.used     = 0;
        ctrl.capacity = 0;
        ctrl.itemSize = itemBytes;
        basePtr       = NTB_NULL;
    }

    void setNewStorage(UByte * newMemory)
    {
        memFree(basePtr);
        basePtr = newMemory;
    }

    void setSize(const int amount)     { ctrl.used     = amount; }
    void setCapacity(const int amount) { ctrl.capacity = amount; }
    void setItemSize(const int amount) { ctrl.itemSize = amount; }

    //
    // 4 or 8 bytes for the pointer and 64 bits
    // shared with the capacity/size/item-size.
    // PODArray total size should not be beyond
    // 16 bytes on a 64-bits architecture.
    //
    // Max item size is 65536 bytes (16-bits)
    //
    #pragma pack(push, 1)
    struct
    {
        int used     : 24; // Slots used by items.
        int capacity : 24; // Total slots allocated.
        int itemSize : 16; // Size in bytes of each item.
    } ctrl;
    #pragma pack(pop)

    // Pointer to first slot.
    UByte * basePtr;
};

// ========================================================
// class ListNode:
//
// Some structures within the library are ordered using
// intrusive lists. They inherit from this node type.
// ========================================================

//TODO make non-inline
class ListNode
{
public:

    ListNode()
        : prev(NTB_NULL)
        , next(NTB_NULL)
    { }

    bool isLinked() const { return prev != NTB_NULL && next != NTB_NULL; }
    template<typename T> T * getNext() const { return static_cast<T *>(next); }
    template<typename T> T * getPrev() const { return static_cast<T *>(prev); }

protected:

    // List nodes are not meant to be directly deleted,
    // so we don't need a public virtual destructor.
    ~ListNode() { }

private:

    // Only the list can directly access the member links.
    friend class IntrusiveList;
    ListNode * prev;
    ListNode * next;
};

// ========================================================
// class IntrusiveList:
//
// Intrusive doubly-linked list. Items inserted
// into the structure must inherit from ListNode.
// Items cannot be members of more than one list
// at any given time (we try to assert for that).
// Items also can't be inserted into the same list
// more than once. Size is stored, so getSize() is O(N).
// List is circularly referenced: head<->tail are linked.
// ========================================================

//TODO make non-inline
class IntrusiveList NTB_FINAL_CLASS
{
    NTB_DISABLE_COPY_ASSIGN(IntrusiveList);

public:

    // Constructs an empty list.
    IntrusiveList()
        : head(NTB_NULL)
        , size(0)
    { }

    //
    // pushFront/Back:
    // Append at the head or tail of the list.
    // Both are constant time. Node must not be null!
    //
    void pushFront(ListNode * node)
    {
        NTB_ASSERT(node != NTB_NULL);
        NTB_ASSERT(!node->isLinked()); // A node can only be a member of one list at a time!

        if (!isEmpty())
        {
            // head->prev points the tail, and vice-versa:
            ListNode * tail = head->prev;
            node->next = head;
            head->prev = node;
            node->prev = tail;
            head = node;
        }
        else // Empty list, first insertion:
        {
            head = node;
            head->prev = head;
            head->next = head;
        }
        ++size;
    }

    void pushBack(ListNode * node)
    {
        NTB_ASSERT(node != NTB_NULL);
        NTB_ASSERT(!node->isLinked()); // A node can only be a member of one list at a time!

        if (!isEmpty())
        {
            // head->prev points the tail, and vice-versa:
            ListNode * tail = head->prev;
            node->prev = tail;
            tail->next = node;
            node->next = head;
            head->prev = node;
        }
        else // Empty list, first insertion:
        {
            head = node;
            head->prev = head;
            head->next = head;
        }
        ++size;
    }

    //
    // popFront/Back:
    // Removes one node from head or tail of the list, without destroying the object.
    // Returns the removed node or null if the list is already empty. Both are constant time.
    //
    void popFront()
    {
        if (isEmpty()) { return; }

        ListNode * removedNode = head;
        ListNode * tail = head->prev;
        head = head->next;
        head->prev = tail;
        --size;

        removedNode->prev = NTB_NULL;
        removedNode->next = NTB_NULL;
    }

    void popBack()
    {
        if (isEmpty()) { return; }

        ListNode * removedNode = head->prev;
        ListNode * tail = head->prev;
        head->prev = tail->prev;
        tail->prev->next = head;
        --size;

        removedNode->prev = NTB_NULL;
        removedNode->next = NTB_NULL;
    }

    //
    // Unlink nodes from anywhere in the list:
    //
    void unlink(ListNode * node)
    {
        // Assumes the node is linked to THIS LIST.
        NTB_ASSERT(node != NTB_NULL);
        NTB_ASSERT(node->isLinked());
        NTB_ASSERT(!isEmpty());

        if (node == head) // Head
        {
            popFront();
        }
        else if (node == head->prev) // Tail
        {
            popBack();
        }
        else // Middle
        {
            ListNode * nodePrev = node->prev;
            ListNode * nodeNext = node->next;
            nodePrev->next = nodeNext;
            nodeNext->prev = nodePrev;
            node->prev = NTB_NULL;
            node->next = NTB_NULL;
            --size;
        }
    }

    void unlinkAndDelete(ListNode * node)
    {
        unlink(node);
        NTB_DELETE node;
    }

    void unlinkAll()
    {
        ListNode * node = head;
        while (size--)
        {
            ListNode * tmp = node;
            node = node->next;

            NTB_ASSERT(tmp != NTB_NULL);
            tmp->prev = NTB_NULL;
            tmp->next = NTB_NULL;
        }
        head = NTB_NULL;
    }

    void unlinkAndDeleteAll()
    {
        ListNode * node = head;
        while (size--)
        {
            ListNode * tmp = node;
            node = node->next;
            NTB_DELETE tmp;
        }
        head = NTB_NULL;
    }

    //
    // Access the head or tail elements
    // of the doubly-linked list.
    //
    template<typename T>
    T * getFirst() const
    {
        return static_cast<T *>(head);
    }
    template<typename T>
    T * getLast() const
    {
        if (isEmpty()) { return NTB_NULL; }
        return static_cast<T *>(head->prev);
    }

    //
    // Constant-time queries:
    //
    bool isEmpty() const { return size == 0; }
    int  getSize() const { return size; }

private:

    ListNode * head; // Head of list, null if list empty. Circularly referenced.
    int size;        // Number of items currently in the list. 0 if list empty.
};

// ========================================================
// class SmallStr:
//
// Simple dynamically sized string class with SSO - Small
// String Optimization for strings under 40 characters.
// A small buffer of chars is kept inline with the object
// to avoid a dynamic memory alloc for small strings.
// It can also grow to accommodate arbitrarily sized strings.
// The overall design is somewhat similar to std::string.
// ========================================================

//TODO make non-inline
//
//TODO 2: update the similar SmallStr class in lib-cfg!
// It still has the resize/setCString bugs!!!
//
class SmallStr NTB_FINAL_CLASS
{
public:

    SmallStr()
    {
        initInternal(NTB_NULL, 0);
    }

    ~SmallStr()
    {
        if (isDynamic())
        {
            memFree(backingStore.dynamic);
        }
        // else storage is inline with the object.
    }

    SmallStr(const char * str)
    {
        NTB_ASSERT(str != NTB_NULL);
        initInternal(str, lengthOf(str));
    }

    SmallStr(const char * str, const int len)
    {
        NTB_ASSERT(str != NTB_NULL);
        initInternal(str, len);
    }

    SmallStr(const SmallStr & other)
    {
        initInternal(other.c_str(), other.getLength());
        setMaxSize(other.getMaxSize());
    }

    SmallStr & operator = (const SmallStr & other)
    {
        setCString(other.c_str(), other.getLength());
        return *this;
    }

    SmallStr & operator = (const char * str)
    {
        setCString(str);
        return *this;
    }

    SmallStr & operator += (const SmallStr & other)
    {
        append(other.c_str(), other.getLength());
        return *this;
    }

    SmallStr & operator += (const char * str)
    {
        append(str, lengthOf(str));
        return *this;
    }

    void setCString(const char * str)
    {
        NTB_ASSERT(str != NTB_NULL);
        setCString(str, lengthOf(str));
    }

    void setCString(const char * str, const int len)
    {
        NTB_ASSERT(str != NTB_NULL);

        if (len <= 0 || *str == '\0')
        {
            clear();
            return;
        }
        if (ctrl.maxSize > 0 && (len + 1) > ctrl.maxSize)
        {
            NTB_ERROR("Setting SmallStr would overflow maxSize!");
            return;
        }
        if ((len + 1) > ctrl.capacity)
        {
            reallocInternal(len + 1, false);
        }

        std::memcpy(c_str(), str, len);
        c_str()[len] = '\0';
        ctrl.length = len;
    }

    void append(const char c)
    {
        if (c == '\0')
        {
            return;
        }

        const int lengthNeeded = ctrl.length + 1;
        if (ctrl.maxSize > 0 && (lengthNeeded + 1) > ctrl.maxSize)
        {
            NTB_ERROR("Appending to SmallStr would overflow maxSize!");
            return;
        }
        if ((lengthNeeded + 1) > ctrl.capacity)
        {
            reallocInternal(lengthNeeded + 1, true);
        }

        c_str()[lengthNeeded - 1] = c;
        c_str()[lengthNeeded] = '\0';
        ctrl.length = lengthNeeded;
    }

    void append(const char * str, const int len)
    {
        NTB_ASSERT(str != NTB_NULL);

        if (len <= 0 || *str == '\0')
        {
            return;
        }

        const int lengthNeeded = ctrl.length + len;
        if (ctrl.maxSize > 0 && (lengthNeeded + 1) > ctrl.maxSize)
        {
            NTB_ERROR("Appending to SmallStr would overflow maxSize!");
            return;
        }
        if ((lengthNeeded + 1) > ctrl.capacity)
        {
            reallocInternal(lengthNeeded + 1, true);
        }

        std::memcpy(c_str() + ctrl.length, str, len);
        c_str()[lengthNeeded] = '\0';
        ctrl.length = lengthNeeded;
    }

    void resize(const int newLength, const bool preserveOldStr = true, const char fillVal = '\0')
    {
        if (newLength <= 0)
        {
            clear();
            return;
        }
        else if (newLength == ctrl.length)
        {
            return;
        }
        else if (ctrl.maxSize > 0 && (newLength + 1) > ctrl.maxSize)
        {
            NTB_ERROR("Resizing SmallStr would overflow maxSize!");
            return;
        }

        if ((newLength + 1) > ctrl.capacity) // newLength doesn't include the NUL-terminator.
        {
            reallocInternal(newLength + 1, preserveOldStr);
        }

        if (!preserveOldStr || isEmpty())
        {
            std::memset(c_str(), fillVal, newLength);
        }
        else
        {
            if (newLength > ctrl.length) // Made longer?
            {
                std::memset(c_str() + ctrl.length, fillVal, (newLength - ctrl.length));
            }
            // If shrunk, just truncate.
        }

        c_str()[newLength] = '\0';
        ctrl.length = newLength;
    }

    void erase(int index)
    {
        const int len = ctrl.length;
        if (len == 0) // Empty string?
        {
            return;
        }

        if (index < 0)
        {
            index = 0;
        }
        else if (index >= len)
        {
            index = len - 1;
        }

        // Erase one char from an arbitrary position by shifting to the left:
        char * str = c_str();
        std::memmove(str + index, str + index + 1, (len - index) + 2);
        ctrl.length = len - 1;
    }

    void insert(int index, const char c)
    {
        int len = ctrl.length;
        if (len == 0 || index >= len) // Empty string or inserting past the end?
        {
            append(c);
            return;
        }

        ++len;
        if (index < 0) // Clamp and allow inserting at the beginning
        {
            index = 0;
        }

        if (ctrl.maxSize > 0 && (len + 1) > ctrl.maxSize)
        {
            NTB_ERROR("Inserting into SmallStr would overflow maxSize!");
            return;
        }
        if ((len + 1) > ctrl.capacity)
        {
            reallocInternal(len + 1, true);
        }

        // Shift the array by one (including the '\0'):
        char * str = c_str();
        std::memmove(str + index + 1, str + index, len - index);

        // Insert new:
        str[index]  = c;
        ctrl.length = len;
    }

    char & operator[] (const int index)
    {
        NTB_ASSERT(index >= 0 && index < getLength());
        return c_str()[index];
    }

    char operator[] (const int index) const
    {
        NTB_ASSERT(index >= 0 && index < getLength());
        return c_str()[index];
    }

    void clear()
    {
        // Does not free dynamic memory.
        ctrl.length = 0;
        c_str()[0] = '\0';
    }

    // Miscellaneous accessors:
    bool isDynamic() const
    {
        return ctrl.capacity > static_cast<int>(sizeof(backingStore));
    }
    bool isEmpty() const
    {
        return ctrl.length == 0;
    }
    int getLength() const
    {
        return ctrl.length;
    }
    int getCapacity() const
    {
        return ctrl.capacity;
    }
    int getMaxSize() const
    {
        return ctrl.maxSize;
    }
    void setMaxSize(const int numChars)
    {
        NTB_ASSERT(numChars <= 65536);
        ctrl.maxSize = numChars;
    }

    // C-string access: (c_str() name is compatible with std::string)
    const char * c_str() const
    {
        return !isDynamic() ? backingStore.fixed : backingStore.dynamic;
    }
    char * c_str()
    {
        return !isDynamic() ? backingStore.fixed : backingStore.dynamic;
    }

    //
    // Swap the internal contents of two SmallStrs:
    //

    friend void swap(SmallStr & first, SmallStr & second)
    {
        using std::swap;

        if (first.isDynamic() && second.isDynamic())
        {
            swap(first.ctrl, second.ctrl);
            swap(first.backingStore.dynamic, second.backingStore.dynamic);
        }
        else // Storage is inline with one/both the objects, need to strcpy.
        {
            SmallStr temp(first);
            first  = second;
            second = temp;
        }
    }

    //
    // Compare against other SmallStr or a char* string:
    //

    bool operator == (const SmallStr & other) const
    {
        return std::strcmp(c_str(), other.c_str()) == 0;
    }
    bool operator != (const SmallStr & other) const
    {
        return std::strcmp(c_str(), other.c_str()) != 0;
    }

    bool operator == (const char * str) const
    {
        NTB_ASSERT(str != NTB_NULL);
        return std::strcmp(c_str(), str) == 0;
    }
    bool operator != (const char * str) const
    {
        NTB_ASSERT(str != NTB_NULL);
        return std::strcmp(c_str(), str) != 0;
    }

    //
    // Covert numbers/pointers/vectors to string:
    //

    enum { NumConvBufSize = 128 };

    static SmallStr fromPointer(const void * ptr, const int base = 16)
    {
        // Hexadecimal is the default for pointers.
        if (base == 16)
        {
            // # of chars to output: ptr32 = 8, ptr64 = 16
            const int width = static_cast<int>(sizeof(void *) * 2);

            // size_t is big enough to hold the platform's pointer value.
            const std::size_t addr = reinterpret_cast<std::size_t>(ptr);

            char buffer[NumConvBufSize];
            std::snprintf(buffer, sizeof(buffer), "0x%0*zX", width, addr);
            buffer[sizeof(buffer) - 1] = '\0';
            return buffer;
        }

        // Cast to integer and display as decimal/bin/octal:
        return fromNumber(static_cast<UInt64>(reinterpret_cast<std::uintptr_t>(ptr)), base);
    }

    static SmallStr fromNumber(const Float64 num, const int base = 10)
    {
        if (base == 10)
        {
            char buffer[NumConvBufSize];
            std::snprintf(buffer, sizeof(buffer), "%f", num);
            buffer[sizeof(buffer) - 1] = '\0';

            // Trim trailing zeros to the right of the decimal point:
            for (char * ptr = buffer; *ptr != '\0'; ++ptr)
            {
                if (*ptr != '.')
                {
                    continue;
                }
                while (*++ptr != '\0') // Find the end of the string
                {
                }
                while (*--ptr == '0') // Remove trailing zeros
                {
                    *ptr = '\0';
                }
                if (*ptr == '.') // If the dot was left alone at the end, remove it
                {
                    *ptr = '\0';
                }
                break;
            }
            return buffer;
        }

        // Cast to integer and display as hex/bin/octal:
        union
        {
            Float64 asF64;
            UInt64  asU64;
        } val;
        val.asF64 = num;
        return fromNumber(val.asU64, base);
    }

    static SmallStr fromNumber(const Int64 num, const int base = 10)
    {
        char buffer[NumConvBufSize];
        intToString(static_cast<UInt64>(num), buffer, sizeof(buffer), base, (num < 0));
        return buffer;
    }

    static SmallStr fromNumber(const UInt64 num, const int base = 10)
    {
        char buffer[NumConvBufSize];
        intToString(num, buffer, sizeof(buffer), base, false);
        return buffer;
    }

    static SmallStr fromFloatVec(const Float32 vec[], const int elemCount, const char * prefix = "")
    {
        NTB_ASSERT(elemCount > 0 && elemCount <= 4);

        SmallStr str(prefix);
        str += "{";
        for (int i = 0; i < elemCount; ++i)
        {
            str += fromNumber(static_cast<Float64>(vec[i]));
            if (i != elemCount - 1)
            {
                str += ",";
            }
        }
        str += "}";

        return str;
    }

private:

    void initInternal(const char * str, const int len)
    {
        ctrl.length   =  0;
        ctrl.maxSize  = -1;
        ctrl.capacity = static_cast<int>(sizeof(backingStore));
        std::memset(&backingStore, 0, sizeof(backingStore));

        if (str != NTB_NULL)
        {
            setCString(str, len);
        }
    }

    void reallocInternal(int newCapacity, const bool preserveOldStr)
    {
        const bool isDyn = isDynamic();

        // This many extra chars are added to the new capacity request to
        // avoid more allocations if the string grows again in the future.
        // This can be tuned for environments with more limited memory.
        newCapacity += 64;
        char * newMemory = memAlloc<char>(newCapacity);

        if (preserveOldStr)
        {
            std::memcpy(newMemory, c_str(), getLength() + 1);
        }
        if (isDyn)
        {
            memFree(backingStore.dynamic);
        }

        ctrl.capacity = newCapacity;
        backingStore.dynamic = newMemory;
    }

    #pragma pack(push, 1)
    // Bitfield used here to avoid additional
    // unnecessary padding and because we don't
    // really need the full range of an integer
    // for SmallStr. So the 3 fields are packed
    // into 64-bits, making the total structure
    // size 48 bytes.
    // maxSize limit is 65536 (16-bits).
    // This is fine since it is only used by some
    // parts of the UI to constrain text-field lengths
    // and sizes of Panel string variables, which are
    // actually already capped to 256 chars anyways.
    struct
    {
        int length   : 24; // Chars used in string, not counting a '\0' at the end.
        int capacity : 24; // Total chars available for use.
        int maxSize  : 16; // Max size (counting the '\0') that this string is allowed to have.
                           // If -1, can have any size. This is only used by the UI text fields.
    } ctrl;

    // Either we are using the small fixed-size buffer
    // or the 'dynamic' field which is heap allocated.
    // Dynamic memory is in use if capacity > sizeof(backingStore)
    union
    {
        char * dynamic;
        char fixed[40];
    } backingStore;
    #pragma pack(pop)
};

// ========================================================
// Point structure (POD):
// For 2D screen-space points. Stores the point's X and Y.
// ========================================================

struct Point
{
    int x;
    int y;

    void set(const int px, const int py)
    {
        x = px;
        y = py;
    }

    void setZero()
    {
        x = 0;
        y = 0;
    }
};

// This could be a constructor, but I want to keep Point as a POD type.
inline Point makePoint(const int px, const int py)
{
    Point pt = { px, py };
    return pt;
}

// ========================================================
// Rectangle structure (POD):
// For screen-space rectangles. Stores the min/max points.
// ========================================================

struct Rectangle
{
    int xMins;
    int yMins;
    int xMaxs;
    int yMaxs;

    int getX()      const { return xMins; }
    int getY()      const { return yMins; }
    int getWidth()  const { return xMaxs - xMins; }
    int getHeight() const { return yMaxs - yMins; }
    int getArea()   const { return getWidth() * getHeight(); }

    Float32 getAspect() const
    {
        return static_cast<Float32>(getWidth()) /
               static_cast<Float32>(getHeight());
    }

    bool containsPoint(const Point p) const
    {
        if (p.x < xMins || p.x > xMaxs) { return false; }
        if (p.y < yMins || p.y > yMaxs) { return false; }
        return true;
    }

    bool containsPoint(const int x, const int y) const
    {
        if (x < xMins || x > xMaxs) { return false; }
        if (y < yMins || y > yMaxs) { return false; }
        return true;
    }

    Rectangle expanded(const int x, const int y) const
    {
        Rectangle rect = { xMins - x, yMins - y, xMaxs + x, yMaxs + y };
        return rect;
    }

    Rectangle shrunk(const int x, const int y) const
    {
        Rectangle rect = { xMins + x, yMins + y, xMaxs - x, yMaxs - y };
        return rect;
    }

    void moveBy(const int displacementX, const int displacementY)
    {
        xMins += displacementX;
        yMins += displacementY;
        xMaxs += displacementX;
        yMaxs += displacementY;
    }

    void expandWidth(const Rectangle & other)
    {
        xMins = std::min(xMins, other.xMins);
        xMaxs = std::max(xMaxs, other.xMaxs);
    }

    void set(const int x0, const int y0, const int x1, const int y1)
    {
        xMins = x0;
        yMins = y0;
        xMaxs = x1;
        yMaxs = y1;
    }

    void setZero()
    {
        xMins = 0;
        yMins = 0;
        xMaxs = 0;
        yMaxs = 0;
    }
};

// This could be a constructor, but I want to keep Rectangle as a POD type.
inline Rectangle makeRect(const int x0, const int y0, const int x1, const int y1)
{
    Rectangle rect = { x0, y0, x1, y1 };
    return rect;
}

// ================================================================================================
// TODO Note:
// Should probably replace the cmath functions with local wrappers
// to ease porting to platforms without double precision (which only
// have the f suffixed ones) or if people want to provide customs.
// Likely wrap that stuff into a math{} namespace (or a static Mathf class??)
//

struct Vec3
{
    Float32 x;
    Float32 y;
    Float32 z;

    void set(const Float32 xx, const Float32 yy, const Float32 zz)
    {
        x = xx;
        y = yy;
        z = zz;
    }

    void setZero()
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }

    static Vec3 subtract(const Vec3 & a, const Vec3 & b)
    {
        Vec3 result;
        result.x = a.x - b.x;
        result.y = a.y - b.y;
        result.z = a.z - b.z;
        return result;
    }

    static Vec3 add(const Vec3 & a, const Vec3 & b)
    {
        Vec3 result;
        result.x = a.x + b.x;
        result.y = a.y + b.y;
        result.z = a.z + b.z;
        return result;
    }

    static Vec3 cross(const Vec3 & a, const Vec3 & b)
    {
        Vec3 result;
        result.x = (a.y * b.z) - (a.z * b.y);
        result.y = (a.z * b.x) - (a.x * b.z);
        result.z = (a.x * b.y) - (a.y * b.x);
        return result;
    }

    static Vec3 normalize(const Vec3 & v)
    {
        Vec3 result;
        const Float32 invLen = 1.0f / Vec3::length(v);
        result.x = v.x * invLen;
        result.y = v.y * invLen;
        result.z = v.z * invLen;
        return result;
    }

    static Float32 length(const Vec3 & v)
    {
        return std::sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
    }

    static Float32 dot(const Vec3 & a, const Vec3 & b)
    {
        return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    }
};

inline Vec3 makeVec3(const Float32 x, const Float32 y, const Float32 z)
{
    Vec3 result = { x, y, z };
    return result;
}

struct Vec4
{
    Float32 x;
    Float32 y;
    Float32 z;
    Float32 w;

    void set(const Float32 xx, const Float32 yy, const Float32 zz, const Float32 ww)
    {
        x = xx;
        y = yy;
        z = zz;
        w = ww;
    }

    void setZero()
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        w = 0.0f;
    }
};

inline Vec4 makeVec4(const Float32 x, const Float32 y, const Float32 z, const Float32 w)
{
    Vec4 result = { x, y, z, w };
    return result;
}

struct Mat4x4
{
    typedef Float32 Vec4Ptr[4];
    Vec4 rows[4];

    Float32       * getData()       { return reinterpret_cast<Float32       *>(this); }
    const Float32 * getData() const { return reinterpret_cast<const Float32 *>(this); }

    Vec4Ptr       * getRows()       { return reinterpret_cast<Vec4Ptr       *>(this); }
    const Vec4Ptr * getRows() const { return reinterpret_cast<const Vec4Ptr *>(this); }

    Vec4       & operator[](const int row)       { NTB_ASSERT(row >= 0 && row < 4); return rows[row]; }
    const Vec4 & operator[](const int row) const { NTB_ASSERT(row >= 0 && row < 4); return rows[row]; }

    void setIdentity()
    {
        rows[0].set(1.0f, 0.0f, 0.0f, 0.0f);
        rows[1].set(0.0f, 1.0f, 0.0f, 0.0f);
        rows[2].set(0.0f, 0.0f, 1.0f, 0.0f);
        rows[3].set(0.0f, 0.0f, 0.0f, 1.0f);
    }

    void setRows(const Vec4 & row0, const Vec4 & row1, const Vec4 & row2, const Vec4 & row3)
    {
        rows[0] = row0;
        rows[1] = row1;
        rows[2] = row2;
        rows[3] = row3;
    }

    // Rotation matrices:
    static Mat4x4 rotationX(const Float32 radians)
    {
        Mat4x4 result;
        const Float32 c = std::cos(radians);
        const Float32 s = std::sin(radians);
        result.rows[0].set(1.0f,  0.0f, 0.0f, 0.0f);
        result.rows[1].set(0.0f,  c,    s,    0.0f);
        result.rows[2].set(0.0f, -s,    c,    0.0f);
        result.rows[3].set(0.0f,  0.0f, 0.0f, 1.0f);
        return result;
    }
    static Mat4x4 rotationY(const Float32 radians)
    {
        Mat4x4 result;
        const Float32 c = std::cos(radians);
        const Float32 s = std::sin(radians);
        result.rows[0].set(c,    0.0f, s,    0.0f),
        result.rows[1].set(0.0f, 1.0f, 0.0f, 0.0f);
        result.rows[2].set(-s,   0.0f, c,    0.0f);
        result.rows[3].set(0.0f, 0.0f, 0.0f, 1.0f);
        return result;
    }
    static Mat4x4 rotationZ(const Float32 radians)
    {
        Mat4x4 result;
        const Float32 c = std::cos(radians);
        const Float32 s = std::sin(radians);
        result.rows[0].set( c,   s,    0.0f, 0.0f);
        result.rows[1].set(-s,   c,    0.0f, 0.0f);
        result.rows[2].set(0.0f, 0.0f, 1.0f, 0.0f);
        result.rows[3].set(0.0f, 0.0f, 0.0f, 1.0f);
        return result;
    }

    // Translation/scaling:
    static Mat4x4 translation(const Float32 x, const Float32 y, const Float32 z)
    {
        Mat4x4 result;
        result.rows[0].set(1.0f, 0.0f, 0.0f, 0.0f);
        result.rows[1].set(0.0f, 1.0f, 0.0f, 0.0f);
        result.rows[2].set(0.0f, 0.0f, 1.0f, 0.0f);
        result.rows[3].set(x,    y,    z,    1.0f);
        return result;
    }
    static Mat4x4 scaling(const Float32 x, const Float32 y, const Float32 z)
    {
        Mat4x4 result;
        result.rows[0].set(x,    0.0f, 0.0f, 0.0f);
        result.rows[1].set(0.0f, y,    0.0f, 0.0f);
        result.rows[2].set(0.0f, 0.0f, z,    0.0f);
        result.rows[3].set(0.0f, 0.0f, 0.0f, 1.0f);
        return result;
    }

    // Left-handed look-at view/camera matrix. Up vector is normally the unit Y axis.
    static Mat4x4 lookAt(const Vec3 & eye, const Vec3 & target, const Vec3 & upVector)
    {
        const Vec3 look  = Vec3::normalize(Vec3::subtract(target, eye));
        const Vec3 right = Vec3::cross(Vec3::normalize(upVector), look);
        const Vec3 up    = Vec3::cross(look, right);

        const Float32 a = -Vec3::dot(right, eye);
        const Float32 b = -Vec3::dot(up,    eye);
        const Float32 c = -Vec3::dot(look,  eye);

        Mat4x4 result;
        result.rows[0].set(right.x, up.x, look.x, 0.0f);
        result.rows[1].set(right.y, up.y, look.y, 0.0f);
        result.rows[2].set(right.z, up.z, look.z, 0.0f);
        result.rows[3].set(a,       b,    c,      1.0f);
        return result;
    }

    // Left-handed perspective projection matrix.
    static Mat4x4 perspective(const Float32 fovYRadians, const Float32 aspect, const Float32 zNear, const Float32 zFar)
    {
        const Float32 invFovTan = 1.0f / std::tan(fovYRadians * 0.5f);
        const Float32 a = (aspect * invFovTan);
        const Float32 c = -(zFar + zNear) / (zFar - zNear);
        const Float32 e = (2.0f * zFar * zNear) / (zFar - zNear);

        Mat4x4 result;
        result.rows[0].set(a,    0.0f,      0.0f, 0.0f);
        result.rows[1].set(0.0f, invFovTan, 0.0f, 0.0f);
        result.rows[2].set(0.0f, 0.0f,      c,    1.0f);
        result.rows[3].set(0.0f, 0.0f,      e,    0.0f);
        return result;
    }

    // Multiply (or combine) two matrices.
    static Mat4x4 multiply(const Mat4x4 & a, const Mat4x4 & b)
    {
        Mat4x4 result;
        Vec4Ptr * R = result.getRows();
        const Vec4Ptr * A = a.getRows();
        const Vec4Ptr * B = b.getRows();
        R[0][0] = (A[0][0] * B[0][0]) + (A[0][1] * B[1][0]) + (A[0][2] * B[2][0]) + (A[0][3] * B[3][0]);
        R[0][1] = (A[0][0] * B[0][1]) + (A[0][1] * B[1][1]) + (A[0][2] * B[2][1]) + (A[0][3] * B[3][1]);
        R[0][2] = (A[0][0] * B[0][2]) + (A[0][1] * B[1][2]) + (A[0][2] * B[2][2]) + (A[0][3] * B[3][2]);
        R[0][3] = (A[0][0] * B[0][3]) + (A[0][1] * B[1][3]) + (A[0][2] * B[2][3]) + (A[0][3] * B[3][3]);
        R[1][0] = (A[1][0] * B[0][0]) + (A[1][1] * B[1][0]) + (A[1][2] * B[2][0]) + (A[1][3] * B[3][0]);
        R[1][1] = (A[1][0] * B[0][1]) + (A[1][1] * B[1][1]) + (A[1][2] * B[2][1]) + (A[1][3] * B[3][1]);
        R[1][2] = (A[1][0] * B[0][2]) + (A[1][1] * B[1][2]) + (A[1][2] * B[2][2]) + (A[1][3] * B[3][2]);
        R[1][3] = (A[1][0] * B[0][3]) + (A[1][1] * B[1][3]) + (A[1][2] * B[2][3]) + (A[1][3] * B[3][3]);
        R[2][0] = (A[2][0] * B[0][0]) + (A[2][1] * B[1][0]) + (A[2][2] * B[2][0]) + (A[2][3] * B[3][0]);
        R[2][1] = (A[2][0] * B[0][1]) + (A[2][1] * B[1][1]) + (A[2][2] * B[2][1]) + (A[2][3] * B[3][1]);
        R[2][2] = (A[2][0] * B[0][2]) + (A[2][1] * B[1][2]) + (A[2][2] * B[2][2]) + (A[2][3] * B[3][2]);
        R[2][3] = (A[2][0] * B[0][3]) + (A[2][1] * B[1][3]) + (A[2][2] * B[2][3]) + (A[2][3] * B[3][3]);
        R[3][0] = (A[2][0] * B[0][0]) + (A[3][1] * B[1][0]) + (A[3][2] * B[2][0]) + (A[3][3] * B[3][0]);
        R[3][1] = (A[2][0] * B[0][1]) + (A[3][1] * B[1][1]) + (A[3][2] * B[2][1]) + (A[3][3] * B[3][1]);
        R[3][2] = (A[2][0] * B[0][2]) + (A[3][1] * B[1][2]) + (A[3][2] * B[2][2]) + (A[3][3] * B[3][2]);
        R[3][3] = (A[2][0] * B[0][3]) + (A[3][1] * B[1][3]) + (A[3][2] * B[2][3]) + (A[3][3] * B[3][3]);
        return result;
    }

    // Multiply the 3D point by the matrix, transforming it. Returns a 4D vector.
    static Vec4 transformPoint(const Vec3 & p, const Mat4x4 & m)
    {
        Vec4 result;
        const Vec4Ptr * M = m.getRows();
        result.x = (M[0][0] * p.x) + (M[1][0] * p.y) + (M[2][0] * p.z) + M[3][0];
        result.y = (M[0][1] * p.x) + (M[1][1] * p.y) + (M[2][1] * p.z) + M[3][1];
        result.z = (M[0][2] * p.x) + (M[1][2] * p.y) + (M[2][2] * p.z) + M[3][2];
        result.w = (M[0][3] * p.x) + (M[1][3] * p.y) + (M[2][3] * p.z) + M[3][3];
        return result;
    }

    // Multiply the 3D point by the matrix. Assumes w = 1 and the last column of the matrix is padding.
    static Vec3 transformPointAffine(const Vec3 & p, const Mat4x4 & m)
    {
        Vec3 result;
        const Vec4Ptr * M = m.getRows();
        result.x = (M[0][0] * p.x) + (M[1][0] * p.y) + (M[2][0] * p.z) + M[3][0];
        result.y = (M[0][1] * p.x) + (M[1][1] * p.y) + (M[2][1] * p.z) + M[3][1];
        result.z = (M[0][2] * p.x) + (M[1][2] * p.y) + (M[2][2] * p.z) + M[3][2];
        return result;
    }

    // Multiply the homogeneous 4D vector with the given matrix, as a row vector, from left to right.
    static Vec4 transformVector(const Vec4 & v, const Mat4x4 & m)
    {
        Vec4 result;
        const Vec4Ptr * M = m.getRows();
        result.x = (M[0][0] * v.x) + (M[1][0] * v.y) + (M[2][0] * v.z) + (M[3][0] * v.w);
        result.y = (M[0][1] * v.x) + (M[1][1] * v.y) + (M[2][1] * v.z) + (M[3][1] * v.w);
        result.z = (M[0][2] * v.x) + (M[1][2] * v.y) + (M[2][2] * v.z) + (M[3][2] * v.w);
        result.w = (M[0][3] * v.x) + (M[1][3] * v.y) + (M[2][3] * v.z) + (M[3][3] * v.w);
        return result;
    }
};

} // namespace ntb {}

#endif // NEO_TWEAK_BAR_UTILS_HPP
