/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2019 Baldur Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#pragma once

#include "genie/file/ISerializable.h"
#include "genie/util/Logger.h"

namespace genie {
enum class CompType : uint8_t
{
    Typeless = 0,
    Float,
    UNorm,
    SNorm,
    UInt,
    SInt,
    UScaled,
    SScaled,
    Depth,
    Double,
    UNormSRGB,
};
enum class ResourceFormatType : uint8_t
{
    Regular = 0,
    Undefined,
    BC1,
    BC2,
    BC3,
    BC4,
    BC5,
    BC6,
    BC7,
    ETC2,
    EAC,
    ASTC,
    R10G10B10A2,
    R11G11B10,
    R5G6B5,
    R5G5B5A1,
    R9G9B9E5,
    R4G4B4A4,
    R4G4,
    D16S8,
    D24S8,
    D32S8,
    S8,
    YUV8,
    YUV10,
    YUV12,
    YUV16,
    PVRTC,
    A8,
};

struct ResourceFormat
{
    ResourceFormat()
    {
        type = ResourceFormatType::Undefined;

        compCount = compByteWidth = 0;
        compType = CompType::Typeless;

        flags = 0;
    }
    ResourceFormat(const ResourceFormat &) = default;
    ResourceFormat &operator=(const ResourceFormat &) = default;

    bool operator==(const ResourceFormat &r) const
    {
        return type == r.type && compCount == r.compCount && compByteWidth == r.compByteWidth &&
                compType == r.compType && flags == r.flags;
    }
    bool operator<(const ResourceFormat &r) const
    {
        if(type != r.type)
            return type < r.type;
        if(compCount != r.compCount)
            return compCount < r.compCount;
        if(compByteWidth != r.compByteWidth)
            return compByteWidth < r.compByteWidth;
        if(compType != r.compType)
            return compType < r.compType;
        if(flags != r.flags)
            return flags < r.flags;
        return false;
    }

    bool operator!=(const ResourceFormat &r) const { return !(*this == r); }
    //  rdcstr Name() const
    //  {
    //    rdcstr ret;
    //    RENDERDOC_ResourceFormatName(*this, ret);
    //    return ret;
    //  }

    bool Special() const { return type != ResourceFormatType::Regular; }
    bool BGRAOrder() const { return (flags & ResourceFormat_BGRA) != 0; }
    bool SRGBCorrected() const { return compType == CompType::UNormSRGB; }
    uint32_t YUVSubsampling() const
    {
        if(flags & ResourceFormat_444)
            return 444;
        else if(flags & ResourceFormat_422)
            return 422;
        else if(flags & ResourceFormat_420)
            return 420;
        return 0;
    }

    uint32_t YUVPlaneCount() const
    {
        if(flags & ResourceFormat_3Planes)
            return 3;
        else if(flags & ResourceFormat_2Planes)
            return 2;
        return 1;
    }

    void SetBGRAOrder(bool flag)
    {
        if(flag)
            flags |= ResourceFormat_BGRA;
        else
            flags &= ~ResourceFormat_BGRA;
    }

    void SetYUVSubsampling(uint32_t subsampling)
    {
        flags &= ~ResourceFormat_SubSample_Mask;
        if(subsampling == 444)
            flags |= ResourceFormat_444;
        else if(subsampling == 422)
            flags |= ResourceFormat_422;
        else if(subsampling == 420)
            flags |= ResourceFormat_420;
    }

    void SetYUVPlaneCount(uint32_t planes)
    {
        flags &= ~ResourceFormat_Planes_Mask;
        if(planes == 2)
            flags |= ResourceFormat_2Planes;
        else if(planes == 3)
            flags |= ResourceFormat_3Planes;
    }

    uint32_t ElementSize() const
    {
        switch(type)
        {
        case ResourceFormatType::Undefined: break;
        case ResourceFormatType::Regular: return compByteWidth * compCount;
        case ResourceFormatType::BC1:
        case ResourceFormatType::BC4:
            return 8;    // 8 bytes for 4x4 block
        case ResourceFormatType::BC2:
        case ResourceFormatType::BC3:
        case ResourceFormatType::BC5:
        case ResourceFormatType::BC6:
        case ResourceFormatType::BC7:
            return 16;    // 16 bytes for 4x4 block
        case ResourceFormatType::ETC2: return 8;
        case ResourceFormatType::EAC:
            if(compCount == 1)
                return 8;    // single channel R11 EAC
            else if(compCount == 2)
                return 16;    // two channel RG11 EAC
            else
                return 16;    // RGBA8 EAC
        case ResourceFormatType::ASTC:
            return 16;    // ASTC is always 128 bits per block
        case ResourceFormatType::R10G10B10A2:
        case ResourceFormatType::R11G11B10:
        case ResourceFormatType::R9G9B9E5: return 4;
        case ResourceFormatType::R5G6B5:
        case ResourceFormatType::R5G5B5A1:
        case ResourceFormatType::R4G4B4A4: return 2;
        case ResourceFormatType::R4G4: return 1;
        case ResourceFormatType::D16S8:
            return 3;    // we define the size as tightly packed, so 3 bytes.
        case ResourceFormatType::D24S8: return 4;
        case ResourceFormatType::D32S8:
            return 5;    // we define the size as tightly packed, so 5 bytes.
        case ResourceFormatType::S8:
        case ResourceFormatType::A8:
            return 1;
            // can't give a sensible answer for YUV formats as the texel varies.
        case ResourceFormatType::YUV8: return compCount;
        case ResourceFormatType::YUV10:
        case ResourceFormatType::YUV12:
        case ResourceFormatType::YUV16: return compCount * 2;
        case ResourceFormatType::PVRTC:
            return 8;    // our representation can't differentiate 2bpp from 4bpp, so guess
        }

        return 0;
    }

    ResourceFormatType type;

    CompType compType;
    uint8_t compCount;
    uint8_t compByteWidth;

private:
    enum
    {
        ResourceFormat_BGRA = 0x001,

        ResourceFormat_444 = 0x004,
        ResourceFormat_422 = 0x008,
        ResourceFormat_420 = 0x010,
        ResourceFormat_SubSample_Mask = 0x01C,

        ResourceFormat_2Planes = 0x020,
        ResourceFormat_3Planes = 0x040,
        ResourceFormat_Planes_Mask = 0x060,
    };
    uint16_t flags;

    // make DoSerialise a friend so it can serialise flags
    template <typename SerialiserType>
    friend void DoSerialise(SerialiserType &ser, ResourceFormat &el);
};

class DdsFile
{
public:
    int width;
    int height;
    int depth;

    int mips;
    int slices;

    bool cubemap;

    ResourceFormat format;

    uint8_t **subdata;
    uint32_t *subsizes;

    ResourceFormat DXGIFormat2ResourceFormat(DXGI_FORMAT format);
    bool is_dds_file(FILE *f);
    bool load_dds_from_file(FILE *f);
    bool write_dds_to_file(FILE *f);

    static Logger &log;

};

} // namespace genie
