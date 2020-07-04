/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "KisBrushServerProvider.h"

#include <QDir>
#include <QApplication>

#include <QGlobalStatic>
#include <KoResourcePaths.h>

#include <KoResource.h>

#include <kis_debug.h>

Q_GLOBAL_STATIC(KisBrushServerProvider, s_instance)


KisBrushServerProvider::KisBrushServerProvider()
{
    m_brushServer = new KoResourceServer<KisBrush>(ResourceType::Brushes);
}

KisBrushServerProvider::~KisBrushServerProvider()
{
    delete m_brushServer;
}

KisBrushServerProvider* KisBrushServerProvider::instance()
{
    return s_instance;
}

KoResourceServer<KisBrush>* KisBrushServerProvider::brushServer()
{
    return m_brushServer;
}
