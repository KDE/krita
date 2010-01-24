/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002 - 2005, Rob Buis <buis@kde.org>
   Copyright (C) 2006 Jan Hambecht <jaham@gmx.net>
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
   Boston, MA 02110-1301, USA.
*/
#include "ColorDocker.h"

#include <klocale.h>
#include <kdebug.h>

#include <KoUniColorChooser.h>

ColorDocker::ColorDocker(bool showOpacitySlider)
    : QDockWidget()
{
    setWindowTitle(i18n("Color Chooser"));

    m_colorChooser = new KoUniColorChooser(this, showOpacitySlider);
    m_colorChooser->changeLayout(KoUniColorChooser::SimpleLayout);
    setWidget(m_colorChooser);
    setMinimumWidth( 194 );
}

ColorDocker::~ColorDocker()
{
}

#include <ColorDocker.moc>

