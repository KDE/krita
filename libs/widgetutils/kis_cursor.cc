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
    return load("cursor-cross.xpm");
}

QCursor KisCursor::roundCursor()
{
    return load("cursor-round.xpm");
}

QCursor KisCursor::pixelBlackCursor()
{
    return load("cursor-pixel-black.xpm");
}

QCursor KisCursor::pixelWhiteCursor()
{
    return load("cursor-pixel-white.xpm");
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

QCursor KisCursor::samplerPlusCursor()
{
    return KisCursorCache::instance()->samplerPlusCursor;
}

QCursor KisCursor::samplerMinusCursor()
{
    return KisCursorCache::instance()->samplerMinusCursor;
}

QCursor KisCursor::pickLayerCursor()
{
    return load("precise-pick-layer-icon.xpm", 7, 23);
}

QCursor KisCursor::penCursor()
{
    return KisCursorCache::instance()->penCursor;
}

QCursor KisCursor::brushCursor()
{
    return KisCursorCache::instance()->brushCursor;
}

QCursor KisCursor::airbrushCursor()
{
    return KisCursorCache::instance()->airbrushCursor;
}

QCursor KisCursor::eraserCursor()
{
    return KisCursorCache::instance()->eraserCursor;
}

QCursor KisCursor::fillerCursor()
{
    return KisCursorCache::instance()->fillerCursor;
}

QCursor KisCursor::colorChangerCursor()
{
    return KisCursorCache::instance()->colorChangerCursor;
}

QCursor KisCursor::zoomSmoothCursor()
{
    return load("zoom_smooth.xpm");
}

QCursor KisCursor::zoomDiscreteCursor()
{
    return load("zoom_discrete.xpm");
}

QCursor KisCursor::rotateCanvasSmoothCursor()
{
    return load("rotate_smooth.xpm");
}

QCursor KisCursor::rotateCanvasDiscreteCursor()
{
    return load("rotate_discrete.xpm");
}

QCursor KisCursor::samplerImageForegroundCursor()
{
    return load("color-sampler_image_foreground.xpm", 8, 23);
}

QCursor KisCursor::samplerImageBackgroundCursor()
{
    return load("color-sampler_image_background.xpm", 8, 23);
}

QCursor KisCursor::samplerLayerForegroundCursor()
{
    return load("color-sampler_layer_foreground.xpm", 8, 23);
}

QCursor KisCursor::samplerLayerBackgroundCursor()
{
    return load("color-sampler_layer_background.xpm", 8, 23);
}

QCursor KisCursor::changeExposureCursor()
{
    return load("exposure-cursor-gesture.xpm");
}

QCursor KisCursor::changeGammaCursor()
{
    return load("gamma-cursor-gesture.xpm");
}

QCursor KisCursor::triangleLeftHandedCursor()
{
    return load("cursor-triangle_lefthanded.xpm");
}

QCursor KisCursor::triangleRightHandedCursor()
{
    return load("cursor-triangle_righthanded.xpm");
}

QCursor KisCursor::moveCursor()
{
    return load("move-tool.png");
}

QCursor KisCursor::moveSelectionCursor()
{
    return load("move-selection.png");
}

QCursor KisCursor::handCursor()
{
    return Qt::PointingHandCursor;
}

QCursor KisCursor::selectCursor()
{
    return KisCursorCache::instance()->selectCursor;
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
    return load("rotate_cursor.xpm");
}

QCursor KisCursor::meshCursorFree()
{
    return load("mesh_cursor_free.png", 5, 5);
}

QCursor KisCursor::meshCursorLocked()
{
    return load("mesh_cursor_locked.png", 5, 5);
}

QCursor KisCursor::load(const QString & cursorName, int hotspotX, int hotspotY)
{
    return KisCursorCache::instance()->load(cursorName, hotspotX, hotspotY);
}
