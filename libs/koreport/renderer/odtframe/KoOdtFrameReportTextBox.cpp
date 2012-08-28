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

#include "KoOdtFrameReportTextBox.h"
#include <KoXmlWriter.h>
#include <KoDpi.h>
#include <KoOdfGraphicStyles.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>

#include "renderobjects.h"

#include <QColor>
#include <QFont>
#include <QPen>

#include <kdebug.h>

KoOdtFrameReportTextBox::KoOdtFrameReportTextBox(OROTextBox *primitive)
    : KoOdtFrameReportPrimitive(primitive)
{
}

KoOdtFrameReportTextBox::~KoOdtFrameReportTextBox()
{
}

OROTextBox *KoOdtFrameReportTextBox::textBox() const
{
    return dynamic_cast<OROTextBox*>(m_primitive);
}

void KoOdtFrameReportTextBox::createStyle(KoGenStyles &coll)
{
    QFont font = textBox()->textStyle().font;

    KoGenStyle ps(KoGenStyle::ParagraphStyle, "paragraph");
    m_paragraphStyleName = coll.insert(ps, "P");

    // text style
    KoGenStyle ts(KoGenStyle::TextStyle, "text");
    ts.addProperty("fo:font-family", font.family());
    ts.addPropertyPt("fo:font-size", font.pointSizeF());
    ts.addProperty("fo:font-weight", font.weight() * 10);
    ts.addProperty("fo:color", textBox()->textStyle().foregroundColor.name());
    QString fs;
    switch (font.style()) {
        case QFont::StyleNormal: fs = "normal"; break;
        case QFont::StyleItalic: fs = "italic"; break;
        case QFont::StyleOblique: fs = "oblique"; break;
    }
    ts.addProperty("fo:font-style", fs);
    m_textStyleName = coll.insert(ts, "T");

    KoGenStyle gs(KoGenStyle::GraphicStyle, "graphic");
    QPen pen;
    pen.setColor(textBox()->lineStyle().lineColor);
    pen.setWidthF(textBox()->lineStyle().weight);
    pen.setStyle(textBox()->lineStyle().style);
    KoOdfGraphicStyles::saveOdfStrokeStyle(gs, coll, pen);
    gs.addProperty("style:horizontal-pos", "from-left");
    gs.addProperty("style:horizontal-rel", "page");
    gs.addProperty("style:vertical-pos", "from-top");
    gs.addProperty("style:vertical-rel", "page");
    gs.addProperty("fo:background-color", textBox()->textStyle().backgroundColor.name());
    m_frameStyleName = coll.insert(gs, "F");
}

void KoOdtFrameReportTextBox::createBody(KoXmlWriter *bodyWriter) const
{
    bodyWriter->startElement("draw:frame");
    bodyWriter->addAttribute("draw:id", itemName());
    bodyWriter->addAttribute("xml:id", itemName());
    bodyWriter->addAttribute("draw:name", itemName());
    bodyWriter->addAttribute("text:anchor-type", "page");
    bodyWriter->addAttribute("text:anchor-page-number", pageNumber());
    bodyWriter->addAttribute("draw:style-name", m_frameStyleName);

    commonAttributes(bodyWriter);

    bodyWriter->startElement("draw:text-box");

    bodyWriter->startElement("text:p");
    bodyWriter->addAttribute("text:style-name", m_paragraphStyleName);
    bodyWriter->startElement("text:span");
    bodyWriter->addAttribute("text:style-name", m_textStyleName);
    bodyWriter->addTextNode(textBox()->text());

    bodyWriter->endElement(); // text:span
    bodyWriter->endElement(); // text:p
    bodyWriter->endElement(); // draw:text-box
    bodyWriter->endElement(); // draw:frame
}
