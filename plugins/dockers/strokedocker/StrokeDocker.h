/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2002 Rob Buis <buis@kde.org>
   Copyright (C) 2004 Laurent Montel <montel@kde.org>
   Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2005 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>
   Copyright (C) 2005-2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 C. Boemann <cbo@boemann.dk>
   Copyright (C) 2011 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>

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

#ifndef STROKEDOCKER_H
#define STROKEDOCKER_H

#include <KoUnit.h>
#include <KoCanvasObserverBase.h>
#include <QDockWidget>
#include <KoMarkerData.h>

class KoShapeStrokeModel;

/// A docker for setting properties of a stroke
class StrokeDocker : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT

public:
    /// Creates the stroke docker
    StrokeDocker();
    virtual ~StrokeDocker();

public slots:
    /// Sets the stroke to edit the properties of
    virtual void setStroke( const KoShapeStrokeModel * );
    /// Sets the unit to be used for the line width editing
    virtual void setUnit( KoUnit unit );

private slots:
    /// line style has changed
    void styleChanged();
    /// line width has changed
    void widthChanged();
    /// line cap has changed
    void slotCapChanged( int ID );
    /// line join has changed
    void slotJoinChanged( int ID );
    /// miter limit has changed
    void miterLimitChanged();
    /// start marker has changed
    void startMarkerChanged();
    /// end marker has changed
    void endMarkerChanged();

    void resourceChanged(int key, const QVariant & value);
    void locationChanged(Qt::DockWidgetArea area);

    /// selection has changed
    void selectionChanged();

private:
    /// apply line changes to the selected shape
    void applyChanges();

    /// apply marker changes to the selected shape
    void applyMarkerChanges(KoMarkerData::MarkerPosition position);

    /// reimplemented
    virtual void setCanvas( KoCanvasBase *canvas );
    virtual void unsetCanvas();

private:
    class Private;
    Private * const d;
};

#endif // STROKEDOCKER_H

