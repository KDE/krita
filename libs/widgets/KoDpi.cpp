/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>
   Copyright 2003 Nicolas GOUTTE <goutte@kde.org>
   Copyright 2009 Thomas Zander <zander@kde.org>

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

#include "KoDpi.h"

#include <QFontInfo>

#ifdef HAVE_X11
#include <QX11Info>
#else
#include <QApplication>
#include <QDesktopWidget>
#endif

#include <QGlobalStatic>

Q_GLOBAL_STATIC(KoDpi, s_instance)

KoDpi* KoDpi::self()
{
    return s_instance;
}

KoDpi::KoDpi()
{
    // Another way to get the DPI of the display would be QPaintDeviceMetrics,
    // but we have no widget here (and moving this to KoView wouldn't allow
    // using this from the document easily).
#ifdef HAVE_X11
    m_dpiX = QX11Info::appDpiX();
    m_dpiY = QX11Info::appDpiY();
#else
    QDesktopWidget *w = QApplication::desktop();
    if (w) {
        m_dpiX = w->logicalDpiX();
        m_dpiY = w->logicalDpiY();
    } else {
        m_dpiX = 75;
        m_dpiY = 75;
    }
#endif
}

KoDpi::~KoDpi()
{
}

void KoDpi::setDPI(int x, int y)
{
    //debugWidgets << x <<"," << y;
    KoDpi* s = self();
    s->m_dpiX = x;
    s->m_dpiY = y;
}
