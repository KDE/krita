/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISTOOLSHAPEUTILS_H
#define KISTOOLSHAPEUTILS_H

namespace KisToolShapeUtils
{

enum FillStyle {
    FillStyleNone,
    FillStyleForegroundColor,
    FillStyleBackgroundColor,
    FillStylePattern,
};

enum StrokeStyle {
    StrokeStyleNone,
    StrokeStyleForeground,
    StrokeStyleBackground
};

}

#endif // KISTOOLSHAPEUTILS_H
