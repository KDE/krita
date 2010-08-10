/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOPACANVASBASE_H
#define KOPACANVASBASE_H

#include <QWidget>
#include <QList>
#include <KoCanvasBase.h>

#include "kopageapp_export.h"

class KoPAViewBase;
class KoPADocument;

/// Widget that shows a KoPAPage
class KOPAGEAPP_EXPORT KoPACanvasBase : public KoCanvasBase
{

public:
    explicit KoPACanvasBase( KoPADocument * doc );
    virtual ~KoPACanvasBase();

    /// set the viewbase on the canvas; this needs to be called before the canvas can be used.
    void setView(KoPAViewBase *view);

    /// Update the canvas
    virtual void repaint() = 0;

    /// Returns pointer to the KoPADocument
    KoPADocument* document() const;

    /// reimplemented method
    virtual void gridSize( qreal *horizontal, qreal *vertical ) const;
    /// reimplemented method
    virtual bool snapToGrid() const;
    /// reimplemented method
    virtual void addCommand( QUndoCommand *command );
    /// reimplemented method
    virtual KoShapeManager * shapeManager() const;
    KoShapeManager * masterShapeManager() const;
    /// reimplemented from KoCanvasBase
    virtual KoGuidesData * guidesData();

    KoToolProxy * toolProxy() const;
    const KoViewConverter *viewConverter() const;
    KoUnit unit() const;

    /// XXX
    void setDocumentOffset(const QPoint &offset);

    /// XXX
    const QPoint & documentOffset() const;

    /// reimplemented in view coordinates
    virtual QPoint documentOrigin() const;
    /// Set the origin of the page inside the canvas in document coordinates
    void setDocumentOrigin(const QPointF & origin);

    KoPAViewBase* koPAView () const;

    /// translate widget coordinates to view coordinates
    QPoint widgetToView(const QPoint& p) const;
    QRect widgetToView(const QRect& r) const;
    QPoint viewToWidget(const QPoint& p) const;
    QRect viewToWidget(const QRect& r) const;

    /// Recalculates the size of the canvas (needed when zooming or changing pagelayout)
    virtual void updateSize() = 0;


protected:

    void paint(QPainter &painter, const QRectF paintRect);

private:
    class Private;
    Private * const d;
};

#endif /* KOPACANVASBASE_H */
