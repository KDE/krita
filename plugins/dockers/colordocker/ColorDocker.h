/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002 - 2005, Rob Buis <buis@kde.org>
   Copyright (C) 2006 Jan Hambecht <jaham@gmx.net>
   Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>

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

#ifndef COLOR_DOCKER_H
#define COLOR_DOCKER_H

#include <QDockWidget>

#include "KoDockFactoryBase.h"

class KoUniColorChooser;

/**
   Dock widget that contains a color selector. Currently this is the
   big, unified color selector, but I would prefer a small strip with
   just a band of colors -- and have the unified color selector popup.
   This just takes a tad too much place.
*/
class ColorDocker : public QDockWidget
{
    Q_OBJECT

public:
    ColorDocker(bool showOpacitySlider);
    virtual ~ColorDocker();

private:
    KoUniColorChooser *m_colorChooser;
};

#endif

