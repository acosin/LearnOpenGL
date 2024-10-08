#include "config_manager.h"
// #include "tn/stitching_common/config_parser.h"

#include <boost/property_tree/json_parser.hpp>
#include <fstream>
#include <iostream>

namespace tn
{
namespace stitching
{

std::mutex ConfigManager::m_;
ConfigManager* ConfigManager::instance_ = nullptr;

ConfigManager::ConfigManager():default_font_(""), avm_config_path_(""), layout_cfg_(""), origin_ppm_(0), custom_ppm_(0)
{
    resource_path_ = "./resource/stitching/";
    lut_parent_path_ = resource_path_; 
    test_window_title_ = "Stitching"; 
    canvas_width_ = 512;
    canvas_height_ = 512;
    input_image_width_ = 1280;
    input_image_height_ = 720;
    optimize_tmq_memory_ = false;
    composed_canvas_ = false;
    frames_between_framebuffer_swap_ = 1;
    initial_pixel_buffer_count_ = 0;
    test_window_enabled_ = false;
    publish_near_raw_ = false;
    max_input_cache_size_ = 1;
    off_screen_surface_type_ = OffScreenSurfaceType::kPbuffer;
    one_frame_lag_for_better_fps_ = false;
    mapbuffer_wait_ms_ = 5;
    debug_output_ = false;
    debug_opengl_ = false;
    render_trajectory_ = false;
    render_uss_ = false;
    is_apa_ = true;
    eglimage_input_ = false;
    off_screen_render_ = true;
    input_image_format_ = ImageFormat::kRgb;
    result_buffer_type_ = ResultBufferType::kNone;
#if defined(TN_PLATFORM_LINUX_TDA4_AARCH64)
    input_image_format_ = ImageFormat::kNv12;
    result_buffer_type_ = ResultBufferType::kDmabuf;
#elif defined(TN_PLATFORM_LINUX_TDA4_CHERYCV_AARCH64)
    input_image_format_ = ImageFormat::kNv12;
    result_buffer_type_ = ResultBufferType::kDmabuf;
#elif defined(TN_PLATFORM_LINUX_TDA4_BYD_AARCH64)
    input_image_format_ = ImageFormat::kUyvy;
    result_buffer_type_ = ResultBufferType::kDmabuf;
#endif
    tda4_display_device_ = "/dev/dri/by-path/platform-4a00000.dss-card";
    tda4_dma_device_ = "/dev/dma_heap/vision_apps_shared-memories";

    lut_param_[RenderType::kFarAvm][Position::kFront] = LutParam(lut_parent_path_ + "lut/far_front.bin", 512, 200, 1280, 720, 4);
    lut_param_[RenderType::kFarAvm][Position::kRear] = LutParam(lut_parent_path_ + "lut/far_rear.bin", 512, 200, 1280, 720, 4);
    lut_param_[RenderType::kFarAvm][Position::kLeft] = LutParam(lut_parent_path_ + "lut/far_left.bin", 236, 512, 1280, 720, 4);
    lut_param_[RenderType::kFarAvm][Position::kRight] = LutParam(lut_parent_path_ + "lut/far_right.bin", 234, 512, 1280, 720, 4);

    camera_render_param_[RenderType::kFarAvm][Position::kFront].grid_per_row = 512;
    camera_render_param_[RenderType::kFarAvm][Position::kFront].grid_per_column = 191;
    camera_render_param_[RenderType::kFarAvm][Position::kRear].grid_per_row = 512;
    camera_render_param_[RenderType::kFarAvm][Position::kRear].grid_per_column = 191;
    camera_render_param_[RenderType::kFarAvm][Position::kLeft].grid_per_row = 233;
    camera_render_param_[RenderType::kFarAvm][Position::kLeft].grid_per_column = 512;
    camera_render_param_[RenderType::kFarAvm][Position::kRight].grid_per_row = 233;
    camera_render_param_[RenderType::kFarAvm][Position::kRight].grid_per_column = 512;
    camera_render_param_[RenderType::kFarAvm][Position::kFrontLeft].grid_per_row = 233;
    camera_render_param_[RenderType::kFarAvm][Position::kFrontLeft].grid_per_column = 191;
    camera_render_param_[RenderType::kFarAvm][Position::kFrontRight].grid_per_row = 233;
    camera_render_param_[RenderType::kFarAvm][Position::kFrontRight].grid_per_column = 191;
    camera_render_param_[RenderType::kFarAvm][Position::kRearLeft].grid_per_row = 233;
    camera_render_param_[RenderType::kFarAvm][Position::kRearLeft].grid_per_column = 191;
    camera_render_param_[RenderType::kFarAvm][Position::kRearRight].grid_per_row = 233;
    camera_render_param_[RenderType::kFarAvm][Position::kRearRight].grid_per_column = 191;

    camera_render_param_[RenderType::kNearAvm][Position::kFront].grid_per_row = 436;
    camera_render_param_[RenderType::kNearAvm][Position::kFront].grid_per_column = 194;
    camera_render_param_[RenderType::kNearAvm][Position::kRear].grid_per_row = 436;
    camera_render_param_[RenderType::kNearAvm][Position::kRear].grid_per_column = 194;
    camera_render_param_[RenderType::kNearAvm][Position::kLeft].grid_per_row = 150;
    camera_render_param_[RenderType::kNearAvm][Position::kLeft].grid_per_column = 760;
    camera_render_param_[RenderType::kNearAvm][Position::kRight].grid_per_row = 150;
    camera_render_param_[RenderType::kNearAvm][Position::kRight].grid_per_column = 760;
    camera_render_param_[RenderType::kNearAvm][Position::kFrontLeft].grid_per_row = 150;
    camera_render_param_[RenderType::kNearAvm][Position::kFrontLeft].grid_per_column = 194;
    camera_render_param_[RenderType::kNearAvm][Position::kFrontRight].grid_per_row = 150;
    camera_render_param_[RenderType::kNearAvm][Position::kFrontRight].grid_per_column = 194;
    camera_render_param_[RenderType::kNearAvm][Position::kRearLeft].grid_per_row = 150;
    camera_render_param_[RenderType::kNearAvm][Position::kRearLeft].grid_per_column = 194;
    camera_render_param_[RenderType::kNearAvm][Position::kRearRight].grid_per_row = 150;
    camera_render_param_[RenderType::kNearAvm][Position::kRearRight].grid_per_column = 194;

    render_pos_enabled_flags_[Position::kFront] = true;
    render_pos_enabled_flags_[Position::kRear] = true;
    render_pos_enabled_flags_[Position::kLeft] = true;
    render_pos_enabled_flags_[Position::kRight] = true;
    render_pos_enabled_flags_[Position::kFrontLeft] = true;
    render_pos_enabled_flags_[Position::kFrontRight] = true;
    render_pos_enabled_flags_[Position::kRearLeft] = true;
    render_pos_enabled_flags_[Position::kRearRight] = true;

    render_type_enabled_flags_[OutputType::kStitchedFar] = true;
    render_type_enabled_flags_[OutputType::kStitchedMedium] = false;
    render_type_enabled_flags_[OutputType::kStitchedNear] = true;
    render_type_enabled_flags_[OutputType::kStitchedNearVPA] = true;
    render_type_enabled_flags_[OutputType::kStitchedCustom1] = true;
    render_type_enabled_flags_[OutputType::kPerspectiveAll] = true;
    render_type_enabled_flags_[OutputType::kPerspectiveFront] = true;
    render_type_enabled_flags_[OutputType::kPerspectiveRear] = true;
    render_type_enabled_flags_[OutputType::kPerspectiveLeft] = false;
    render_type_enabled_flags_[OutputType::kPerspectiveRight] = false;
    render_type_enabled_flags_[OutputType::kSurroundThumbnailAll] = false;
    render_type_enabled_flags_[OutputType::kSurroundThumbnailFront] = false;
    render_type_enabled_flags_[OutputType::kSurroundThumbnailRear] = false;
    render_type_enabled_flags_[OutputType::kSurroundThumbnailLeft] = false;
    render_type_enabled_flags_[OutputType::kSurroundThumbnailRight] = false;
}


void ConfigManager::Load(const std::string& path)
{

}

void ConfigManager::Save()
{

}

int ConfigManager::CanvasWidth()
{
    return canvas_width_;
}
    
int ConfigManager::CanvasHeight()
{
    return canvas_height_;
}

int ConfigManager::OutputImageWidth(OutputType type)
{
    return output_image_width_[type];
}

int ConfigManager::OutputImageHeight(OutputType type)
{
    return output_image_height_[type];
}

ImageFormat ConfigManager::OutputImageFormat(OutputType type)
{
    return output_image_format_[type];
}

std::string ConfigManager::TestWindowTitle()
{
    return test_window_title_;
}

int ConfigManager::InputImageWidth()
{
    return input_image_width_;
}

int ConfigManager::InputImageHeight()
{
    return input_image_height_;
}

LutParam ConfigManager::GetLutParam(RenderType type, Position pos)
{
    return lut_param_[type][pos];
}

CameraRenderParam ConfigManager::GetRenderParam(RenderType type, Position pos)
{
    return camera_render_param_[type][pos];
}

bool ConfigManager::OptimizeTmqMemory()
{
    return optimize_tmq_memory_;
}

bool ConfigManager::ComposedCanvas()
{
    return composed_canvas_;
}

int ConfigManager::FramesBetweenFramebufferSwap()
{
    return frames_between_framebuffer_swap_;
}

int ConfigManager::InitialPixelBufferCount()
{
    return initial_pixel_buffer_count_;
}

std::string ConfigManager::ShaderPath()
{
    return resource_path_ + "/shader/";
}

std::string ConfigManager::ImagePath()
{
    return resource_path_;
}

void ConfigManager::SwitchCamera(int key)
{
    Position p = (Position)key;
    render_pos_enabled_flags_[p] = !render_pos_enabled_flags_[p];
}

bool ConfigManager::RenderEnabled(Position pos)
{
    return render_pos_enabled_flags_[pos];
}

bool ConfigManager::RenderEnabled(OutputType type)
{
    if (!render_type_enabled_flags_.count(type))
        return false;
    return render_type_enabled_flags_[type];
}

void ConfigManager::SetRenderEnabled(OutputType type, bool enabled)
{
    render_type_enabled_flags_[type] = enabled;
}

bool ConfigManager::TestWindowEnabled()
{
    return test_window_enabled_;
}

bool ConfigManager::PublishNearRaw()
{
    return publish_near_raw_;
}

bool ConfigManager::BlindAreaEnabled(OutputType type)
{
    return false;
}

bool ConfigManager::CheckLocationJump()
{
    return check_location_jump_;
}

int ConfigManager::MaxInputCacheSize()
{
    return max_input_cache_size_;
}

ConfigManager::OffScreenSurfaceType ConfigManager::OffScreenSurface()
{
    return off_screen_surface_type_;
}

bool ConfigManager::OneFrameLagForBetterFps()
{
    return one_frame_lag_for_better_fps_;
}

int ConfigManager::MapbufferWaitMs()
{
    return mapbuffer_wait_ms_;
}

ConfigManager::ResultBufferType ConfigManager::RenderResultBufferType()
{
    return result_buffer_type_;
}

std::string ConfigManager::Tda4DisplayDevice()
{
    return tda4_display_device_;
}

std::string ConfigManager::Tda4DmaDevice()
{
    return tda4_dma_device_;
}

ImageFormat ConfigManager::InputImageFormat()
{
    return input_image_format_;
}

bool ConfigManager::DebugOutput()
{
    return debug_output_;
}

bool ConfigManager::DebugOpengl()
{
    return debug_opengl_;
}

bool ConfigManager::EglimageInput()
{
    return eglimage_input_;
}

bool ConfigManager::NeedCreateInputDMAFD()
{
    return need_create_input_dma_fd_;
}

bool ConfigManager::DynamicCreateEGLImg()
{
    return dynamic_create_egl_image_;
}

bool ConfigManager::OffScreenRender()
{
    return off_screen_render_;
}

bool ConfigManager::RenderTrajectory()
{
    return render_trajectory_;
}

bool ConfigManager::RenderUss()
{
    return render_uss_;
}

uint64_t ConfigManager::MaxCameraDataAvailableLagInMs()
{
    return max_camera_data_available_lags_ms_;
}

bool ConfigManager::IsApa()
{
    return is_apa_;
}

void ConfigManager::SetIsApa(bool val)
{
    is_apa_ = val;
}

bool ConfigManager::RenderThirdpartyAvm()
{
    return render_thirdparty_avm_;
}

bool ConfigManager::UseExternalDMABufForThirdpartyAvm()
{
    return use_external_dma_buf_for_3rd_avm_;
}

float ConfigManager::ThirdpartyAvmFPS()
{
    return thirdparty_avm_fps_;
}

int ConfigManager::ThirdpartyAvmWidth()
{
    return thirdparty_avm_width_;
}

int ConfigManager::ThirdpartyAvmHeight()
{
    return thirdparty_avm_height_;
}

bool ConfigManager::YFlipThirdpartyAvmOutput()
{
    return y_flip_thirdparty_avm_output_;
}

ImageFormat ConfigManager::ThridpartyAVMOutputFormat()
{
    return thirdparty_avm_output_format_;
}

bool ConfigManager::ThirdAVMUseAPAFPSWhenInAPAMode() const
{
    return third_avm_using_apa_fps_when_apa_mode_;
}

bool ConfigManager::ThirdAVMSupportAPAFuction() const
{
    return third_avm_support_apa_func_;
}

uint64_t ConfigManager::ThirdAVMHideDelayInMs() const
{
    return thirdparty_avm_hide_delay_in_ms_;
}

uint64_t ConfigManager::Avs1DelayInMs() const
{
    return avs_1_delay_in_ms_;
}

bool ConfigManager::ApaStitchingControlEnabled()
{
    return control_apa_stitching_;
}

bool ConfigManager::EnableFrontView() const
{
    return enable_front_view_;
}

bool ConfigManager::ShowAVMWhenInit() const
{
    return show_avm_when_init_;
}

bool ConfigManager::AlwaysAPAMode() const
{
    return always_apa_mode_;
}

bool ConfigManager::GenerateSurroundThumnails() const
{
    return generate_surround_thumbnails_;
}

float ConfigManager::SurroundThumnailsFPS() const
{
    return surround_thumbnails_fps_;
}

bool ConfigManager::AlwaysOutputThumbnails() const
{
    return always_output_thumbnails_;
}

std::string ConfigManager::GetDefautFont() const
{
    return default_font_;
}

std::string ConfigManager::GetAvmCfgPath()
{
    return avm_config_path_;
}

std::string ConfigManager::GetLayoutCfg()
{
    return layout_cfg_;
}

float ConfigManager::GetOriginPpm()
{
    return origin_ppm_;
}

float ConfigManager::GetSmallOriginPpm()
{
    return small_origin_ppm_;
}

float ConfigManager::GetAPAPpm()
{
    return apa_ppm_;
}

float ConfigManager::GetSmallAPAPpm()
{
    return small_apa_ppm_;
}

float ConfigManager::GetCustomPpm()
{
    return custom_ppm_;
}

float ConfigManager::ExitAvmTimer()
{
    return exit_avm_timer_;
}

bool ConfigManager::OnlyThirdpartyAVMRender() const
{
    return only_thirdparty_avm_;
}

bool ConfigManager::IsShowSmallScreen()
{
    return is_show_small_screen_;
}

bool ConfigManager::ShouldPrintRenderCost() const
{
    return should_print_render_cost_;
}

bool ConfigManager::ShouldPrintThirdAVMRenderCost() const
{
    return should_print_render_3rd_avm_cost_;
}

uint64_t ConfigManager::PerfLogIntervalMS() const
{
    return perf_log_interval_ms_;
}

bool ConfigManager::EnableCalib() const
{
    return enable_calib_;
}

bool ConfigManager::EnableObjPrompt() const
{
    return enable_obj_prompt_;
}

bool ConfigManager::RenderBlackWhenHide() const
{
    return render_black_when_hide_;
}

float ConfigManager::FrontCarLenInCM() const
{
    return front_car_len_cm_;
}

float ConfigManager::OverSpeedLimit() const
{
    return over_speed_limit_;
}

float ConfigManager::EnterSpeedLimit() const
{
    return enter_speed_limit_;
}

float ConfigManager::RearCarLenInCM() const
{
    return rear_car_len_cm_;
}

float ConfigManager::HalCarWidthInCM() const
{
    return half_car_width_cm_;
}

float ConfigManager::ObjPromtInCM() const
{
    return obj_prompt_cm_;
}

uint64_t ConfigManager::ObjHoldInMS() const
{
    return obj_hold_time_in_ms_;
}

uint64_t ConfigManager::DelayAfterPlatformInitInMs() const
{
    return delay_after_platform_init_in_ms_;
}

#ifdef SUPPORT_CALIB    
uint64_t ConfigManager::GetCalibTimeoutMs() const
{
    return calib_timeout_ms_;
}

float ConfigManager::GetCalibGetimageSpeedLimit() const
{
    return calib_getimage_speed_limit_;
}

std::string ConfigManager::GetCalibOutputYaml() const
{
    return calib_output_yaml_;
}

std::string ConfigManager::GetCalibDefaultYaml() const
{
    return calib_default_yaml_;
}
std::string ConfigManager::GetCalibBackupYaml() const
{
    return calib_backup_yaml_;
}

std::string ConfigManager::GetCalibInputConfig() const
{
    return calib_input_config_;
}

std::string ConfigManager::GetOnlineCalibInputConfig() const
{
    return calib_online_input_config_;
}

std::string ConfigManager::GetIntrinsicInputConfig() const
{
    return intrinsic_input_config_;
}

Rect ConfigManager::GetCalibFloatingRect() const
{
    return calib_floating_rect_;
}

int ConfigManager::GetCalibImageFps() const
{
    return set_calib_image_fps_;
}

#endif

} // namespace stitching {
} // namespace tn {