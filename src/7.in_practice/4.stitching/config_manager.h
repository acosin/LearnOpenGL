/**
 * config_manager.h
 *
 * ConfigManager
 *
 * Created by jfzheng on 11/1/2020.
 * Copyright 2020 Telenav Inc. All rights reserved.
 *
 */
#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H
#include <string>
#include <vector>
#include <mutex>
#include <map>
#include <atomic>
#include <boost/property_tree/ptree.hpp>

#include "stitching_common.h"
// #include "tn/stitching_common/config_parser.h"
#include "avm_type.h"

namespace tn
{
namespace stitching
{

class ConfigManager
{
public:
    enum class OffScreenSurfaceType {
        kPbuffer = 0,
        kPixmap
    };
    enum class ResultBufferType {
        kPbo = 0,
        kDmabuf,
        kNone
    };
    static ConfigManager* Instance() {
        if (nullptr == instance_)
            instance_ = new ConfigManager();
        return instance_;
    }

    void Save();
    void Load(const std::string& path);
    int CanvasWidth();
    int CanvasHeight();
    int OutputImageWidth(OutputType type);
    int OutputImageHeight(OutputType type);
    ImageFormat OutputImageFormat(OutputType type);
    int InputImageWidth();
    int InputImageHeight();
    ImageFormat InputImageFormat();
    std::string TestWindowTitle();
    std::string ShaderPath();
    std::string ImagePath();
    void SwitchCamera(int key);
    bool RenderEnabled(Position pos);
    bool RenderEnabled(OutputType type);
    void SetRenderEnabled(OutputType type, bool enabled);
    LutParam GetLutParam(RenderType type, Position pos);
    CameraRenderParam GetRenderParam(RenderType type, Position pos);

    bool OptimizeTmqMemory();
    bool ComposedCanvas();
    int FramesBetweenFramebufferSwap();
    int InitialPixelBufferCount();
    bool TestWindowEnabled();
    bool PublishNearRaw();
    bool BlindAreaEnabled(OutputType type);
    bool CheckLocationJump();
    bool EglimageInput();
    bool NeedCreateInputDMAFD();
    bool DynamicCreateEGLImg();
    bool OffScreenRender();
    int MaxInputCacheSize();
    OffScreenSurfaceType OffScreenSurface();
    bool OneFrameLagForBetterFps();
    int MapbufferWaitMs();
    ResultBufferType RenderResultBufferType();
    std::string Tda4DisplayDevice();
    std::string Tda4DmaDevice();
    bool DebugOutput();
    bool DebugOpengl();
    bool RenderTrajectory();
    bool RenderUss();
    uint64_t MaxCameraDataAvailableLagInMs();
    bool IsApa();
    void SetIsApa(bool val);
    bool RenderThirdpartyAvm();
    bool UseExternalDMABufForThirdpartyAvm();
    float ThirdpartyAvmFPS();
    int ThirdpartyAvmWidth();
    int ThirdpartyAvmHeight();
    bool YFlipThirdpartyAvmOutput();
    ImageFormat ThridpartyAVMOutputFormat();
    bool ThirdAVMUseAPAFPSWhenInAPAMode() const;
    bool ThirdAVMSupportAPAFuction() const;
    uint64_t ThirdAVMHideDelayInMs() const;
    uint64_t Avs1DelayInMs() const;
    bool ApaStitchingControlEnabled();
    std::string GetDefautFont() const;
    std::string GetAvmCfgPath();
    std::string GetLayoutCfg();
    float GetOriginPpm();
    float GetSmallOriginPpm();
    float GetAPAPpm();
    float GetSmallAPAPpm();
    float GetCustomPpm(); 
    bool ShowAVMWhenInit() const;
    bool AlwaysAPAMode() const;
    bool GenerateSurroundThumnails() const;
    bool AlwaysOutputThumbnails() const;
    float SurroundThumnailsFPS() const;
    bool EnableFrontView() const;
    float CustomPpm();
    float OriginPpm();
    float ExitAvmTimer();
    bool OnlyThirdpartyAVMRender() const;
    bool IsShowSmallScreen();
    bool ShouldPrintRenderCost() const;
    bool ShouldPrintThirdAVMRenderCost() const;
    uint64_t PerfLogIntervalMS() const;
    bool EnableCalib() const;
    bool EnableObjPrompt() const;
    bool RenderBlackWhenHide() const;
    float FrontCarLenInCM() const;
    float RearCarLenInCM() const;
    float HalCarWidthInCM() const;
    float ObjPromtInCM() const;
    uint64_t ObjHoldInMS() const;
    uint64_t DelayAfterPlatformInitInMs() const;
    float OverSpeedLimit() const;
    float EnterSpeedLimit() const;
#ifdef SUPPORT_CALIB    
    uint64_t GetCalibTimeoutMs() const;
    std::string GetCalibDefaultYaml() const;
    std::string GetCalibBackupYaml() const;
    std::string GetCalibOutputYaml() const;
    std::string GetCalibInputConfig() const;
    std::string GetOnlineCalibInputConfig() const;
    std::string GetIntrinsicInputConfig() const;
    Rect GetCalibFloatingRect() const;
    int GetCalibImageFps() const;
    float GetCalibGetimageSpeedLimit() const;
#endif

TN_PRIVATE
    ConfigManager();

