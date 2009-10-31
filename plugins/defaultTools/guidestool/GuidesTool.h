/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009 Carlos Licea <carlos.licea@kdemail.net>
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

#ifndef GUIDESTOOL_H
#define GUIDESTOOL_H

#include <KoGuidesTool.h>

#include <QString>
#include <QPair>

class KoCanvasBase;
class GuidesTransaction;

class GuidesTool : public KoGuidesTool
{
    Q_OBJECT

public:
    explicit GuidesTool( KoCanvasBase * canvas );
    virtual ~GuidesTool();
    /// reimplemented form KoTool
    virtual void paint( QPainter &painter, const KoViewConverter &converter );
    /// reimplemented form KoTool
    virtual void mousePressEvent( KoPointerEvent *event );
    /// reimplemented form KoTool
    virtual void mouseMoveEvent( KoPointerEvent *event );
    /// reimplemented form KoTool
    virtual void mouseReleaseEvent( KoPointerEvent *event );
    /// reimplemented form KoTool
    virtual void mouseDoubleClickEvent( KoPointerEvent *event );
    /// reimplemented form KoTool
    virtual void repaintDecorations();
    /// reimplemented form KoTool
    virtual void activate(bool temporary = false);
    /// reimplemented form KoTool
    virtual void deactivate();
    /// reimplemented form KoTool
    virtual QMap< QString, QWidget* > createOptionWidgets();

    /// reimplemented from KoGuidesTool
    virtual void addGuideLine( Qt::Orientation orientation, qreal position );
    /// reimplemented from KoGuidesTool
    virtual void moveGuideLine( Qt::Orientation orientation, uint index );
    /// reimplemented from KoGuidesTool
    virtual void editGuideLine( Qt::Orientation orientation, uint index );

private slots:
    void updateGuidePosition( qreal position );
    void guideLineSelected( Qt::Orientation orientation, uint index );
    void guideLinesChanged( Qt::Orientation orientation );
    /// reimplemented from KoTool
    virtual void resourceChanged( int key, const QVariant & res );

    void insertorCreateGuidesSlot( GuidesTransaction* result );

private:
    typedef QPair<Qt::Orientation, int> GuideLine;
    GuideLine guideLineAtPosition( const QPointF &position );

    class Private;
    Private * const d;
};

#endif // GUIDESTOOL_H
