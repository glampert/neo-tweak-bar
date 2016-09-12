
// ================================================================================================
// -*- C++ -*-
// File: ntb_utils.hpp
// Author: Guilherme R. Lampert
// Created on: 25/04/16
// Brief: Internal use functions, types and structures of the NeoTweakBar library.
// ================================================================================================

#ifndef NTB_UTILS_HPP
#define NTB_UTILS_HPP

#include "ntb.hpp"

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>

#include <memory>
#include <utility>
#include <algorithm>

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

namespace ntb
{

// ========================================================
// Assorted helper functions:
// ========================================================

UInt32 hashString(const char * cstr);
int copyString(char * dest, int destSizeInChars, const char * source);
bool intToString(UInt64 number, char * dest, int destSizeInChars, int numBase, bool isNegative);
int decodeUtf8(const char * encodedBuffer, int * outCharLength);

inline bool stringsEqual(const char * const a, const char * const b)
{
    NTB_ASSERT(a != NTB_NULL && b != NTB_NULL);
    return std::strcmp(a, b) == 0;
}

inline int lengthOfString(const char * const str)
{
    NTB_ASSERT(str != NTB_NULL);
    // Don't care about 32-64bit truncation.
    // Our strings are not nearly that long.
    return static_cast<int>(std::strlen(str));
}

// Lightens/darkens the given color by a percentage. Alpha channel remains unaltered.
// NOTE: The algorithm used is not very accurate!
Color32 lighthenRGB(Color32 color, Float32 percent);
Color32 darkenRGB(Color32 color, Float32 percent);

// Simple blending of colors by a given percentage.
Color32 blendColors(const Float32 color1[], const Float32 color2[], Float32 percent);
Color32 blendColors(Color32 color1, Color32 color2, Float32 percent);

// Hue Lightens Saturation <=> Red Green Blue conversions:
void RGBToHLS(Float32 fR, Float32 fG, Float32 fB, Float32 & hue, Float32 & light, Float32 & saturation);
void HLSToRGB(Float32 hue, Float32 light, Float32 saturation, Float32 & fR, Float32 & fG, Float32 & fB);

// ========================================================
// Internal memory allocator:
// ========================================================

template<typename T>
inline T * implAllocT(const UInt32 countInItems = 1)
{
    NTB_ASSERT(countInItems != 0);
    NTB_ASSERT(getShellInterface() != NTB_NULL);
    return static_cast<T *>(getShellInterface()->memAlloc(countInItems * sizeof(T)));
}

inline void implFree(void * ptrToFree)
{
    if (ptrToFree != NTB_NULL)
    {
        NTB_ASSERT(getShellInterface() != NTB_NULL);
        getShellInterface()->memFree(ptrToFree);
    }
}

template<typename T>
inline T * construct(T * obj)
{
    return ::new(obj) T;
}

template<typename T>
inline void destroy(T * obj)
{
    if (obj != NTB_NULL)
    {
        obj->~T();
    }
}

// ========================================================
// class PODArray:
// ========================================================

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
class PODArray NTB_FINAL_CLASS
{
    NTB_DISABLE_COPY_ASSIGN(PODArray);

public:

    // Constructor must receive the size in bytes of the stored type.
    explicit PODArray(int itemSizeBytes);
    PODArray(int itemSizeBytes, int sizeInItems);

    // Free all memory associated with the PODArray.
    ~PODArray();

    // memsets the whole array.
    void zeroFill();

    // Explicitly allocate storage or expand current. Size not changed.
    // No-op when new capacity is less than or equal the current.
    void allocate();

    // Also allocates some extra trailing slots for future array growths.
    void allocate(int capacityHint);

    // This one allocates the exact amount requested, without reserving
    // some extra to prevent new allocations on future array growths.
    // Use this method when you are sure the array will only be allocated once.
    void allocateExact(int capacityWanted);

    // Frees all memory and sets size & capacity to zero.
    void deallocate();

    // Ensure space is allocated and sets size to 'newSizeInItems'.
    // Newly allocated items are uninitialized. No-op if new size <= current size.
    void resize(int newSizeInItems);

    // Removes at index, shifting the array by one.
    void erase(int index);

    // Swap the last element of the array into the given index.
    // Unlike erase() this is constant time.
    void eraseSwap(int index);

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

    // Access item with cast and debug bounds checking:
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

    // Pointer to base address:
    // (T doesn't have to match itemSize in this case)
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

    // Miscellaneous accessors:
    bool isAllocated() const { return basePtr != NTB_NULL; }
    bool isEmpty()     const { return ctrl.used == 0; }
    int  getSize()     const { return ctrl.used;      }
    int  getCapacity() const { return ctrl.capacity;  }
    int  getItemSize() const { return ctrl.itemSize;  }
    void clear()             { setSize(0);            }

private:

