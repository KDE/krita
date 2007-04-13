/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002 - 2005, Rob Buis <buis@kde.org>
   Copyright (C) 2006 Jan Hambecht <jaham@gmx.net>
   Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>

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

#ifndef __KO_COLOR_DOCKER_H__
#define __KO_COLOR_DOCKER_H__

#include <QDockWidget>

#include <KoDockFactory.h>
#include "kopainter_export.h"

class KoUniColorChooser;

/**
   Dock widget that contains a color selector. Currently this is the
   big, unified color selector, but I would prefer a small strip with
   just a band of colors -- and have the unified color selector popup.
   This just takes a tad too much place.
*/
class KOPAINTER_EXPORT KoColorDocker : public QDockWidget
{
    Q_OBJECT

public:
    KoColorDocker(bool showOpacitySlider);
    virtual ~KoColorDocker();

private:
    KoUniColorChooser *m_colorChooser;
};


class KOPAINTER_EXPORT KoColorDockerFactory : public KoDockFactory
{
public:
    KoColorDockerFactory(bool showOpacitySlider = false)
        : m_showOpacitySlider( showOpacitySlider )
        {}
    ~KoColorDockerFactory() {}

    QString id() const;
    Qt::DockWidgetArea defaultDockWidgetArea() const;
    QDockWidget * createDockWidget();
private:

    bool m_showOpacitySlider;
};

#endif

