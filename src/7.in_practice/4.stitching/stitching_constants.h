

#pragma once

// #define TN_PLATFORM_LINUX_BST_AARCH64

namespace tn {
namespace stitching {

enum class OutputType {
    kStitchedFar = 0,
    kStitchedMedium,
    kStitchedNear,
    kStitchedNearRaw,
    kStitchedNearVPA,
    kStitchedCustom1,
    kPerspectiveAll,
    kPerspectiveFront,
    kPerspectiveRear,
    kPerspectiveLeft,
    kPerspectiveRight,
    kStitchedDebug,
    kSurroundThumbnailAll,
    kSurroundThumbnailFront,
    kSurroundThumbnailRear,
    kSurroundThumbnailLeft,
    kSurroundThumbnailRight,
    kInvalid
};

enum class SwitchType {
    kStitchedFar = 0,
    kStitchedMedium,
    kStitchedNear,
    kStitchedNearRaw,
    kStitchedNearVPA,
    kStitchedCustom1,
    kPerspectiveFront,
    kPerspectiveRear,
    kPerspectiveLeft,
    kPerspectiveRight,
    kBlindAreaReset
};

enum class TopicType : uint64_t
{
    kInvalid = 0,
    kCameraFramesAll = (1 << 0),
    kSurroundViewCameraFrames = (1 << 1),
    kSurroundViewCameraFramesThumbnail = (1 << 2),
    kFrontViewCameraFrames = (1 << 3),
    kStitchedAll = (1 << 4),
    kStitchedFar = (1 << 5),
    kStitchedMedium = (1 << 6),
    kStitchedNear = (1 << 7),
    kStitchedNearRaw = (1 << 8),
    kStitchedNearVPA = (1 << 9),
    kPerspectiveAll = (1 << 10),
    kPerspectiveFront = (1 << 11),
    kPerspectiveRear = (1 << 12),
    kPerspectiveLeft = (1 << 13),
    kPerspectiveRight = (1 << 14),
    kStitchedDebug = (1 << 15),
    kStitchedCustom1 = (1 << 16)
};

enum class StitchingControlType
{
    kNone = 0,
    kPauseAPAStitching,
    kResumeAPAStiching
};

}}
