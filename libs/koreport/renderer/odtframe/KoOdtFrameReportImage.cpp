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

#include "KoOdtFrameReportImage.h"
#include <KoXmlWriter.h>
#include <KoDpi.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

#include "renderobjects.h"

#include <kmimetype.h>
#include <kdebug.h>

KoOdtFrameReportImage::KoOdtFrameReportImage(OROImage *primitive)
    : KoOdtFrameReportPrimitive(primitive)
{
}

KoOdtFrameReportImage::~KoOdtFrameReportImage()
{
}

OROImage *KoOdtFrameReportImage::image() const
{
    return static_cast<OROImage*>(m_primitive);
}

void KoOdtFrameReportImage::setImageName(const QString &name)
{
    m_name = name;
}

void KoOdtFrameReportImage::createBody(KoXmlWriter *bodyWriter) const
{
    bodyWriter->startElement("draw:frame");
    bodyWriter->addAttribute("draw:style-name", "picture");
    bodyWriter->addAttribute("draw:id", itemName());
    bodyWriter->addAttribute("draw:name", itemName());
    bodyWriter->addAttribute("text:anchor-type", "page");
    bodyWriter->addAttribute("text:anchor-page-number", pageNumber());
    bodyWriter->addAttribute("draw:style-name", m_frameStyleName);

    commonAttributes(bodyWriter);

    bodyWriter->startElement("draw:image");
    bodyWriter->addAttribute("xlink:href", "Pictures/" + imageName());
    bodyWriter->addAttribute("xlink:type", "simple");
    bodyWriter->addAttribute("xlink:show", "embed");
    bodyWriter->addAttribute("xlink:actuate", "onLoad");
    bodyWriter->endElement();

    bodyWriter->endElement(); // draw:frame
}

bool KoOdtFrameReportImage::saveData(KoStore* store, KoXmlWriter* manifestWriter) const
{
    QString name = "Pictures/" + imageName();
    if (!store->open(name)) {
        return false;
    }
    KoStoreDevice device(store);
    bool ok = image()->image().save(&device, "PNG");
    if (ok) {
        const QString mimetype(KMimeType::findByPath(name, 0 , true)->name());
        manifestWriter->addManifestEntry(name,  mimetype);
    }
    ok = store->close() && ok;
    return ok;
}
