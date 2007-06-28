/*
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef ZOOMHANDLER_H
#define ZOOMHANDLER_H

//   #include <KoCanvasBase.h>
#include <KoViewConverter.h>
//   #include <KoShapeControllerBase.h>
//
//   #include <QDockWidget>
//   #include <QRectF>
//
//   class KoTool;
//   class KoShape;
//   class KoViewConverter;
//   class QUndoCommand;
//   class KoCanvasController;
//   class KoShapeManager;
//   class QKeyEvent;
//   class QPainter;

class ZoomHandler : public KoViewConverter {
    /// reimplemented from superclass
    virtual QPointF documentToView (const QPointF &documentPoint) const;
    /// reimplemented from superclass
    virtual QPointF viewToDocument (const QPointF &viewPoint) const;
    /// reimplemented from superclass
    virtual QRectF documentToView (const QRectF &documentRect) const;
    /// reimplemented from superclass
    virtual QRectF viewToDocument (const QRectF &viewRect) const;
    /// reimplemented from superclass
    virtual QSizeF documentToView (const QSizeF &documentSize) const;
    /// reimplemented from superclass
    virtual QSizeF viewToDocument (const QSizeF &viewSize) const;
    /// reimplemented from superclass
    virtual void zoom (double *zoomX, double *zoomY) const;
    /// reimplemented from superclass
    virtual double documentToViewX (double documentX) const;
    /// reimplemented from superclass
    virtual double documentToViewY (double documentY) const;
    /// reimplemented from superclass
    virtual double viewToDocumentX (double viewX) const;
    /// reimplemented from superclass
    virtual double viewToDocumentY (double viewY) const;
};

#endif
