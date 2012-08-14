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

#include "KoOdtFrameReportCheckBox.h"
#include <KoXmlWriter.h>
#include <KoDpi.h>
#include <KoOdfGraphicStyles.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoUnit.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

#include "renderobjects.h"

#include <QColor>
#include <QPainter>
#include <QPen>
#include <QImage>

#include <kmimetype.h>

#include <kdebug.h>

KoOdtFrameReportCheckBox::KoOdtFrameReportCheckBox(OROCheck *primitive)
    : KoOdtFrameReportPrimitive(primitive)
{
}

KoOdtFrameReportCheckBox::~KoOdtFrameReportCheckBox()
{
}

OROCheck *KoOdtFrameReportCheckBox::checkBox() const
{
    return static_cast<OROCheck*>(m_primitive);
}

void KoOdtFrameReportCheckBox::createStyle(KoGenStyles &coll)
{
    KoGenStyle gs(KoGenStyle::GraphicStyle, "graphic");
    gs.addProperty("draw:fill", "none");
    gs.addPropertyPt("fo:margin", 0);
    gs.addProperty("style:horizontal-pos", "from-left");
    gs.addProperty("style:horizontal-rel", "page");
    gs.addProperty("style:vertical-pos", "from-top");
    gs.addProperty("style:vertical-rel", "page");
    gs.addProperty("style:wrap", "dynamic");
    gs.addPropertyPt("style:wrap-dynamic-threshold", 0);

    QPen pen;
    qreal weight = checkBox()->lineStyle().weight;
    if (weight < 1.0) {
        weight = 1.0;
    }
    pen.setWidthF(weight);
    pen.setColor(checkBox()->lineStyle().lineColor);
    pen.setStyle(checkBox()->lineStyle().style);
    KoOdfGraphicStyles::saveOdfStrokeStyle(gs, coll, pen);

    m_frameStyleName = coll.insert(gs, "F");
}

void KoOdtFrameReportCheckBox::createBody(KoXmlWriter *bodyWriter) const
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
    bodyWriter->addAttribute("xlink:href", "Pictures/" + imageName());
    bodyWriter->addAttribute("xlink:type", "simple");
    bodyWriter->addAttribute("xlink:show", "embed");
    bodyWriter->addAttribute("xlink:actuate", "onLoad");
    bodyWriter->endElement(); // draw:image

    bodyWriter->endElement(); // draw:frame
}

QString KoOdtFrameReportCheckBox::imageName() const
{
    return QString("Checkbox_%1.png").arg(m_uid);
}

bool KoOdtFrameReportCheckBox::saveData(KoStore* store, KoXmlWriter* manifestWriter) const
{
    QString name = "Pictures/" + imageName();
    if (!store->open(name)) {
        return false;
    }
    OROCheck * chk = checkBox();
    QSizeF sz = chk->size();
    QPen fpen; // frame pen
    if (chk->lineStyle().style == Qt::NoPen || chk->lineStyle().weight <= 0) {
        fpen = QPen(QColor(224, 224, 224));
    } else {
        fpen = QPen(chk->lineStyle().lineColor, chk->lineStyle().weight, chk->lineStyle().style);
    }
    QPointF ps(fpen.widthF(), fpen.widthF());
    QRectF rc = QRectF(0, 0, sz.width() + (ps.x()*2), sz.height() + (ps.y()*2));

    QPainter painter;
    QImage image(rc.size().toSize(), QImage::Format_ARGB32);
    image.fill(0);
    painter.begin(&image);
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.setRenderHint(QPainter::Antialiasing);

    qreal ox = sz.width() / 5;
    qreal oy = sz.height() / 5;

    //Checkbox Style
    if (chk->checkType() == "Cross") {
        painter.drawRoundedRect(rc.adjusted(ps.x(), ps.y(), -ps.x(), -ps.y()), sz.width() / 10 , sz.height() / 10);

        if (chk->value()) {
            QPen lp;
            lp.setColor(chk->foregroundColor());
            lp.setWidth(ox > oy ? oy : ox);
            painter.setPen(lp);
            QRectF r = rc.adjusted(ox + ps.x(), oy + ps.y(), -(ox + ps.x()), -(oy + ps.y()));
            painter.drawLine(r.topLeft(), r.bottomRight());
            painter.drawLine(r.bottomLeft(), r.topRight());
        }
    } else if (chk->checkType() == "Dot") {
        //Radio Style
        painter.drawEllipse(rc);

        if (chk->value()) {
            QBrush lb(chk->foregroundColor());
            painter.setBrush(lb);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(rc.center(), sz.width() / 2 - ox, sz.height() / 2 - oy);
        }
    } else {
        //Tickbox Style
        painter.drawRoundedRect(rc.adjusted(ps.x(), ps.y(), -ps.x(), -ps.y()), sz.width() / 10 , sz.height() / 10);

        if (chk->value()) {
            QPen lp;
            lp.setColor(chk->foregroundColor());
            lp.setWidth(ox > oy ? oy : ox);
            painter.setPen(lp);
            painter.drawLine(QPointF(ox, sz.height() / 2) + ps, QPointF(sz.width() / 2, sz.height() - oy) + ps);
            painter.drawLine(QPointF(sz.width() / 2, sz.height() - oy) + ps, QPointF(sz.width() - ox, oy) + ps);
        }
    }
    painter.end();

    KoStoreDevice device(store);
    bool ok = image.save(&device, "PNG");
    if (ok) {
        const QString mimetype(KMimeType::findByPath(name, 0 , true)->name());
        manifestWriter->addManifestEntry(name,  mimetype);
        kDebug()<<"manifest:"<<mimetype;
    }
    bool cl = store->close();
    kDebug()<<ok<<cl;
    return ok && cl;
}
