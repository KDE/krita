/*
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>, (C)
 * Copyright (C) 2015 Michael Abrahams <miabraha@gmail.com>
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
#ifndef _KIS_ABSTRACT_CANVAS_WIDGET_
#define _KIS_ABSTRACT_CANVAS_WIDGET_

class QWidget;
class QRect;
class QPainter;
class QRect;

class KoToolProxy;

#include <kis_canvas_decoration.h>

class KisDisplayFilter;
class KisDisplayColorConverter;
class QBitArray;
class KoColorSpace;

#include "kis_types.h"
#include "kis_ui_types.h"

class KisAbstractCanvasWidget
{

public:

    KisAbstractCanvasWidget() {}

    virtual ~KisAbstractCanvasWidget() {}

    virtual QWidget * widget() = 0;

    virtual KoToolProxy * toolProxy() const = 0;

    /// Draw the specified decorations on the view.
    virtual void drawDecorations(QPainter & gc, const QRect &updateWidgetRect) const = 0;

    virtual void addDecoration(KisCanvasDecorationSP deco) = 0;
    virtual void removeDecoration(const QString& id) = 0;

    virtual KisCanvasDecorationSP decoration(const QString& id) const = 0;

    virtual void setDecorations(const QList<KisCanvasDecorationSP> &) = 0;

    virtual QList<KisCanvasDecorationSP> decorations() const = 0;

    /// set the specified display filter on the canvas
    virtual void setDisplayFilter(QSharedPointer<KisDisplayFilter> displayFilter) = 0;

    /// set/update the color space of the attached image
    virtual void notifyImageColorSpaceChanged(const KoColorSpace *cs) = 0;

    virtual void setWrapAroundViewingMode(bool value) = 0;

    // Called from KisCanvas2::channelSelectionChanged
    virtual void channelSelectionChanged(const QBitArray &channelFlags) = 0;

    // Called from KisCanvas2::slotSetDisplayProfile
    virtual void setDisplayColorConverter(KisDisplayColorConverter *colorConverter) = 0;

    // Called from KisCanvas2::finishResizingImage
    virtual void finishResizingImage(qint32 w, qint32 h) = 0;

    // Called from KisCanvas2::startUpdateProjection
    virtual KisUpdateInfoSP startUpdateCanvasProjection(const QRect & rc, const QBitArray &channelFlags) = 0;

    // Called from KisCanvas2::updateCanvasProjection
    virtual QRect updateCanvasProjection(KisUpdateInfoSP info) = 0;
    virtual QVector<QRect> updateCanvasProjection(const QVector<KisUpdateInfoSP> &infoObjects) = 0;

    /**
     * Returns true if the asynchronous engine of the canvas
     * (e.g. openGL pipeline) is busy with processing of the previous
     * update events. This will make KisCanvas2 to postpone and
     * compress update events.
     */
    virtual bool isBusy() const = 0;

    virtual void setLodResetInProgress(bool value) = 0;
};

#endif // _KIS_ABSTRACT_CANVAS_WIDGET_
