
// ================================================================================================
// -*- C++ -*-
// File: ntb_utils.cpp
// Author: Guilherme R. Lampert
// Created on: 25/04/16
// Brief: Internal use functions, types and structures of the NeoTweakBar library.
// ================================================================================================

#include "ntb_utils.hpp"

namespace ntb
{

// ========================================================
// Assorted helper functions:
// ========================================================

UInt32 hashString(const char * cstr)
{
    NTB_ASSERT(cstr != nullptr);

    // Simple and fast One-at-a-Time (OAT) hash algorithm:
    //  http://en.wikipedia.org/wiki/Jenkins_hash_function
    //
    UInt32 h = 0;
    while (*cstr != '\0')
    {
        h += *cstr++;
        h += (h << 10);
        h ^= (h >>  6);
    }
    h += (h <<  3);
    h ^= (h >> 11);
    h += (h << 15);
    return h;
}

int copyString(char * dest, int destSizeInChars, const char * source)
{
    NTB_ASSERT(dest   != NTB_NULL);
    NTB_ASSERT(source != NTB_NULL);
    NTB_ASSERT(destSizeInChars > 0);

    // Copy until the end of source or until we run out of space in dest:
    char * ptr = dest;
    while ((*ptr++ = *source++) && --destSizeInChars > 0) { }

    // Truncate on overflow:
    if (destSizeInChars == 0)
    {
        *(--ptr) = '\0';
        errorF("Overflow in ntb::copyString()! Output was truncated.");
        return static_cast<int>(ptr - dest);
    }

    // Return the number of chars written to dest (not counting the null terminator).
    return static_cast<int>(ptr - dest - 1);
}

bool intToString(UInt64 number, char * dest, const int destSizeInChars, const int numBase, const bool isNegative)
{
    NTB_ASSERT(dest != NTB_NULL);
    NTB_ASSERT(destSizeInChars > 3); // - or 0x and a '\0'

    // Supports binary, octal, decimal and hexadecimal.
    const bool goodBase = (numBase == 2  || numBase == 8 ||
                           numBase == 10 || numBase == 16);
    if (!goodBase)
    {
        dest[0] = '\0';
        return errorF("Bad numeric base in ntb::intToString()!");
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
    } while (number > 0 && length < destSizeInChars);

    // Check for buffer overflow. Return an empty string in such case.
    if (length >= destSizeInChars)
    {
        dest[0] = '\0';
        return errorF("Buffer overflow in integer => string conversion!");
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
    } while (firstDigit < ptr);

    // Converted successfully.
    return true;
}

int decodeUtf8(const char * encodedBuffer, int * outCharLength)
{
    // Reference: http://en.wikipedia.org/wiki/Utf8
    //
    // Adapted from code found on the "AngelCode Toolbox Library"
    //  http://www.angelcode.com/dev/bmfonts/

    NTB_ASSERT(encodedBuffer != NTB_NULL);
    NTB_ASSERT(outCharLength != NTB_NULL);

    const UInt8 * buf = reinterpret_cast<const UInt8 *>(encodedBuffer);
    int value  =  0;
    int length = -1;
    UInt8 byte = buf[0];

    if ((byte & 0x80) == 0)
    {
        // This is the only byte
        (*outCharLength) = 1;
        return byte;
    }
    else if ((byte & 0xE0) == 0xC0)
    {
        // There is one more byte
        value  = static_cast<int>(byte & 0x1F);
        length = 2;

        // The value at this moment must not be less than 2, because
        // that should have been encoded with one byte only.
        if (value < 2)
        {
            length = -1;
        }
    }
    else if ((byte & 0xF0) == 0xE0)
    {
        // There are two more bytes
        value  = static_cast<int>(byte & 0x0F);
        length = 3;
    }
    else if ((byte & 0xF8) == 0xF0)
    {
        // There are three more bytes
        value  = static_cast<int>(byte & 0x07);
        length = 4;
    }

    int n = 1;
    for (; n < length; ++n)
    {
        byte = buf[n];
        if ((byte & 0xC0) == 0x80)
        {
            value = (value << 6) + static_cast<int>(byte & 0x3F);
        }
        else
        {
            break;
        }
    }

    if (n == length)
    {
        (*outCharLength) = length;
        return value;
    }

    // The byte sequence isn't a valid UTF-8 sequence.
    return -1;
}

Color32 lighthenRGB(const Color32 color, const Float32 percent)
{
    UInt8 bR, bG, bB, bA;
    unpackColor(color, bR, bG, bB, bA);

    const Float32 scale = percent / 100.0f;

    Float32 fR = byteToFloat(bR);
    fR += fR * scale;
    fR = std::min(fR, 1.0f);

    Float32 fG = byteToFloat(bG);
    fG += fG * scale;
    fG = std::min(fG, 1.0f);

    Float32 fB = byteToFloat(bB);
    fB += fB * scale;
    fB = std::min(fB, 1.0f);

    return packColor(floatToByte(fR), floatToByte(fG), floatToByte(fB), bA);
}

Color32 darkenRGB(const Color32 color, const Float32 percent)
{
    UInt8 bR, bG, bB, bA;
    unpackColor(color, bR, bG, bB, bA);

    const Float32 scale = percent / 100.0f;

    Float32 fR = byteToFloat(bR);
    fR -= fR * scale;
    fR = std::max(fR, 0.0f);

    Float32 fG = byteToFloat(bG);
    fG -= fG * scale;
    fG = std::max(fG, 0.0f);

    Float32 fB = byteToFloat(bB);
    fB -= fB * scale;
    fB = std::max(fB, 0.0f);

    return packColor(floatToByte(fR), floatToByte(fG), floatToByte(fB), bA);
}

Color32 blendColors(const Float32 color1[], const Float32 color2[], const Float32 percent)
{
    const Float32 t = 1.0f - percent;
    const Float32 fR = (t * color1[0]) + (percent * color2[0]);
    const Float32 fG = (t * color1[1]) + (percent * color2[1]);
    const Float32 fB = (t * color1[2]) + (percent * color2[2]);
    const Float32 fA = (t * color1[3]) + (percent * color2[3]);

    return packColor(floatToByte(fR), floatToByte(fG),
                     floatToByte(fB), floatToByte(fA));
}

Color32 blendColors(const Color32 color1, const Color32 color2, const Float32 percent)
{
    UInt8 r1, g1, b1, a1;
    unpackColor(color1, r1, g1, b1, a1);
    const Float32 fR1 = byteToFloat(r1);
    const Float32 fG1 = byteToFloat(g1);
    const Float32 fB1 = byteToFloat(b1);
    const Float32 fA1 = byteToFloat(a1);

    UInt8 r2, g2, b2, a2;
    unpackColor(color2, r2, g2, b2, a2);
    const Float32 fR2 = byteToFloat(r2);
    const Float32 fG2 = byteToFloat(g2);
    const Float32 fB2 = byteToFloat(b2);
    const Float32 fA2 = byteToFloat(a2);

    const Float32 t = 1.0f - percent;
    const Float32 finalR = (t * fR1) + (percent * fR2);
    const Float32 finalG = (t * fG1) + (percent * fG2);
    const Float32 finalB = (t * fB1) + (percent * fB2);
    const Float32 finalA = (t * fA1) + (percent * fA2);

    return packColor(floatToByte(finalR), floatToByte(finalG),
                     floatToByte(finalB), floatToByte(finalA));
}

void RGBToHLS(const Float32 fR, const Float32 fG, const Float32 fB, Float32 & hue, Float32 & light, Float32 & saturation)
{
    // Compute HLS from RGB. The R,G,B triplet is between [0,1],
    // hue is between [0,360], light and saturation are [0,1].

    Float32 r = 0.0f;
    Float32 g = 0.0f;
    Float32 b = 0.0f;

    if (fR > 0.0f) { r = fR;   }
    if (r  > 1.0f) { r = 1.0f; }
    if (fG > 0.0f) { g = fG;   }
    if (g  > 1.0f) { g = 1.0f; }
    if (fB > 0.0f) { b = fB;   }
    if (b  > 1.0f) { b = 1.0f; }

    Float32 minVal = r;
    if (g < minVal) { minVal = g; }
    if (b < minVal) { minVal = b; }

    Float32 maxVal = r;
    if (g > maxVal) { maxVal = g; }
    if (b > maxVal) { maxVal = b; }

    const Float32 mDiff = maxVal - minVal;
    const Float32 mSum  = maxVal + minVal;
    const Float32 l     = 0.5f * mSum;

    Float32 rNorm = 0.0f;
    Float32 gNorm = 0.0f;
    Float32 bNorm = 0.0f;

    light = l;

    if (maxVal != minVal)
    {
        rNorm = (maxVal - r) / mDiff;
        gNorm = (maxVal - g) / mDiff;
        bNorm = (maxVal - b) / mDiff;
    }
    else
    {
        saturation = hue = 0.0f;
        return;
    }

    if (l < 0.5f)
    {
        saturation = mDiff / mSum;
    }
    else
    {
        saturation = mDiff / (2.0f - mSum);
    }

    if (r == maxVal)
    {
        hue = 60.0f * (6.0f + bNorm - gNorm);
    }
    else if (g == maxVal)
    {
        hue = 60.0f * (2.0f + rNorm - bNorm);
    }
    else
    {
        hue = 60.0f * (4.0f + gNorm - rNorm);
    }

    if (hue > 360.0f)
    {
        hue -= 360.0f;
    }
}

void HLSToRGB(const Float32 hue, const Float32 light, const Float32 saturation, Float32 & fR, Float32 & fG, Float32 & fB)
{
    // Compute RGB from HLS. The light and saturation are between [0,1]
    // and hue is between [0,360]. The returned R,G,B triplet is between [0,1].

    Float32 rh = 0.0f;
    Float32 rl = 0.0f;
    Float32 rs = 0.0f;

    if (hue > 0.0f)
    {
        rh = hue;
    }
    if (rh > 360.0f)
    {
        rh = 360.0f;
    }

    if (light > 0.0f)
    {
        rl = light;
    }
    if (rl > 1.0f)
    {
        rl = 1.0f;
    }

    if (saturation > 0.0f)
    {
        rs = saturation;
    }
    if (rs > 1.0f)
    {
        rs = 1.0f;
    }

    Float32 rm2;
    if (rl <= 0.5f)
    {
        rm2 = rl * (1.0f + rs);
    }
    else
    {
        rm2 = rl + rs - rl * rs;
    }

    const Float32 rm1 = 2.0f * rl - rm2;

    if (!rs)
    {
        fR = rl;
        fG = rl;
        fB = rl;
    }
    else
    {
        struct Helper
        {
            static Float32 HLS2RGB(Float32 a, Float32 b, Float32 h)
            {
                if (h > 360.0f) { h = h - 360.0f; }
                if (h < 0.0f  ) { h = h + 360.0f; }
                if (h < 60.0f ) { return a + (b - a) * h / 60.0f; }
                if (h < 180.0f) { return b; }
                if (h < 240.0f) { return a + (b - a) * (240.0f - h) / 60.0f; }
                return a;
            }
        };
        fR = Helper::HLS2RGB(rm1, rm2, rh + 120.0f);
        fG = Helper::HLS2RGB(rm1, rm2, rh);
        fB = Helper::HLS2RGB(rm1, rm2, rh - 120.0f);
    }
}

// ========================================================
// class PODArray:
// ========================================================

PODArray::PODArray(const int itemSizeBytes)
{
    // Default constructor creates an empty array.
    // First insertion will init with default capacity.
    // No memory is allocated until them.
    initInternal(itemSizeBytes);
}

PODArray::PODArray(const int itemSizeBytes, const int sizeInItems)
{
    initInternal(itemSizeBytes);
    resize(sizeInItems);
    // New items are left uninitialized.
}

PODArray::~PODArray()
{
    deallocate();
}

void PODArray::initInternal(const int itemBytes)
{
    // Size we can fit in the 16 bits of ctrl.itemSize.
    NTB_ASSERT(itemBytes <= 65536);
    ctrl.used     = 0;
    ctrl.capacity = 0;
    ctrl.itemSize = itemBytes;
    basePtr       = NTB_NULL;
}

void PODArray::setNewStorage(UInt8 * newMemory)
{
    implFree(basePtr);
    basePtr = newMemory;
}

void PODArray::zeroFill()
{
    if (!isAllocated())
    {
        return;
    }

    std::memset(basePtr, 0, getCapacity() * getItemSize());
}

void PODArray::allocate()
{
    if (isAllocated())
    {
        return;
    }

    // Default to 2 initial slots plus allocation
    // extra added to all reallocations.
    allocate(2);
}

void PODArray::allocate(const int capacityHint)
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
    UInt8 * newMemory = implAllocT<UInt8>(newCapacity * itemSize);

