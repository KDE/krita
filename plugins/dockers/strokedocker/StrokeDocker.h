/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2002 Rob Buis <buis@kde.org>
   Copyright (C) 2004 Laurent Montel <montel@kde.org>
   Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2005 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>
   Copyright (C) 2005-2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Casper Boemann <cbr@boemann.dk>

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
#include <QtGui/QDockWidget>

class KoShapeBorderModel;

/// A docker for setting properties of a line border
class StrokeDocker : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT

public:
    /// Creates the stroke docker
    StrokeDocker();
    virtual ~StrokeDocker();

public slots:
    /// Sets the border to edit the properties of
    virtual void setStroke( const KoShapeBorderModel * );
    /// Sets the unit to be used for the line width editing
    virtual void setUnit( KoUnit unit );

private slots:
    /// line cap has changed
    void slotCapChanged( int ID );
    /// line join has changed
    void slotJoinChanged( int ID );
    /// apply line changes to the selected shape
    void applyChanges();
    /// update the controls setting the values from the border
    void updateControls();
    /// line width has changed
    void widthChanged();
    /// miter limit has changed
    void miterLimitChanged();
    /// line style has changed
    void styleChanged();
    /// blocks/unblocks child control signals
    void blockChildSignals( bool block );
    /// selection has changed
    void selectionChanged();

    /// reimplemented
    virtual void setCanvas( KoCanvasBase *canvas );

    void resourceChanged(int key, const QVariant & value);
    void locationChanged(Qt::DockWidgetArea area);

private:
    class Private;
    Private * const d;
};

#endif // STROKEDOCKER_H

