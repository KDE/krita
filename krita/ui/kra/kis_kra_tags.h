/* This file is part of the KDE project
 * Copyright 2008 (C) Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KIS_KRA_TAGS
#define KIS_KRA_TAGS

#include <QString>



/**
 * Tag definitions for our xml file format
 */
namespace KRA
{

// mimetype
const QString NATIVE_MIMETYPE = "application/x-kra";

// xml tags
const QString SEPARATOR = "/";
const QString SHAPE_LAYER_PATH = "/shapelayers/";
const QString EXIF_PATH = "/annotations/exif";
const QString ICC_PATH = "/annotations/icc";
const QString LAYER_PATH = "/layers/";

const QString ADJUSTMENT_LAYER = "adjustmentlayer";
const QString CHANNEL_FLAGS = "channelflags";
const QString CLONE_FROM = "clonefrom";
const QString CLONE_LAYER = "clonelayer";
const QString CLONE_TYPE = "clonetype";
const QString COLORSPACE_NAME = "colorspacename";
const QString COMPOSITE_OP = "compositeop";
const QString DESCRIPTION = "description";

const QString DOT_FILTERCONFIG = ".filterconfig";
const QString DOT_ICC = ".icc";
const QString DOT_PIXEL_SELECTION = ".pixelselection";
const QString DOT_SHAPE_SELECTION = ".shapeselection";
const QString DOT_SHAPE_LAYER = ".shapelayer";
const QString DOT_METADATA = ".metadata";

const QString FILE_NAME = "filename";
const QString FILTER_MASK = "filtermask";
const QString FILTER_NAME = "filtername";
const QString FILTER_STATEGY = "filter_strategy";
const QString FILTER_VERSION = "filterversion";
const QString GENERATOR_LAYER = "generatorlayer";
const QString GENERATOR_NAME = "generatorname";
const QString GENERATOR_VERSION = "generatorversion";
const QString GROUP_LAYER = "grouplayer";
const QString HEIGHT = "height";
const QString ICC = "icc";
const QString LAYER = "layer";
const QString LAYERS = "layers";
const QString NODE_TYPE = "nodetype";
const QString LOCKED = "locked";
const QString MASK = "mask";
const QString MASKS = "masks";
const QString MIME = "mime";
const QString NAME = "name";
const QString OPACITY = "opacity";
const QString PAINT_LAYER = "paintlayer";
const QString PROFILE = "profile";
const QString ROTATION = "rotation";
const QString SELECTION_MASK = "selectionmask";
const QString SHAPE_LAYER = "shapelayer";
const QString TRANSFORMATION_MASK = "transformationmask";
const QString TRANSPARENCY_MASK = "transparencymask";
const QString VISIBLE = "visible";
const QString WIDTH = "width";
const QString X = "x";
const QString X_RESOLUTION = "x-res";
const QString X_SCALE = "x_scale";
const QString X_SHEAR = "x_shear";
const QString X_TRANSLATION = "x_translation";
const QString Y = "y";
const QString Y_RESOLUTION = "y-res";
const QString Y_SCALE = "y_scale";
const QString Y_SHEAR = "y_shear";
const QString Y_TRANSLATION = "y_translation";
}



#endif
