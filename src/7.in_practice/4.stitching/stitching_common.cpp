/**
 * stitching_common.h
 *
 * 
 *
 * Created by jfzheng on 11/1/2020.
 * Copyright 2020 Telenav Inc. All rights reserved.
 *
 */

#include "stitching_common.h"



#ifndef debug_camera_renderer
#include "GLES2/gl2.h"
#include "tn_logger.h"
#else
#include <iostream>

#include <glad/glad.h>
#include "util.h"
#endif

#include "EGL/egl.h"


namespace tn
{
namespace stitching
{

void ShowApiInfo()
{
  const GLubyte * version =CheckGLError(glGetString(GL_VERSION));
  const GLubyte * shading_language_version =CheckGLError(glGetString(GL_SHADING_LANGUAGE_VERSION));
  const GLubyte * vendor =CheckGLError(glGetString(GL_VENDOR));
  const GLubyte * render =CheckGLError(glGetString(GL_RENDERER));
  const GLubyte * extensions =CheckGLError(glGetString(GL_EXTENSIONS));
  TN_LOG(TN_LOG_INFO) << "=====================BEGIN ShowApiInfo=====================\n";
  TN_LOG(TN_LOG_INFO) << "gl version:" << version;
  TN_LOG(TN_LOG_INFO) << "shading language version:" << shading_language_version;
  TN_LOG(TN_LOG_INFO) << "vendor :" <<  vendor;
  TN_LOG(TN_LOG_INFO) << "render :" << render;
  TN_LOG(TN_LOG_INFO) << "render :" << render;

#if 0
    GLint n, i;
    CheckGLError(glGetIntegerv(GL_NUM_EXTENSIONS, &n));

    if(extensions && n > 0)
        TN_LOG(TN_LOG_INFO) << "extensions :" << extensions;

    // for (i = 0; i < n; i++) {
    //     auto ext = CheckGLError(glGetStringi(GL_EXTENSIONS, i));
    //     TN_LOG(TN_LOG_INFO) << "<" << i << ">" << ext;
    // }
#endif
  TN_LOG(TN_LOG_INFO) <<  "=====================END ShowApiInfo=====================\n";

}

} // namespace stitching {
} // namespace tn {
