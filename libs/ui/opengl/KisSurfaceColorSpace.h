/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSURFACECOLORSPACE_H
#define KISSURFACECOLORSPACE_H

#include <QSurfaceFormat>

/**
 * This file is a simple workaround for building our surface format
 * selection code on Qt older than 5.10, which doesn't have this feature
 *
 * TODO: remove this file when we drop support for Qt 5.9 LTS
 */

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)

using KisSurfaceColorSpace = QSurfaceFormat::ColorSpace;

#else

enum KisSurfaceColorSpace {
    DefaultColorSpace,
    sRGBColorSpace
};

#endif

#endif // KISSURFACECOLORSPACE_H
