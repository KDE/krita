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
#include "kis_gbr_brush_factory.h"

#include <QDomDocument>

#include <KoResourceServer.h>
#include <KoResourceServerAdapter.h>

#include "kis_brush_server.h"


KisGbrBrushFactory::KisGbrBrushFactory()
{
}

KisBrushSP KisGbrBrushFactory::getOrCreateBrush(const QDomElement& brushDefinition)
{

    KoResourceServer<KisBrush>* rServer = KisBrushServer::instance()->brushServer();
    QString brushFileName = brushDefinition.attribute("filename", "9circle.gbr");
    KisBrushSP brush = rServer->resourceByFilename(brushFileName);
    //Fallback for files that still use the old format
    if(!brush) {
        QFileInfo info(brushFileName);
        brush = rServer->resourceByFilename(info.fileName());
    }
    if(!brush) {
        brush = rServer->resources().first();
    }

    bool result;
    QLocale c(QLocale::German);
    double spacing = brushDefinition.attribute("spacing", "1.0").toDouble(&result);
    if (!result){
        spacing = c.toDouble(brushDefinition.attribute("spacing"));
    }
    brush->setSpacing(spacing);

    double angle = brushDefinition.attribute("angle", "0.0").toDouble();
    brush->setAngle(angle);

    double scale = brushDefinition.attribute("scale", "1.0").toDouble();
    brush->setScale(scale);

    return brush;

}
