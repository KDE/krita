/*
 *  kis_cursor.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2013 David Revoy <info@davidrevoy.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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


QCursor KisCursor::pickerCursor()
{
    return pickerLayerForegroundCursor();
}

QCursor KisCursor::pickerPlusCursor()
{
    return KisCursorCache::instance()->pickerPlusCursor;
}

QCursor KisCursor::pickerMinusCursor()
{
    return KisCursorCache::instance()->pickerMinusCursor;
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

QCursor KisCursor::pickerImageForegroundCursor()
{
    return load("color-picker_image_foreground.xpm", 8, 23);
}

QCursor KisCursor::pickerImageBackgroundCursor()
{
    return load("color-picker_image_background.xpm", 8, 23);
}

QCursor KisCursor::pickerLayerForegroundCursor()
{
    return load("color-picker_layer_foreground.xpm", 8, 23);
}

QCursor KisCursor::pickerLayerBackgroundCursor()
{
    return load("color-picker_layer_background.xpm", 8, 23);
}

QCursor KisCursor::changeExposureCursor()
{
    return load("exposure-cursor-gesture.xpm", 8, 23);
}

QCursor KisCursor::changeGammaCursor()
{
    return load("gamma-cursor-gesture.xpm", 8, 23);
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

QCursor KisCursor::load(const QString & cursorName, int hotspotX, int hotspotY)
{
    return KisCursorCache::instance()->load(cursorName, hotspotX, hotspotY);
}
