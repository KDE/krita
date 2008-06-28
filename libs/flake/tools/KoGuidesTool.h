/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOGUIDESTOOL_H
#define KOGUIDESTOOL_H

#include <KoTool.h>
#include "flake_export.h"

#include <QString>

class KoCanvasBase;

#define KoGuidesTool_ID "GuidesTool"

class FLAKE_EXPORT KoGuidesTool : public KoTool
{
public:
    explicit KoGuidesTool( KoCanvasBase * canvas );
    virtual ~KoGuidesTool();
    /// reimplemented form KoTool
    virtual void paint( QPainter &painter, const KoViewConverter &converter );
    /// reimplemented form KoTool
    virtual void mousePressEvent( KoPointerEvent *event );
    /// reimplemented form KoTool
    virtual void mouseMoveEvent( KoPointerEvent *event );
    /// reimplemented form KoTool
    virtual void mouseReleaseEvent( KoPointerEvent *event );
    /// reimplemented form KoTool
    virtual void repaintDecorations();
    /// reimplemented form KoTool
    virtual void activate(bool temporary = false);
    /// reimplemented form KoTool
    virtual void deactivate();

    /// Sets a new guide line to be added
    void setNewGuideLine( Qt::Orientation orientation, qreal position );

    /// Sets an existing guide line to be edited
    void setEditGuideLine( Qt::Orientation orientation, uint index );
private:
    class Private;
    Private * const d;
};

#endif // KOGUIDESTOOL_H
