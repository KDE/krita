/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_abr_brush_factory.h"

#include <QDomDocument>

#include <KoResourceServer.h>
#include <KoResourceServerAdapter.h>

#include "kis_brush_server.h"


KisAbrBrushFactory::KisAbrBrushFactory()
{
}

KisBrushSP KisAbrBrushFactory::getOrCreateBrush(const QDomElement& brushDefinition)
{
    KoResourceServer<KisBrush>* rServer = KisBrushServer::instance()->brushServer();
    QString brushName = brushDefinition.attribute("name", "test_1");
    
    KisBrushSP brush = rServer->getResourceByName(brushName);
    if (!brush){
        return 0;
    }
    
    double spacing = brushDefinition.attribute("spacing", "0.25").toDouble();
    brush->setSpacing(spacing);

    return brush;

}