    // Preserve old data, if any:
    if (getSize() > 0)
    {
        std::memcpy(newMemory, basePtr, getSize() * itemSize);
    }

    setCapacity(newCapacity);
    setNewStorage(newMemory);
}

void PODArray::allocateExact(const int capacityWanted)
{
    if (capacityWanted <= getCapacity())
    {
        return; // Doesn't shrink
    }

    const int itemSize = getItemSize();
    UInt8 * newMemory  = implAllocT<UInt8>(capacityWanted * itemSize);
    if (getSize() > 0) // Preserve old data, if any:
    {
        std::memcpy(newMemory, basePtr, getSize() * itemSize);
    }

    setCapacity(capacityWanted);
    setNewStorage(newMemory);
}

void PODArray::deallocate()
{
    if (!isAllocated())
    {
        return;
    }

    setNewStorage(NTB_NULL);
    setCapacity(0);
    setSize(0);
}

void PODArray::resize(const int newSizeInItems)
{
    if (newSizeInItems <= getSize())
    {
        return; // Doesn't shrink
    }

    allocate(newSizeInItems);
    setSize(newSizeInItems);
}

void PODArray::erase(const int index)
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

void PODArray::eraseSwap(const int index)
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

// ========================================================
// class SmallStr:
// ========================================================

