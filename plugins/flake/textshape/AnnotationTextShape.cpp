/* This file is part of the KDE project
 * Copyright (C) 2013 Mojtaba Shahi Senobari <mojtaba.shahi3000@gmail.com>
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

#include "AnnotationTextShape.h"

#include <KoInlineTextObjectManager.h>
#include <KoTextRangeManager.h>
#include <KoShapeLoadingContext.h>
#include <KoShapePaintingContext.h>
#include <KoShapeSavingContext.h>
#include <KoViewConverter.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoTextLoader.h>
#include <KoColorBackground.h>

#include <QFont>
#include <QPainter>
#include <QPen>
#include <QTextLayout>

#include <QDebug>

AnnotationTextShape::AnnotationTextShape(KoInlineTextObjectManager *inlineTextObjectManager, KoTextRangeManager *textRangeManager)
    : TextShape(inlineTextObjectManager, textRangeManager)
    , m_creator()
    , m_date()
    , m_dateString()
{
    setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(Qt::yellow)));
    setGeometryProtected(true);
}

AnnotationTextShape::~AnnotationTextShape()
{
}

void AnnotationTextShape::setAnnotaionTextData(KoTextShapeData *textShapeData)
{
    m_textShapeData = textShapeData;
    m_textShapeData->setTopPadding(HeaderSpace);
    m_textShapeData->setLeftPadding(qreal(4.0)); // Make it a little nicer
    m_textShapeData->setRightPadding(qreal(4.0));
    m_textShapeData->setBottomPadding(qreal(4.0));
    m_textShapeData->setResizeMethod(KoTextShapeData::AutoGrowHeight);
}

void AnnotationTextShape::paintComponent(QPainter &painter, const KoViewConverter &converter,
        KoShapePaintingContext &paintcontext)
{
    if (paintcontext.showAnnotations) {
        TextShape::paintComponent(painter, converter, paintcontext);
        QRectF clipRect = outlineRect();

        // Paint creator and of creation of the annotation.
        QPen peninfo(Qt::darkYellow);
        QFont serifFont("Times", HeaderFontSize, QFont::Bold);
        painter.setPen(peninfo);
        painter.setFont(serifFont);

        QDate date = QDate::fromString(m_date, Qt::ISODate);
        QString info = "  " + m_creator + "\n  " + date.toString(Qt::LocalDate);
        painter.drawText(clipRect, Qt::AlignTop, info);
    }
}

bool AnnotationTextShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    //qDebug() << "****** Start Load odf ******";

    KoTextLoader textLoader(context);
    QTextCursor cursor(textShapeData()->document());

    const QString localName(element.localName());

    if (localName == "annotation") {

        // FIXME: Load more attributes here

        // Load the metadata (author, date) and contents here.
        KoXmlElement el;
        forEachElement(el, element) {
            if (el.localName() == "creator" && el.namespaceURI() == KoXmlNS::dc) {
                m_creator = el.text();
                if (m_creator.isEmpty()) {
                    m_creator = "Unknown";
                }
            } else if (el.localName() == "date" && el.namespaceURI() == KoXmlNS::dc) {
                m_date = el.text();
            } else if (el.localName() == "datestring" && el.namespaceURI() == KoXmlNS::meta) {
                m_dateString = el.text();
            }
        }
        textLoader.loadBody(element, cursor);
        //qDebug() << "****** End Load ******";
    } else {
        // something pretty weird going on...
        return false;
    }
    return true;
}

void AnnotationTextShape::saveOdf(KoShapeSavingContext &context) const
{
    //qDebug() << " ****** Start saveing annotation shape **********";
    KoXmlWriter *writer = &context.xmlWriter();

    writer->startElement("dc:creator", false);
    writer->addTextNode(m_creator);
    writer->endElement(); // dc:creator
    writer->startElement("dc:date", false);
    writer->addTextNode(m_date);
    writer->endElement(); // dc:date

    if (!m_dateString.isEmpty()) {
        writer->startElement("meta:date-string", false);
        writer->addTextNode(m_dateString);
        writer->endElement(); // meta:date-string
    }

    m_textShapeData->saveOdf(context, 0, 0, -1);
}

void AnnotationTextShape::setCreator(const QString &creator)
{
    m_creator = creator;
}

QString AnnotationTextShape::creator() const
{
    return m_creator;
}

void AnnotationTextShape::setDate(const QString &date)
{
    m_date = date;
}

QString AnnotationTextShape::date() const
{
    return m_date;
}

void AnnotationTextShape::setDateString(const QString &dateString)
{
    m_dateString = dateString;
}

QString AnnotationTextShape::dateString() const
{
    return m_dateString;
}

const qreal AnnotationTextShape::HeaderSpace = 25.0; // The space needed for the annotation header.
const qreal AnnotationTextShape::HeaderFontSize = 6.0;
