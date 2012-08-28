/*
   Calligra Report Engine
   Copyright (C) 2011, 2012 by Dag Andersen (danders@get2net.dk)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoOdtFrameReportPicture.h"
#include <KoXmlWriter.h>
#include <KoDpi.h>
#include <KoOdfGraphicStyles.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoUnit.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

#include "renderobjects.h"

#include <KMimeType>
#include <kdebug.h>

#include <QPainter>
#include <QPicture>

#include <kdebug.h>

KoOdtFrameReportPicture::KoOdtFrameReportPicture(OROPrimitive *primitive)
    : KoOdtFrameReportPrimitive(primitive)
{
}

KoOdtFrameReportPicture::~KoOdtFrameReportPicture()
{
}

OROPicture *KoOdtFrameReportPicture::picture() const
{
    return dynamic_cast<OROPicture*>(m_primitive);
}

void KoOdtFrameReportPicture::createBody(KoXmlWriter *bodyWriter) const
{
    bodyWriter->startElement("draw:frame");
    bodyWriter->addAttribute("draw:id", itemName());
    bodyWriter->addAttribute("xml:id", itemName());
    bodyWriter->addAttribute("draw:name", itemName());
    bodyWriter->addAttribute("text:anchor-type", "page");
    bodyWriter->addAttribute("text:anchor-page-number", pageNumber());
    bodyWriter->addAttribute("draw:style-name", m_frameStyleName);

    commonAttributes(bodyWriter);

    bodyWriter->startElement("draw:image");
    bodyWriter->addAttribute("xlink:href", "Pictures/" + pictureName());
    bodyWriter->addAttribute("xlink:type", "simple");
    bodyWriter->addAttribute("xlink:show", "embed");
    bodyWriter->addAttribute("xlink:actuate", "onLoad");
    bodyWriter->endElement();

    bodyWriter->endElement(); // draw:frame
}

bool KoOdtFrameReportPicture::saveData(KoStore* store, KoXmlWriter* manifestWriter) const
{
    QString name = "Pictures/" + pictureName();
    if (!store->open(name)) {
        return false;
    }
    KoStoreDevice device(store);
    QImage image(m_primitive->size().toSize(), QImage::Format_ARGB32);
    image.fill(0);
    QPainter painter;
    painter.begin(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPicture(0, 0, *(picture()->picture()));
    painter.end();
    kDebug()<<image.format();
    bool ok = image.save(&device, "PNG");
    if (ok) {
        const QString mimetype(KMimeType::findByPath(name, 0 , true)->name());
        manifestWriter->addManifestEntry(name,  mimetype);
    }
    ok = store->close() && ok;
    return ok;
}
