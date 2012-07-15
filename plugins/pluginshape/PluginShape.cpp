/* This file is part of the KDE project
 * Copyright (C) 2010  Vidhyapria  Arunkumar <vidhyapria.arunkumar@nokia.com>
 * Copyright (C) 2010  Amit Aggarwal <amit.5.aggarwal@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "PluginShape.h"

#include <KoViewConverter.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoStoreDevice.h>
#include <KoUnit.h>

#include <QPainter>
#include <kdebug.h>
#include <klocale.h>


PluginShape::PluginShape()
    : KoFrameShape(KoXmlNS::draw, "plugin")
{
    setKeepAspectRatio(true);
}

PluginShape::~PluginShape()
{
}

void PluginShape::paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &)
{
    QRectF pixelsF = converter.documentToView(QRectF(QPointF(0,0), size()));
    painter.fillRect(pixelsF, QColor(Qt::yellow));
    painter.setPen(Qt::blue);
    QString mimetype = i18n("Unknown");
    if (!m_mimetype.isEmpty()) {
        mimetype = m_mimetype;
    }
    painter.drawText(pixelsF, Qt::AlignCenter, i18n("Plugin of mimetype: %1", mimetype));
}

void PluginShape::saveOdf(KoShapeSavingContext &context) const
{
    KoXmlWriter &writer = context.xmlWriter();

    writer.startElement("draw:frame");
    saveOdfAttributes(context, OdfAllAttributes);
    writer.startElement("draw:plugin");
    // cannot use "this" as referent for context.xmlid, already done for getting the xml:id for the frame
    // so (randomly) choosing m_xlinkhref to get another, separate unique referent for this shape
    const QString xmlId = context.xmlid(&m_xlinkhref, QLatin1String("plugin"), KoElementReference::Counter).toString();
    writer.addAttribute("xml:id", xmlId);
    writer.addAttribute("draw:mime-type", m_mimetype);
    writer.addAttribute("xlink:type", m_xlinktype);
    writer.addAttribute("xlink:show", m_xlinkshow);
    writer.addAttribute("xlink:actuate", m_xlinkactuate);
    writer.addAttribute("xlink:href", m_xlinkhref);

    QMap<QString,QString>::const_iterator itr = m_drawParams.constBegin();
    while (itr != m_drawParams.constEnd()) {
        writer.startElement("draw:param", true);
        writer.addAttribute("draw:name", itr.key());
        writer.addAttribute("draw:value", itr.value());
        writer.endElement(); // draw:param
        ++itr;
    }
    writer.endElement(); // draw:plugin
    saveOdfCommonChildElements(context);
    writer.endElement(); // draw:frame

}

bool PluginShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    loadOdfAttributes(element, context, OdfAllAttributes);
    return loadOdfFrame(element, context);
}

bool PluginShape::loadOdfFrameElement(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(context);
    if(element.isNull()) {
        return false;
    }

    if(element.localName() == "plugin") {
        m_mimetype  = element.attributeNS(KoXmlNS::draw, "mime-type");
        m_xlinktype  = element.attributeNS(KoXmlNS::xlink, "type");
        m_xlinkshow  = element.attributeNS(KoXmlNS::xlink, "show");
        m_xlinkactuate  = element.attributeNS(KoXmlNS::xlink, "actuate");
        m_xlinkhref  = element.attributeNS(KoXmlNS::xlink, "href");
        m_drawParams.clear();
        if(element.hasChildNodes()) {
            KoXmlNode node = element.firstChild();
            while(!node.isNull()) {
                if(node.isElement()) {
                    KoXmlElement nodeElement = node.toElement();
                    if(nodeElement.localName() == "param") {
                        QString name = nodeElement.attributeNS(KoXmlNS::draw, "name");
                        if(!name.isEmpty()) {
                            m_drawParams.insert(name,nodeElement.attributeNS(KoXmlNS::draw, "value"));
                        }
                    }
                }
                node = node.nextSibling();
            }
        }
        return true;
    }
    return false;
}