void SmallStr::initInternal(const char * str, const int len)
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

void SmallStr::reallocInternal(int newCapacity, const bool preserveOldStr)
{
    const bool isDyn = isDynamic();

    // This many extra chars are added to the new capacity request to
    // avoid more allocations if the string grows again in the future.
    // This can be tuned for environments with more limited memory.
    newCapacity += 64;
    char * newMemory = implAllocT<char>(newCapacity);

    if (preserveOldStr)
    {
        std::memcpy(newMemory, c_str(), getLength() + 1);
    }
    if (isDyn)
    {
        implFree(backingStore.dynamic);
    }

    ctrl.capacity = newCapacity;
    backingStore.dynamic = newMemory;
}

void SmallStr::setCString(const char * str, const int len)
{
    NTB_ASSERT(str != NTB_NULL);

    if (len <= 0 || *str == '\0')
    {
        clear();
        return;
    }
    if (ctrl.maxSize > 0 && (len + 1) > ctrl.maxSize)
    {
        errorF("Setting SmallStr would overflow maxSize!");
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

void SmallStr::append(const char c)
{
    if (c == '\0')
    {
        return;
    }

    const int lengthNeeded = ctrl.length + 1;
    if (ctrl.maxSize > 0 && (lengthNeeded + 1) > ctrl.maxSize)
    {
        errorF("Appending to SmallStr would overflow maxSize!");
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

void SmallStr::append(const char * str, const int len)
{
    NTB_ASSERT(str != NTB_NULL);

    if (len <= 0 || *str == '\0')
    {
        return;
    }

    const int lengthNeeded = ctrl.length + len;
    if (ctrl.maxSize > 0 && (lengthNeeded + 1) > ctrl.maxSize)
    {
        errorF("Appending to SmallStr would overflow maxSize!");
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

void SmallStr::resize(const int newLength, const bool preserveOldStr, const char fillVal)
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
        errorF("Resizing SmallStr would overflow maxSize!");
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

void SmallStr::erase(int index)
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

void SmallStr::insert(int index, const char c)
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
        errorF("Inserting into SmallStr would overflow maxSize!");
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

void SmallStr::clear()
{
    // Does not free dynamic memory.
    ctrl.length = 0;
    c_str()[0]  = '\0';
}

// For the number => string converters below.
enum { NumConvBufSize = 128 };

SmallStr SmallStr::fromPointer(const void * ptr, const int base)
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
    else
    {
        // Cast to integer and display as decimal/bin/octal:
        return fromNumber(static_cast<UInt64>(reinterpret_cast<std::uintptr_t>(ptr)), base);
    }
}

SmallStr SmallStr::fromNumber(const Float64 num, const int base)
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
    else
    {
        // Cast to integer and display as hex/bin/octal:
        union F2I
        {
            Float64 asF64;
            UInt64  asU64;
        } val;
        val.asF64 = num;
        return fromNumber(val.asU64, base);
    }
}

SmallStr SmallStr::fromNumber(const Int64 num, const int base)
{
    char buffer[NumConvBufSize];
    intToString(static_cast<UInt64>(num), buffer, sizeof(buffer), base, (num < 0));
    return buffer;
}

SmallStr SmallStr::fromNumber(const UInt64 num, const int base)
{
    char buffer[NumConvBufSize];
    intToString(num, buffer, sizeof(buffer), base, false);
    return buffer;
}

SmallStr SmallStr::fromFloatVec(const Float32 vec[], const int elemCount, const char * prefix)
{
    NTB_ASSERT(elemCount > 0);

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

// ========================================================
// struct Mat4x4:
// ========================================================

void Mat4x4::setIdentity()
{
    rows[0].set(1.0f, 0.0f, 0.0f, 0.0f);
    rows[1].set(0.0f, 1.0f, 0.0f, 0.0f);
    rows[2].set(0.0f, 0.0f, 1.0f, 0.0f);
    rows[3].set(0.0f, 0.0f, 0.0f, 1.0f);
}

void Mat4x4::setRows(const Vec4 & row0, const Vec4 & row1, const Vec4 & row2, const Vec4 & row3)
{
    rows[0] = row0;
    rows[1] = row1;
    rows[2] = row2;
    rows[3] = row3;
}

Mat4x4 Mat4x4::rotationX(const Float32 radians)
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

Mat4x4 Mat4x4::rotationY(const Float32 radians)
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

Mat4x4 Mat4x4::rotationZ(const Float32 radians)
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

Mat4x4 Mat4x4::translation(const Float32 x, const Float32 y, const Float32 z)
{
    Mat4x4 result;
    result.rows[0].set(1.0f, 0.0f, 0.0f, 0.0f);
    result.rows[1].set(0.0f, 1.0f, 0.0f, 0.0f);
    result.rows[2].set(0.0f, 0.0f, 1.0f, 0.0f);
    result.rows[3].set(x,    y,    z,    1.0f);
    return result;
}

Mat4x4 Mat4x4::scaling(const Float32 x, const Float32 y, const Float32 z)
{
    Mat4x4 result;
    result.rows[0].set(x,    0.0f, 0.0f, 0.0f);
    result.rows[1].set(0.0f, y,    0.0f, 0.0f);
    result.rows[2].set(0.0f, 0.0f, z,    0.0f);
    result.rows[3].set(0.0f, 0.0f, 0.0f, 1.0f);
    return result;
}

Mat4x4 Mat4x4::lookAt(const Vec3 & eye, const Vec3 & target, const Vec3 & upVector)
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

Mat4x4 Mat4x4::perspective(const Float32 fovYRadians, const Float32 aspect, const Float32 zNear, const Float32 zFar)
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

Mat4x4 Mat4x4::multiply(const Mat4x4 & a, const Mat4x4 & b)
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

Vec4 Mat4x4::transformPoint(const Vec3 & p, const Mat4x4 & m)
{
    Vec4 result;
    const Vec4Ptr * M = m.getRows();
    result.x = (M[0][0] * p.x) + (M[1][0] * p.y) + (M[2][0] * p.z) + M[3][0];
    result.y = (M[0][1] * p.x) + (M[1][1] * p.y) + (M[2][1] * p.z) + M[3][1];
    result.z = (M[0][2] * p.x) + (M[1][2] * p.y) + (M[2][2] * p.z) + M[3][2];
    result.w = (M[0][3] * p.x) + (M[1][3] * p.y) + (M[2][3] * p.z) + M[3][3];
    return result;
}

Vec3 Mat4x4::transformPointAffine(const Vec3 & p, const Mat4x4 & m)
{
    Vec3 result;
    const Vec4Ptr * M = m.getRows();
    result.x = (M[0][0] * p.x) + (M[1][0] * p.y) + (M[2][0] * p.z) + M[3][0];
    result.y = (M[0][1] * p.x) + (M[1][1] * p.y) + (M[2][1] * p.z) + M[3][1];
    result.z = (M[0][2] * p.x) + (M[1][2] * p.y) + (M[2][2] * p.z) + M[3][2];
    return result;
}

Vec4 Mat4x4::transformVector(const Vec4 & v, const Mat4x4 & m)
{
    Vec4 result;
    const Vec4Ptr * M = m.getRows();
    result.x = (M[0][0] * v.x) + (M[1][0] * v.y) + (M[2][0] * v.z) + (M[3][0] * v.w);
    result.y = (M[0][1] * v.x) + (M[1][1] * v.y) + (M[2][1] * v.z) + (M[3][1] * v.w);
    result.z = (M[0][2] * v.x) + (M[1][2] * v.y) + (M[2][2] * v.z) + (M[3][2] * v.w);
    result.w = (M[0][3] * v.x) + (M[1][3] * v.y) + (M[2][3] * v.z) + (M[3][3] * v.w);
    return result;
}

// ========================================================
// Geometry helpers:
// ========================================================

void makeTexturedBoxGeometry(BoxVert vertsOut[24], UInt16 indexesOut[36], const Color32 faceColors[6],
                             const Float32 width, const Float32 height, const Float32 depth)
{
    //
    // -0.5,+0.5 indexed box:
    //
    static const UInt16 boxFaces[][4] =
    {
        { 0, 1, 5, 4 },
        { 4, 5, 6, 7 },
        { 7, 6, 2, 3 },
        { 1, 0, 3, 2 },
        { 1, 2, 6, 5 },
        { 0, 4, 7, 3 }
    };
    static const Float32 boxPositions[][3] =
    {
        { -0.5f, -0.5f, -0.5f },
        { -0.5f, -0.5f,  0.5f },
        {  0.5f, -0.5f,  0.5f },
        {  0.5f, -0.5f, -0.5f },
        { -0.5f,  0.5f, -0.5f },
        { -0.5f,  0.5f,  0.5f },
        {  0.5f,  0.5f,  0.5f },
        {  0.5f,  0.5f, -0.5f },
    };
    static const Float32 boxNormalVectors[][3] =
    {
        { -1.0f,  0.0f,  0.0f },
        {  0.0f,  1.0f,  0.0f },
        {  1.0f,  0.0f,  0.0f },
        {  0.0f, -1.0f,  0.0f },
        {  0.0f,  0.0f,  1.0f },
        {  0.0f,  0.0f, -1.0f }
    };
    static const Float32 boxTexCoords[][2] =
    {
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
        { 1.0f, 0.0f },
        { 0.0f, 0.0f }
    };

    const Color32 * pColor = faceColors;
    BoxVert       * pVert  = vertsOut;
    UInt16        * pFace  = indexesOut;

    // 'i' iterates over the faces, 2 triangles per face:
    UInt16 vertIndex = 0;
    for (int i = 0; i < 6; ++i, vertIndex += 4)
    {
        for (int j = 0; j < 4; ++j, ++pVert)
        {
            pVert->position.x = boxPositions[boxFaces[i][j]][0] * width;
            pVert->position.y = boxPositions[boxFaces[i][j]][1] * height;
            pVert->position.z = boxPositions[boxFaces[i][j]][2] * depth;
            pVert->normal.x = boxNormalVectors[i][0];
            pVert->normal.y = boxNormalVectors[i][1];
            pVert->normal.z = boxNormalVectors[i][2];
            pVert->u = boxTexCoords[j][0];
            pVert->v = boxTexCoords[j][1];
            pVert->color = *pColor;
        }
        ++pColor;
        pFace[0] = vertIndex;
        pFace[1] = vertIndex + 1;
        pFace[2] = vertIndex + 2;
        pFace += 3;
        pFace[0] = vertIndex + 2;
        pFace[1] = vertIndex + 3;
        pFace[2] = vertIndex;
        pFace += 3;
    }
}

void screenProjectionXY(VertexPTC & vOut, const VertexPTC & vIn, const Mat4x4 & viewProjMatrix, const Rectangle & viewport)
{
    // Project the vertex (we don't care about z/depth here):
    const Mat4x4::Vec4Ptr * M = viewProjMatrix.getRows();
    const Float32 vx = (M[0][0] * vIn.x) + (M[1][0] * vIn.y) + (M[2][0] * vIn.z) + M[3][0];
    const Float32 vy = (M[0][1] * vIn.x) + (M[1][1] * vIn.y) + (M[2][1] * vIn.z) + M[3][1];
    const Float32 vw = (M[0][3] * vIn.x) + (M[1][3] * vIn.y) + (M[2][3] * vIn.z) + M[3][3];

    // Perspective divide:
    const Float32 ndcX = vx / vw;
    const Float32 ndcY = vy / vw;

    // Map to window coordinates:
    vOut.x = (((ndcX * 0.5f) + 0.5f) * viewport.getWidth())  + viewport.getX();
    vOut.y = (((ndcY * 0.5f) + 0.5f) * viewport.getHeight()) + viewport.getY();
}

} // namespace ntb {}