    void initInternal(int itemBytes);
    void setNewStorage(UInt8 * newMemory);

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
    UInt8 * basePtr;
};

// ========================================================
// class SmallStr:
// ========================================================

// Simple dynamically sized string class with SSO - Small
// String Optimization for strings under 40 characters.
// A small buffer of chars is kept inline with the object
// to avoid a dynamic memory alloc for small strings.
// It can also grow to accommodate arbitrarily sized strings.
// The overall design is somewhat similar to std::string.
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
            implFree(backingStore.dynamic);
        }
        // else storage is inline with the object.
    }

    SmallStr(const char * str)
    {
        NTB_ASSERT(str != NTB_NULL);
        initInternal(str, lengthOfString(str));
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
        append(str, lengthOfString(str));
        return *this;
    }

    void setMaxSize(const int numChars)
    {
        NTB_ASSERT(numChars <= 65536);
        ctrl.maxSize = numChars;
    }
    void setCString(const char * str)
    {
        NTB_ASSERT(str != NTB_NULL);
        setCString(str, lengthOfString(str));
    }

    // Array access operator:
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

    // C-string access: (c_str() name for compatibility with std::string)
    const char * c_str() const
    {
        return !isDynamic() ? backingStore.fixed : backingStore.dynamic;
    }
    char * c_str()
    {
        return !isDynamic() ? backingStore.fixed : backingStore.dynamic;
    }

    // Compare against other SmallStr or a char* string:
    bool operator == (const SmallStr & other) const
    {
        return std::strcmp(c_str(), other.c_str()) == 0;
    }
    bool operator != (const SmallStr & other) const
    {
        return std::strcmp(c_str(), other.c_str()) != 0;
    }
    bool operator == (const char * const str) const
    {
        NTB_ASSERT(str != NTB_NULL);
        return std::strcmp(c_str(), str) == 0;
    }
    bool operator != (const char * const str) const
    {
        NTB_ASSERT(str != NTB_NULL);
        return std::strcmp(c_str(), str) != 0;
    }

    // Miscellaneous accessors:
    bool isDynamic()   const { return ctrl.capacity > static_cast<int>(sizeof(backingStore)); }
    bool isEmpty()     const { return ctrl.length == 0; }
    int  getLength()   const { return ctrl.length;      }
    int  getCapacity() const { return ctrl.capacity;    }
    int  getMaxSize()  const { return ctrl.maxSize;     }

    // String manipulation:
    void setCString(const char * str, int len);
    void resize(int newLength, bool preserveOldStr = true, char fillVal = '\0');
    void append(char c);
    void append(const char * str, int len);
    void erase(int index);
    void insert(int index, char c);
    void clear();

    // Covert numbers/pointers/vectors to string:
    static SmallStr fromPointer(const void * ptr, const int base = 16);
    static SmallStr fromNumber(const Float64 num, const int base = 10);
    static SmallStr fromNumber(const Int64 num,   const int base = 10);
    static SmallStr fromNumber(const UInt64 num,  const int base = 10);
    static SmallStr fromFloatVec(const Float32 vec[], const int elemCount, const char * prefix = "");

private:

    void initInternal(const char * str, int len);
    void reallocInternal(int newCapacity, bool preserveOldStr);

    #pragma pack(push, 1)
    // Bitfield used here to avoid additional
    // unnecessary padding and because we don't
    // really need the full range of an integer
    // for SmallStr. So the 3 fields are packed
    // into 64-bits, making the total structure
    // size = 48 bytes.
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
    union Backing
    {
        char * dynamic;
        char fixed[40];
    } backingStore;
    #pragma pack(pop)
};

// ========================================================
// template class ListNode<T>:
// ========================================================

// Some structures within the library are ordered using
// intrusive lists. They inherit from this node type.
template<typename T>
class ListNode
{
public:

    T * prev;
    T * next;

    ListNode() : prev(NTB_NULL), next(NTB_NULL) { }
    bool isLinked() const { return prev != NTB_NULL && next != NTB_NULL; }

protected:

    // List nodes are not meant to be directly deleted,
    // so we don't need a public virtual destructor.
    ~ListNode() { }
};

// ========================================================
// template class IntrusiveList<T>:
// ========================================================

// Intrusive doubly-linked list. Items inserted
// into the structure must inherit from ListNode.
// Items cannot be members of more than one list
// at any given time (we try to assert for that).
// Items also can't be inserted into the same list
// more than once. Size is stored, so getSize() is O(N).
// List is circularly referenced: head<->tail are linked.
template<typename T>
class IntrusiveList NTB_FINAL_CLASS
{
    NTB_DISABLE_COPY_ASSIGN(IntrusiveList);

public:

