/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file host.h
 *
 *  Copyright 2017 Sébastien Fourey
 *
 *  This file is part of "gmic_qt", a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  gmic_qt is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gmic_qt is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gmic_qt.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _GMIC_QT_HOST_H_
#define _GMIC_QT_HOST_H_
#include "Common.h"
#include "gmic_qt.h"
#include <QString>

namespace cimg_library {
template<typename T> struct CImg;
template<typename T> struct CImgList;
}

namespace GmicQt {
extern const QString HostApplicationName;
extern const char * HostApplicationShortname;
}

/**
 * @brief gmic_qt_get_image_size
 *
 * Already deprecated ! gmic_qt_get_layers_extends is the one actually used.
 *
 * @param[out] width
 * @param[out] height
 */
void gmic_qt_get_image_size(int * width, int * height);

/**
 * @brief Get the largest width and largest height among all the layers
 *        according to the input mode (\see gmic_qt.h).
 *
 * @param[out] width
 * @param[out] height
 */
void gmic_qt_get_layers_extends(int * width, int * height, GmicQt::InputMode );

/**
 * @brief Get a list of (cropped) image layers from host software.
 *
 * @param[out] images list
 * @param[out] imageNames Per layer description strings (position, opacity, etc.)
 * @param x Top-left corner normalized x coordinate w.r.t. image/extends width (i.e., in [0,1])
 * @param y Top-left corner normalized y coordinate w.r.t. image/extends width (i.e., in [0,1])
 * @param width Normalized width of the layers w.r.t. image/extends width
 * @param height Normalized height of the layers w.r.t. image/extends height
 * @param mode Input mode
 */
void gmic_qt_get_cropped_images( cimg_library::CImgList<gmic_pixel_type> & images,
                                 cimg_library::CImgList<char> & imageNames,
                                 double x,
                                 double y,
                                 double width,
                                 double height,
                                 GmicQt::InputMode mode);

/**
 * @brief Send a list of new image layers to the host application according to
 *        an output mode (\see gmic_qt.cpp)
 *
 * @param images List of layers to be sent to the host application. May be modified.
 * @param imageNames Layers labels
 * @param mode Output mode (\see gmic_qt.cpp)
 * @param verboseLayersLabel Name used for all layers in VerboseLayerName mode, otherwise null.
 */
void gmic_qt_output_images(cimg_library::CImgList<gmic_pixel_type> & images,
                           const cimg_library::CImgList<char> & imageNames,
                           GmicQt::OutputMode mode,
                           const char * verboseLayersLabel = nullptr);


#endif // _GMIC_QT_HOST_H_
