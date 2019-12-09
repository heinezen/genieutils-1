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

//    bool operator==(const ResourceFormat &r) const
//    {
//        return type == r.type && compCount == r.compCount && compByteWidth == r.compByteWidth &&
//                compType == r.compType && flags == r.flags;
//    }
//    bool operator<(const ResourceFormat &r) const
//    {
//        if(type != r.type)
//            return type < r.type;
//        if(compCount != r.compCount)
//            return compCount < r.compCount;
//        if(compByteWidth != r.compByteWidth)
//            return compByteWidth < r.compByteWidth;
//        if(compType != r.compType)
//            return compType < r.compType;
//        if(flags != r.flags)
//            return flags < r.flags;
//        return false;
//    }

//    bool operator!=(const ResourceFormat &r) const { return !(*this == r); }

    bool Special() const { return type != ResourceFormatType::Regular; }
    bool BGRAOrder() const { return (flags & ResourceFormat_BGRA) != 0; }
    bool SRGBCorrected() const { return compType == CompType::UNormSRGB; }

    uint32_t YUVSubsampling() const {
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

    void SetBGRAOrder(bool flag) {
        if(flag)
            flags |= ResourceFormat_BGRA;
        else
            flags &= ~ResourceFormat_BGRA;
    }

    void SetYUVSubsampling(uint32_t subsampling) {
        flags &= ~ResourceFormat_SubSample_Mask;
        if(subsampling == 444)
            flags |= ResourceFormat_444;
        else if(subsampling == 422)
            flags |= ResourceFormat_422;
        else if(subsampling == 420)
            flags |= ResourceFormat_420;
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

class DdsFile : public ISerializable
{
public:
    int width;
    int height;
    int depth;

    int mips;
    int slices;

    bool cubemap;

    ResourceFormat format;

    char **subdata;
    uint32_t *subsizes;

    bool load_dds_from_file();
    bool write_dds_to_file();

protected:
    void serializeObject() override;

private:
    static Logger &log;
};

} // namespace genie