    // Constructs an empty list.
    IntrusiveList() : head(NTB_NULL), size(0) { }

    //
    // pushFront/pushBack:
    // Append at the head or tail of the list.
    // Both are constant time. Node must not be null!
    //
    void pushFront(T * node)
    {
        NTB_ASSERT(node != NTB_NULL);
        NTB_ASSERT(!node->isLinked()); // A node can only be a member of one list at a time!

        if (!isEmpty())
        {
            // head->prev points the tail, tail->next points back to the head.
            T * tail = head->prev;
            node->next = head;
            head->prev = node;
            node->prev = tail;
            tail->next = node;
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
    void pushBack(T * node)
    {
        NTB_ASSERT(node != NTB_NULL);
        NTB_ASSERT(!node->isLinked()); // A node can only be a member of one list at a time!

        if (!isEmpty())
        {
            // head->prev points the tail, and vice-versa:
            T * tail = head->prev;
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
    // popFront/popBack:
    // Removes one node from the head or tail of the list, without destroying the object.
    // Returns the removed node or null if the list is already empty. Both are constant time.
    //
    T * popFront()
    {
        if (isEmpty())
        {
            return NTB_NULL;
        }

        T * removedNode = head;
        T * tail = head->prev;

        head = head->next;
        head->prev = tail;
        tail->next = head;
        --size;

        removedNode->prev = NTB_NULL;
        removedNode->next = NTB_NULL;
        return removedNode;
    }
    T * popBack()
    {
        if (isEmpty())
        {
            return NTB_NULL;
        }

        T * tail = head->prev;
        T * removedNode = tail;

        head->prev = tail->prev;
        tail->prev->next = head;
        --size;

        removedNode->prev = NTB_NULL;
        removedNode->next = NTB_NULL;
        return removedNode;
    }

    //
    // Unlink nodes from anywhere in the list:
    //
    void unlink(T * node)
    {
        // Note: Assumes the node is linked to THIS LIST.
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
            T * nodePrev = node->prev;
            T * nodeNext = node->next;
            nodePrev->next = nodeNext;
            nodeNext->prev = nodePrev;
            node->prev = NTB_NULL;
            node->next = NTB_NULL;
            --size;
        }
    }
    void unlinkAndFree(T * node)
    {
        unlink(node);
        destroy(node);
        implFree(node);
    }
    void unlinkAll()
    {
        T * node = head;
        while (size--)
        {
            T * tmp = node;
            node = node->next;

            NTB_ASSERT(tmp != NTB_NULL);
            tmp->prev = NTB_NULL;
            tmp->next = NTB_NULL;
        }
        head = NTB_NULL;
    }
    void unlinkAndFreeAll()
    {
        T * node = head;
        while (size--)
        {
            T * tmp = node;
            node = node->next;

            destroy(tmp);
            implFree(tmp);
        }
        head = NTB_NULL;
    }

    // Access the head or tail elements of the doubly-linked list:
    T * getFirst() const { return head; }
    T * getLast()  const { return isEmpty() ? NTB_NULL : head->prev; }

    // Constant-time queries:
    int  getSize() const { return size; }
    bool isEmpty() const { return size == 0; }

private:

    T * head; // Head of list, null if list empty. Circularly referenced.
    int size; // Number of items currently in the list. 0 if list empty.
};

// ========================================================
// struct Point:
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
// struct Rectangle:
// ========================================================

struct Rectangle
{
    int xMins;
    int yMins;
    int xMaxs;
    int yMaxs;

    void set(const int x0, const int y0, const int x1, const int y1)
    {
        xMins = x0;
        yMins = y0;
        xMaxs = x1;
        yMaxs = y1;
    }
    void set(const int viewport[4])
    {
        xMins = viewport[0];
        yMins = viewport[1];
        xMaxs = viewport[0] + viewport[2];
        yMaxs = viewport[1] + viewport[3];
    }
    void setZero()
    {
        xMins = 0;
        yMins = 0;
        xMaxs = 0;
        yMaxs = 0;
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

    Rectangle & moveBy(const int displacementX, const int displacementY)
    {
        xMins += displacementX;
        yMins += displacementY;
        xMaxs += displacementX;
        yMaxs += displacementY;
        return *this;
    }
    Rectangle & expandWidth(const Rectangle & other)
    {
        xMins = std::min(xMins, other.xMins);
        xMaxs = std::max(xMaxs, other.xMaxs);
        return *this;
    }

    Float32 getAspect() const
    {
        return static_cast<Float32>(getWidth()) /
               static_cast<Float32>(getHeight());
    }

    int getX()      const { return xMins; }
    int getY()      const { return yMins; }
    int getWidth()  const { return xMaxs - xMins; }
    int getHeight() const { return yMaxs - yMins; }
};

// This could be a constructor, but I want to keep Rectangle as a POD type.
inline Rectangle makeRect(const int x0, const int y0, const int x1, const int y1)
{
    Rectangle rect = { x0, y0, x1, y1 };
    return rect;
}

// ========================================================
// struct Vec3:
// ========================================================

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

    static Float32 dot(const Vec3 & a, const Vec3 & b)
    {
        return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    }
    static Vec3 cross(const Vec3 & a, const Vec3 & b)
    {
        Vec3 result;
        result.x = (a.y * b.z) - (a.z * b.y);
        result.y = (a.z * b.x) - (a.x * b.z);
        result.z = (a.x * b.y) - (a.y * b.x);
        return result;
    }

    static Float32 length(const Vec3 & v)
    {
        return std::sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
    }
    static Vec3 normalize(const Vec3 & v)
    {
        const Float32 invLen = 1.0f / Vec3::length(v);
        Vec3 result = { v.x * invLen, v.y * invLen, v.z * invLen };
        return result;
    }
};

inline Vec3 makeVec3(const Float32 x, const Float32 y, const Float32 z)
{
    Vec3 result = { x, y, z };
    return result;
}

// ========================================================
// struct Vec4:
// ========================================================

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

// ========================================================
// struct Mat4x4:
// ========================================================

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

    void setIdentity();
    void setRows(const Vec4 & row0, const Vec4 & row1, const Vec4 & row2, const Vec4 & row3);

    // Rotation matrices:
    static Mat4x4 rotationX(Float32 radians);
    static Mat4x4 rotationY(Float32 radians);
    static Mat4x4 rotationZ(Float32 radians);

    // Translation/scaling:
    static Mat4x4 translation(Float32 x, Float32 y, Float32 z);
    static Mat4x4 scaling(Float32 x, Float32 y, Float32 z);

    // Left-handed look-at view/camera matrix. Up vector is normally the unit Y axis.
    static Mat4x4 lookAt(const Vec3 & eye, const Vec3 & target, const Vec3 & upVector);

    // Left-handed perspective projection matrix.
    static Mat4x4 perspective(Float32 fovYRadians, Float32 aspect, Float32 zNear, Float32 zFar);

    // Multiply (or combine) two matrices.
    static Mat4x4 multiply(const Mat4x4 & a, const Mat4x4 & b);

    // Multiply the 3D point by the matrix, transforming it. Returns a 4D vector.
    static Vec4 transformPoint(const Vec3 & p, const Mat4x4 & m);

    // Multiply the 3D point by the matrix. Assumes w = 1 and the last column of the matrix is padding.
    static Vec3 transformPointAffine(const Vec3 & p, const Mat4x4 & m);

    // Multiply the homogeneous 4D vector with the given matrix, as a row vector, from left to right.
    static Vec4 transformVector(const Vec4 & v, const Mat4x4 & m);
};

// ========================================================
// Geometry helpers:
// ========================================================

struct SphereVert
{
    Vec3 position;
    Color32 color;
};

struct ArrowVert
{
    Vec3 position;
    Vec3 normal;
};

struct BoxVert
{
    Vec3 position;
    Vec3 normal;
    Float32 u, v;
    Color32 color;
};

inline Float32 degToRad(const Float32 degrees)
{
    return degrees * (3.1415926535897931f / 180.0f);
}

inline Float32 radToDeg(const Float32 radians)
{
    return radians * (180.0f / 3.1415926535897931f);
}

inline bool angleNearZero(const Float32 num)
{
    return std::fabs(num) <= 0.01f;
}

inline Float32 normalizeAngle360(Float32 degrees)
{
    if (degrees >= 360.0f || degrees < 0.0f)
    {
        degrees -= std::floor(degrees * (1.0f / 360.0f)) * 360.0f;
    }
    return degrees;
}

inline Float32 normalizeAngle180(Float32 degrees)
{
    degrees = normalizeAngle360(degrees);
    if (degrees > 180.0f)
    {
        degrees -= 360.0f;
    }
    return degrees;
}

inline Float32 lerpAngles(Float32 a, Float32 b, const Float32 t)
{
    // Ensure we wrap around the shortest way.
    a = normalizeAngle180(a);
    b = normalizeAngle180(b);
	return (a + t * (b - a));
}

void makeTexturedBoxGeometry(BoxVert vertsOut[24], UInt16 indexesOut[36], const Color32 faceColors[6], Float32 width, Float32 height, Float32 depth);
void screenProjectionXY(VertexPTC & vOut, const VertexPTC & vIn, const Mat4x4 & viewProjMatrix, const Rectangle & viewport);

} // namespace ntb {}

#endif // NTB_UTILS_HPP
