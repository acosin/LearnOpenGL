/**
 * stitching_common.h
 *
 * 
 *
 * Created by jfzheng on 11/1/2020.
 * Copyright 2020 Telenav Inc. All rights reserved.
 *
 */

#ifndef STITCHING_COMMON_H
#define STITCHING_COMMON_H

#include <string>

#include "stitching_constants.h"

#ifdef UNIT_TEST
#define TN_PRIVATE public:
#else
#define TN_PRIVATE private:
#endif

// #define ENABLE_GL_CHECK
// #define ENABLE_EGL_CHECK

namespace tn
{
namespace stitching
{

enum RenderType : int
{
    kInvalidType = -1,
    kFarAvm,
    kMediumAvm,
    kNearAvm,
    kNearAvmVPA,
    kCustomAvm1,
    kPerspective,
    kSurroundViewCFThumbnail,
    kThirdPartyAVM
};

enum Position : int
{
    kInvalidPosition = -1,
    kFront,
    kRear,
    kLeft,
    kRight,
    kFrontLeft,
    kFrontRight,
    kRearLeft,
    kRearRight,
    kAllPosition,
    kMaxPosition
};

struct LutParam
{
    LutParam() : width(-1), height(-1),  src_image_width(-1),  src_image_height(-1) {}
    LutParam(const std::string& coord, int wid = 0, int ht = 0, int src_w = 0, int src_h = 0, int bpp = 0) : 
        coord_path(coord), width(wid), height(ht), src_image_width(src_w), src_image_height(src_h) {}
    std::string coord_path; // including name
    int width;
    int height;
    int src_image_width;
    int src_image_height;
};

struct CameraRenderParam
{
    CameraRenderParam() : grid_per_row(-1), grid_per_column(-1) {}
    CameraRenderParam(int row, int col) : grid_per_column(col), grid_per_row(row) {}
    int grid_per_row;
    int grid_per_column;
};

enum class ImageFormat {
    kInvalid = -1,
    kRgb = 0,
    kRgba,
    kBgr,
    kLuminance,
    kLuminanceAlpha,
    kNv12,
    kUyvy
};

inline OutputType GetOutputType(RenderType rtype, Position pos = Position::kAllPosition)
{
    OutputType ret = OutputType::kInvalid;
    switch (rtype)
    {
    case kFarAvm:
        ret = OutputType::kStitchedFar;
        break;
    case kMediumAvm:
        ret = OutputType::kStitchedMedium;
        break;
    case kNearAvm:
        ret = OutputType::kStitchedNear;
        break;
    case kNearAvmVPA:
        ret = OutputType::kStitchedNearVPA;
        break;
    case kCustomAvm1:
        ret = OutputType::kStitchedCustom1;
        break;
    case kPerspective:
        if (pos == Position::kFront)
            ret = OutputType::kPerspectiveFront;
        else if (pos == Position::kRear)
            ret = OutputType::kPerspectiveRear;
        else if (pos == Position::kLeft)
            ret = OutputType::kPerspectiveLeft;
        else if (pos == Position::kRight)
            ret = OutputType::kPerspectiveRight;
        else
            ret = OutputType::kPerspectiveAll;
        break;
    case kSurroundViewCFThumbnail:
        if (pos == Position::kFront)
            ret = OutputType::kSurroundThumbnailFront;
        else if (pos == Position::kRear)
            ret = OutputType::kSurroundThumbnailRear;
        else if (pos == Position::kLeft)
            ret = OutputType::kSurroundThumbnailLeft;
        else if (pos == Position::kRight)
            ret = OutputType::kSurroundThumbnailRight;
        else
            ret = OutputType::kSurroundThumbnailAll;
        break;
    
    default:
        break;
    }
    return ret;
}

inline RenderType GetRenderType(OutputType otype)
{
    RenderType ret = RenderType::kInvalidType;
    switch (otype)
    {
    case OutputType::kStitchedFar:
        ret = RenderType::kFarAvm;
        break;
    case OutputType::kStitchedMedium:
        ret = RenderType::kMediumAvm;
        break;
    case OutputType::kStitchedNear:
        ret = RenderType::kNearAvm;
        break;
    case OutputType::kStitchedNearVPA:
        ret = RenderType::kNearAvmVPA;
        break;
    case OutputType::kStitchedCustom1:
        ret = RenderType::kCustomAvm1;
        break;
    case OutputType::kPerspectiveFront:
    case OutputType::kPerspectiveRear:
    case OutputType::kPerspectiveLeft:
    case OutputType::kPerspectiveRight:
        ret = RenderType::kPerspective;
        break;
    
    default:
        break;
    }
    return ret;
}

inline ImageFormat ToImageFormatFrom(const std::string& img_fmt_name)
{
    if (img_fmt_name == "rgba")
    {
        return ImageFormat::kRgba;
    }
    else if (img_fmt_name == "bgr")
    {
        return ImageFormat::kBgr;
    }
    else
    {
        return ImageFormat::kRgb;
    }
}



void ShowApiInfo();



} // namespace stitching {
} // namespace tn {

#endif // #ifndef STITCHING_COMMON_H