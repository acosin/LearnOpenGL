#ifndef AVMTYPE
#define AVMTYPE
namespace tn
{

namespace stitching
{
    enum class ViewType : int
    {
        kAroundView = 0,
        kTopAroundView,
        kFrontTopAroundView,
        kRearTopAroundView,
        kFrontAroundView,
        kRearAroundView,
        kLeftAroundView,
        kRightAroundView,

        kFrontFishEye,
        kRightFishEye,
        kRearFishEye,
        kLeftFishEye,

        kFrontDewarp,
        kRightDewarp,
        kRearDewarp,
        kLeftDewarp,

        kFrontWideAngleDewarp,
        kRightWideAngleDewarp,
        kRearWideAngleDewarp,
        kLeftWideAngleDewarp,

        kFrontRightTire,
        kRearRightTire,
        kRearLeftTire,
        kFrontLeftTire,
        kTireAroundVIew,

        kSmallFrontDewarp,
        kSmallRightDewarp,
        kSmallRearDewarp,
        kSmallLeftDewarp,

        kApaFrontDewarp,
        kApaRightDewarp,
        kApaRearDewarp,
        kApaLeftDewarp,
        kApaTopAroundView,
        kCustomTopAroundView,

        kAutoCalibTopAround,
        kManualCalibTopAround,
        kManualCalibFrontFishEye,
        kManualCalibRightFishEye,
        kManualCalibRearFishEye,
        kManualCalibLeftFishEye,
        kRoadCalibTopAround,

    };
    enum class CameraPos : int
    {
        kFront = 0,
        kRear,
        kLeft,
        kRight,
        kFrontTire,
        kRearTire,
        kApaAroundView,
        kApaDewarpView, 
        kFishEye,
        kWideAngle
    };

    enum class AvmStatus
    {
        kShutingDown = 0,
        kStarting,
        kShowAvm,
        kCalibration,
        kHide,
        kFault
    };

    enum class EventType
    {
        kOneFingerDown = 0,
        kOneFingerUp,
        kOneFingerMoving,
        kZoomIn,
        kZoomOut,
        kRotate,
    };

    enum class SingleTouchEventType
    {
        kOneFingerDown,
        kOneFingerUp,
        kOneFingerMoving
    };

    enum class ComGearType:int
    {
        sNone = 0,
        sPark = 1,
        sReverse = 2,
        sNeutral = 3,
        sDrive = 4
    };

    enum class ComDrivingDirection:int
    {
        kStop = 0,
        kForward = 1,
        kBackward = 2,
        kUndefined
    };

    enum class ComLampType:int
    {
        sLeft,
        sRight,
        sBoth
    };

    enum class ComDoorInfo:int
    {
        sFrontLeft,
        sFrontRight,
        sRearLeft,
        sRearRight
    };

    enum class ComCamInfo:int
    {
        sFront = 0,
        sRear,
        sLeft,
        sRight
    };

    enum class ComCommandCtrlSts:int
    {
        sStart,
        sStop
    };

    enum class ComClickStatus:int
    {
        sClick,
        sNoClick
    };

    enum class CurVehicleStatus:int
    {
        sAvm,
        sApa,
        sEnterBackTracking,
        sBackTracking,
        sParking,
        sBackGround,
        sBackTrackToAvm,
        sTireView
    };

    struct CameraErrStatus
    {
        uint8_t status_sf = 0;
        uint8_t cover = 0;
        uint8_t img_invalid = 0;
        uint8_t link_down = 0;
        uint8_t module = 0;

        bool operator == (const CameraErrStatus& rhs)
        {
            return status_sf == rhs.status_sf &&
                cover == rhs.cover &&
                img_invalid == rhs.img_invalid &&
                link_down == rhs.link_down &&
                module == rhs.module;
        }

        bool operator != (const CameraErrStatus& rhs)
        {
            return !(*this == rhs);
        }

        bool Failue() const
        {
            return 0 != status_sf || 0 != img_invalid || 0 != link_down || 0 != module;
        }

        bool Covered() const
        {
            return 0 != cover;
        }
    };

    struct CameraCalibStatus
    {
        uint8_t signal_st = 0;
        uint8_t swc_st = 0;
        /*
        0 - calibrated; 1 - parse calibration file failed; 2 - CRC failed; 3 - not calibrated
        */
        uint8_t calibration = 0;

        bool operator==(const CameraCalibStatus& rhs)
        {
            return (signal_st == rhs.signal_st) && 
                (swc_st == rhs.swc_st) &&
                (calibration == rhs.calibration);
        }

        bool operator!=(const CameraCalibStatus& rhs)
        {
            return !(*this == rhs);
        }
    };

    struct UssMalStatus
    {
        bool sensor_error_st_has_error = false;
        bool sensor_blockage_st_block = false;  // 遮挡

        bool operator==(const UssMalStatus& rhs)
        {
            return (sensor_error_st_has_error == rhs.sensor_error_st_has_error) &&
                (sensor_blockage_st_block == rhs.sensor_blockage_st_block);
        }

        bool operator!=(const UssMalStatus& rhs)
        {
            return !(*this == rhs);
        }
    };

    struct UssErrStatus
    {
        UssMalStatus fls;
        UssMalStatus fl;
        UssMalStatus fml;
        UssMalStatus fmr;
        UssMalStatus fr;
        UssMalStatus frs;
        UssMalStatus rls;
        UssMalStatus rl;
        UssMalStatus rml;
        UssMalStatus rmr;
        UssMalStatus rr;
        UssMalStatus rrs;
        bool operator!=(const UssErrStatus& rhs)
        {
            return (fls != rhs.fls) ||
                (fl != rhs.fl) ||
                (fml != rhs.fml) ||
                (fmr != rhs.fmr) ||
                (fr != rhs.fr) ||
                (frs != rhs.frs) ||
                (rls != rhs.rls) ||
                (rl != rhs.rl) ||
                (rml != rhs.rml) ||
                (rmr != rhs.rmr) ||
                (rr != rhs.rr) ||
                (rrs != rhs.rrs);
        }
    };

    struct Settings
    {
        bool blind_area = false;
        bool obstacle_check = false;
        bool auto_avm = false; 
        uint8_t auto_view = true;
    };

    
    struct DoorsCoverInfo
    {
        bool fl = false;
        bool fr = false;
        bool rl = false;
        bool rr = false;
        bool luggage_door = false;
        bool egine_hood = false;
        bool sun_roof = false;
    };

    enum class ThemeType : int 
    {
        kUnknown = -1,
        kDark = 0,
        kLight = 1
    };

    enum class PageStatus : int
    {
        kNormal = 0,
        kFactoryMode,
        kAutoCalib,
        kManualCalib,
        kRoadCalib
    };

    enum class CalibProcessStatus : int
    {
        kInvalid,      
        kReadyForCalib,         // ready for calib
        kAutoCalibStart,        // start auto calib 
        kPrepareForManualCalib, // get corner points and prepare for maunual calib
        kManaulCalibStart,      // start manual calib
        kRoadCalibStart,        // start road calib
        kInterrupt,             // calib cancled by user
        kFinish,                // calib success
        kFailed                 // calib fail
    };

    struct Rect
    {
        float left_; 
        float top_; 
        float width_;
        float height_;
    };

}
}
#endif