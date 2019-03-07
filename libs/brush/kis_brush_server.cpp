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
#include "kis_brush_server.h"

#include <QDir>
#include <QApplication>

#include <QGlobalStatic>
#include <KoResourcePaths.h>

#include <KoResource.h>
#include <KoResourceServerProvider.h>

#include <kis_debug.h>

#include "kis_abr_brush.h"
#include "kis_abr_brush_collection.h"
#include "kis_gbr_brush.h"
#include "kis_imagepipe_brush.h"
#include "kis_png_brush.h"
#include "kis_svg_brush.h"

Q_GLOBAL_STATIC(KisBrushServer, s_instance)


class BrushResourceServer : public KoResourceServer<KisBrush>
{

public:

    BrushResourceServer()
        : KoResourceServer<KisBrush>(ResourceType::Brushes)
    {
    }
};

KisBrushServer::KisBrushServer()
{
    m_brushServer = new BrushResourceServer();
}

KisBrushServer::~KisBrushServer()
{
    delete m_brushServer;
}

KisBrushServer* KisBrushServer::instance()
{
    return s_instance;
}

KoResourceServer<KisBrush>* KisBrushServer::brushServer()
{
    return m_brushServer;
}
