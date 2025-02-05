/*
 *  kis_cursor.cc - part of KImageShop
 *
 *  SPDX-FileCopyrightText: 1999 Matthias Elter <elter@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2013 David Revoy <info@davidrevoy.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_cursor.h"


#include <QtGlobal>
#include <QCursor>

#include "kis_cursor_cache.h"

KisCursor::KisCursor() {}

/*
 * Predefined Qt cursors
 */
QCursor KisCursor::arrowCursor()
{
    return Qt::ArrowCursor;
}

QCursor KisCursor::upArrowCursor()
{
    return Qt::UpArrowCursor;
}

QCursor KisCursor::crossCursor()
{
    return loadWithSize("cursor-cross.svg", 31, 31);
}

QCursor KisCursor::roundCursor()
{
    return loadWithSize("cursor-round.svg", 31, 31);
}

QCursor KisCursor::pixelBlackCursor()
{
    return loadWithSize("cursor-pixel-black.xpm", 31, 31);
}

QCursor KisCursor::pixelWhiteCursor()
{
    return loadWithSize("cursor-pixel-white.xpm", 31, 31);
}

QCursor KisCursor::waitCursor()
{
    return Qt::WaitCursor;
}

QCursor KisCursor::ibeamCursor()
{
    return Qt::IBeamCursor;
}

QCursor KisCursor::sizeVerCursor()
{
    return Qt::SizeVerCursor;
}

QCursor KisCursor::sizeHorCursor()
{
    return Qt::SizeHorCursor;
}

QCursor KisCursor::sizeBDiagCursor()
{
    return Qt::SizeBDiagCursor;
}

QCursor KisCursor::sizeFDiagCursor()
{
    return Qt::SizeFDiagCursor;
}

QCursor KisCursor::sizeAllCursor()
{
    return Qt::SizeAllCursor;
}

QCursor KisCursor::blankCursor()
{
    return Qt::BlankCursor;
}

QCursor KisCursor::splitVCursor()
{
    return Qt::SplitVCursor;
}

QCursor KisCursor::splitHCursor()
{
    return Qt::SplitHCursor;
}

QCursor KisCursor::pointingHandCursor()
{
    return Qt::PointingHandCursor;
}


QCursor KisCursor::samplerCursor()
{
    return samplerLayerForegroundCursor();
}

QCursor KisCursor::pickLayerCursor()
{
    return loadWithSize("precise-pick-layer-icon.svg", 33, 31, 7, 23);
}

QCursor KisCursor::zoomSmoothCursor()
{
    return loadWithSize("zoom_smooth.svg", 33, 31);
}

QCursor KisCursor::zoomDiscreteCursor()
{
    return loadWithSize("zoom_discrete.svg", 33, 31);
}

QCursor KisCursor::rotateCanvasSmoothCursor()
{
    return loadWithSize("rotate_smooth.svg", 33, 31);
}

QCursor KisCursor::rotateCanvasDiscreteCursor()
{
    return loadWithSize("rotate_discrete.svg", 33, 31);
}

QCursor KisCursor::samplerImageForegroundCursor()
{
    return loadWithSize("color-sampler_image_foreground.svg", 33, 31, 8, 23);
}

QCursor KisCursor::samplerImageBackgroundCursor()
{
    return loadWithSize("color-sampler_image_background.svg", 33, 31,  8, 23);
}

QCursor KisCursor::samplerGroupForegroundCursor()
{
    return load("color-sampler_group_foreground.xpm", 8, 23);
}

QCursor KisCursor::samplerGroupBackgroundCursor()
{
    return load("color-sampler_group_background.xpm", 8, 23);
}

QCursor KisCursor::samplerLayerForegroundCursor()
{
    return loadWithSize("color-sampler_layer_foreground.svg", 33, 31, 8, 23);
}

QCursor KisCursor::samplerLayerBackgroundCursor()
{
    return loadWithSize("color-sampler_layer_background.svg", 33, 31, 8, 23);
}

QCursor KisCursor::changeExposureCursor()
{
    return loadWithSize("exposure-cursor-gesture.svg", 22, 22);
}

QCursor KisCursor::changeGammaCursor()
{
    return loadWithSize("gamma-cursor-gesture.svg", 32, 32);
}

QCursor KisCursor::triangleLeftHandedCursor()
{
    return loadWithSize("cursor-triangle_lefthanded.svg", 31, 31);
}

QCursor KisCursor::triangleRightHandedCursor()
{
    return loadWithSize("cursor-triangle_righthanded.svg", 31, 31);
}

QCursor KisCursor::eraserCursor()
{
    return loadWithSize("cursor-eraser.svg", 32, 32, 2, 2);
}

QCursor KisCursor::moveCursor()
{
    return loadWithSize("move-tool.svg", 24, 24);
}

QCursor KisCursor::moveSelectionCursor()
{
    return loadWithSize("move-selection.svg", 32, 32, 11, 11);
}

QCursor KisCursor::handCursor()
{
    return Qt::PointingHandCursor;
}

QCursor KisCursor::openHandCursor()
{
    return Qt::OpenHandCursor;
}

QCursor KisCursor::closedHandCursor()
{
    return Qt::ClosedHandCursor;
}

QCursor KisCursor::rotateCursor()
{
    return loadWithSize("rotate_cursor.svg", 22, 22);
}

QCursor KisCursor::meshCursorFree()
{
    return loadWithSize("mesh_cursor_free.svg", 32, 32, 5, 5);
}

QCursor KisCursor::meshCursorLocked()
{
    return loadWithSize("mesh_cursor_locked.svg", 32, 32, 5, 5);
}

QCursor KisCursor::load(const QString &cursorName, int hotspotX, int hotspotY)
{
    return KisCursorCache::instance()->load(cursorName, 32, 32, hotspotX, hotspotY);
}

QCursor KisCursor::loadWithSize(const QString & cursorName, int width, int height, int hotspotX, int hotspotY)
{
    return KisCursorCache::instance()->load(cursorName, width, height, hotspotX, hotspotY);
}
