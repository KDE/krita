/* This file is part of the KDE project
 * Copyright 2008 (C) Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_KRA_TAGS
#define KIS_KRA_TAGS

#include <QString>
#include <KisResourceTypes.h>

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
const QString ICC_PROOFING_PATH = "/annotations/proofing/icc";
const QString LAYER_STYLES_PATH = "/annotations/layerstyles.asl";
const QString ASSISTANTS_PATH = "/assistants/";
const QString LAYER_PATH = "/layers/";
const QString PALETTE_PATH = "/palettes/";
const QString STORYBOARD_PATH = "/storyboard/";
const QString ANIMATION_METADATA_PATH = "/animation/";

const QString ADJUSTMENT_LAYER = "adjustmentlayer";
const QString CHANNEL_FLAGS = "channelflags";
const QString CHANNEL_LOCK_FLAGS = "channellockflags";
const QString CLONE_FROM = "clonefrom";
const QString CLONE_FROM_UUID = "clonefromuuid";
const QString CLONE_LAYER = "clonelayer";
const QString CLONE_TYPE = "clonetype";
const QString COLORSPACE_NAME = "colorspacename";
const QString COMPOSITE_OP = "compositeop";
const QString DESCRIPTION = "description";
const QString ONION_SKIN_ENABLED = "onionskin";
const QString VISIBLE_IN_TIMELINE = "intimeline";

const QString DOT_FILTERCONFIG = ".filterconfig";
const QString DOT_TRANSFORMCONFIG = ".transformconfig";
const QString DOT_ICC = ".icc";
const QString DOT_PIXEL_SELECTION = ".pixelselection";
const QString DOT_SHAPE_SELECTION = ".shapeselection";
const QString DOT_SHAPE_LAYER = ".shapelayer";
const QString DOT_COLORIZE_MASK = ".colorizemask";
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
const QString COLLAPSED = "collapsed";
const QString COLOR_LABEL = "colorlabel";
const QString PAINT_LAYER = "paintlayer";
const QString PROFILE = "profile";
const QString ROTATION = "rotation";
const QString SELECTION_MASK = "selectionmask";
const QString SHAPE_LAYER = "shapelayer";
const QString REFERENCE_IMAGES_LAYER = "referenceimages";
const QString FILE_LAYER = "filelayer";
const QString TRANSPARENCY_MASK = "transparencymask";
const QString COLORIZE_MASK = "colorizemask";
const QString COLORIZE_SHOW_COLORING = "show-coloring";
const QString COLORIZE_EDIT_KEYSTROKES = "edit-keystrokes";
const QString COLORIZE_KEYSTROKE = "keystroke";
const QString COLORIZE_KEYSTROKE_COLOR = "color";
const QString COLORIZE_KEYSTROKE_IS_TRANSPARENT = "is-transparent";
const QString COLORIZE_COLORING_DEVICE = "colorize-coloring";
const QString COLORIZE_KEYSTROKES_SECTION = "keystrokes";
const QString COLORIZE_USE_EDGE_DETECTION = "use-edge-detection";
const QString COLORIZE_EDGE_DETECTION_SIZE = "edge-detection-size";
const QString COLORIZE_FUZZY_RADIUS = "fuzzy-radius";
const QString COLORIZE_CLEANUP = "cleanup";
const QString COLORIZE_LIMIT_TO_DEVICE = "limit-to-device";
const QString TRANSFORM_MASK = "transformmask";
const QString UUID = "uuid";
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
const QString ACTIVE = "active";
const QString LAYER_STYLE_UUID = "layerstyle";
const QString PASS_THROUGH_MODE = "passthrough";
const QString KEYFRAME_FILE = "keyframes";
const QString PROOFINGPROFILENAME = "proofing-profile-name";
const QString PROOFINGMODEL = "proofing-model";
const QString PROOFINGDEPTH = "proofing-depth";
const QString PROOFINGINTENT = "proofing-intent";
const QString PROOFINGWARNINGCOLOR ="ProofingWarningColor";
const QString PROOFINGADAPTATIONSTATE = "proofing-adaptation-state";
const QString ICCPROOFINGPROFILE ="icc-proofing-profile";
const QString CANVASPROJECTIONCOLOR = "ProjectionBackgroundColor";
const QString COLORBYTEDATA = "ColorData";
const QString SIMPLECOLORDATA = "SimpleColorData"; // easier 8-bit color data that works well with XML
const QString GLOBALASSISTANTSCOLOR = "GlobalAssistantsColor";
const QString PALETTES = "Palettes"; // ResourceType::Palettes is lowercase, while the tag is uppercase
const QString MIRROR_AXIS = "MirrorAxis";
}



#endif
