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
   Copyright (C) 2012 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>

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

#ifndef SNAPGUIDEDOCKER_H
#define SNAPGUIDEDOCKER_H

#include <KoCanvasObserverBase.h>
#include <QDockWidget>

/// A docker for setting properties of a snapping
class SnapGuideDocker : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT

public:
    /// Creates the stroke docker
    SnapGuideDocker();
    virtual ~SnapGuideDocker();

private slots:
    void locationChanged(Qt::DockWidgetArea area);

private:
    /// reimplemented
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();

private:
    class Private;
    Private * const d;
};

#endif // SNAPGUIDEDOCKER_H