    static std::mutex m_;
    static ConfigManager* instance_;
    std::map<RenderType, std::map<Position, LutParam>> lut_param_;
    std::map<RenderType, std::map<Position, CameraRenderParam>> camera_render_param_;


    std::map<Position, bool> render_pos_enabled_flags_;
    std::map<OutputType, bool> render_type_enabled_flags_;
    std::map<OutputType, int> output_image_width_;
    std::map<OutputType, int> output_image_height_;
    std::map<OutputType, ImageFormat> output_image_format_;
    std::string resource_path_;
    std::string lut_parent_path_;
    std::string test_window_title_;
    std::string channel_info_path_;
    std::string tda4_display_device_;
    std::string tda4_dma_device_;
    uint64_t max_camera_data_available_lags_ms_ = 0;
    int input_image_width_;
    int input_image_height_;
    int canvas_width_;
    int canvas_height_;
    int frames_between_framebuffer_swap_;
    int initial_pixel_buffer_count_;
    int max_input_cache_size_;
    int mapbuffer_wait_ms_;
    OffScreenSurfaceType off_screen_surface_type_;
    ResultBufferType result_buffer_type_;
    ImageFormat input_image_format_;
    bool check_location_jump_ = false;
    bool optimize_tmq_memory_;
    bool composed_canvas_;
    bool test_window_enabled_;
    bool publish_near_raw_;
    bool one_frame_lag_for_better_fps_;
    bool debug_output_;
    bool debug_opengl_ = false;
    bool render_trajectory_;
    bool render_uss_;
    bool eglimage_input_;
    bool need_create_input_dma_fd_ = false;
    bool dynamic_create_egl_image_ = false;
    bool off_screen_render_;
    std::atomic<bool> is_apa_;
    bool render_thirdparty_avm_ = false;
    bool use_external_dma_buf_for_3rd_avm_ = false;
    float thirdparty_avm_fps_ = 15.0;
    int thirdparty_avm_width_ = 0;
    int thirdparty_avm_height_ = 0;
    bool y_flip_thirdparty_avm_output_ = true;
    bool control_apa_stitching_ = false;
    bool always_apa_mode_ = false;
    bool third_avm_using_apa_fps_when_apa_mode_ = false;
    std::string default_font_;
    std::string avm_config_path_;
    std::string layout_cfg_; 
    ImageFormat thirdparty_avm_output_format_ = ImageFormat::kNv12; 
    bool show_avm_when_init_ = false;
    bool generate_surround_thumbnails_ = false;
    bool always_output_thumbnails_ = false;
    float surround_thumbnails_fps_ = 5.0;
    bool enable_front_view_ = false;
    float custom_ppm_ = 0;
    float origin_ppm_ = 0;
    float small_origin_ppm_ = 0;
    float apa_ppm_ = 0;
    float small_apa_ppm_ = 0;
    float exit_avm_timer_ = 0;
    float front_car_len_cm_ = 415.2;
    float rear_car_len_cm_ = 120.1;
    float half_car_width_cm_ = 99.9;
    float obj_prompt_cm_ = 400.0;
    uint64_t obj_hold_time_in_ms_ = 3000;
    bool only_thirdparty_avm_ = false;
    bool is_show_small_screen_ = false;
    bool should_print_render_cost_ = false;
    bool should_print_render_3rd_avm_cost_ = false;
    bool third_avm_support_apa_func_ = true;
    uint64_t thirdparty_avm_hide_delay_in_ms_ = 0;
    uint64_t avs_1_delay_in_ms_ = 0;
    uint64_t perf_log_interval_ms_ = 3000;
    uint64_t delay_after_platform_init_in_ms_ = 0;
    float enter_speed_limit_ = 15.0;
    float over_speed_limit_ = 20.0;
    bool enable_calib_ = false;
    bool enable_obj_prompt_ = true;
    bool render_black_when_hide_ = false;
#ifdef SUPPORT_CALIB  
    std::string calib_default_yaml_;
    std::string calib_output_yaml_;
    std::string calib_backup_yaml_;
    std::string calib_online_input_config_;
    std::string calib_input_config_;
    std::string intrinsic_input_config_;
    Rect calib_floating_rect_;
    uint64_t calib_timeout_ms_{5000};
    int set_calib_image_fps_{10};
    float calib_getimage_speed_limit_{20.0};
#endif
};


} // namespace stitching {
} // namespace tn {

#endif // #ifndef CONFIG_MANAGER_H