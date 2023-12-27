/*
 * SPDX-FileCopyrightText: 2022 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGCONFIGSELECTORTYPES_H
#define WGCONFIGSELECTORTYPES_H

#include <KisVisualColorModel.h>
#include <KisVisualColorSelector.h>

#include "WGConfig.h"

namespace WGConfig {

extern const NumericSetting<KisVisualColorModel::ColorModel> rgbColorModel;
extern const NumericSetting<KisVisualColorSelector::RenderMode> selectorRenderMode;

}

#endif // WGCONFIGSELECTORTYPES_H
