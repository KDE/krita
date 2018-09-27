/*
 *  Copyright (c) 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_ARCS_CONSTANTS_H
#define KIS_ARCS_CONSTANTS_H

#include <QString>
#include <QColor>

static const int MIN_NUM_HUE_PIECES       = 1;
static const int MIN_NUM_UI_HUE_PIECES    = 2;
static const int MAX_NUM_HUE_PIECES       = 48;
static const int MIN_NUM_LIGHT_PIECES     = 1;
static const int MIN_NUM_UI_LIGHT_PIECES  = 2;
static const int MAX_NUM_LIGHT_PIECES     = 30;
static const int MIN_NUM_SATURATION_RINGS = 1;
static const int MAX_NUM_SATURATION_RINGS = 20;

static const int DEFAULT_HUE_STEPS = 12;
static const int DEFAULT_SATURATION_STEPS = 7;
static const int DEFAULT_VALUE_SCALE_STEPS = 11;

static const qreal DEFAULT_LUMA_R = 0.2126;
static const qreal DEFAULT_LUMA_G = 0.7152;
static const qreal DEFAULT_LUMA_B = 0.0722;
static const qreal DEFAULT_LUMA_GAMMA = 2.2;

// color scheme for the selector
static const QColor COLOR_MIDDLE_GRAY = QColor(128,128,128,255);
static const QColor COLOR_DARK = QColor(20,20,20,255);
static const QColor COLOR_LIGHT = QColor(232,232,232,255);
static const QColor COLOR_ACCENT = QColor(255,60,60,255);

static const QColor COLOR_MASK_FILL = COLOR_MIDDLE_GRAY;
static const QColor COLOR_MASK_OUTLINE = COLOR_LIGHT;
static const QColor COLOR_MASK_CLEAR = COLOR_LIGHT;
static const QColor COLOR_SELECTED = COLOR_ACCENT;
static const QColor COLOR_NORMAL_OUTLINE = COLOR_MIDDLE_GRAY;

#endif // KIS_ARCS_CONSTANTS_H
