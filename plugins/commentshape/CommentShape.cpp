/* This file is part of the KDE project
 * Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
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

#include "CommentShape.h"

#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>

#include <KoShapeLoadingContext.h>
#include <KoShapeApplicationData.h>
#include <KoShapeSavingContext.h>
#include <KoShapeRegistry.h>
#include <KoTextShapeData.h>
#include <KoColorBackground.h>
#include <KoLineBorder.h>

#include <kmessagebox.h>
#include <klocale.h>

#include <QPainter>
#include <QGradient>
#include <QBrush>
#include <QRectF>

#define TextShapeId "TextShapeID"
#include <KoGradientBackground.h>

namespace {
    const QPointF initialsBoxPoint = QPointF(0.0,0.0);
    const QSizeF initialsBoxSize = QSizeF(20.0,20.0);
    const QPointF commentBoxPoint = QPointF(22.0,0.0);
    const QSizeF commentBoxSize = QSizeF(100.0,100.0);

    const QSizeF wholeSize = QSizeF(120.0,100.0);
}

CommentShape::CommentShape(KoResourceManager* resourceManager)
: KoShapeContainer()
, m_selected(false)
{
    KoShapeContainer::setSize(initialsBoxSize);

    m_comment = KoShapeRegistry::instance()->value(TextShapeId)->createDefaultShape(resourceManager);
    if ( !m_comment ) {
//         m_comment = new KoShape;
        KMessageBox::error( 0, i18n("The plugin needed for displaying comments is not present."), i18n("Plugin Missing") );
    }
    if ( dynamic_cast<KoTextShapeData*>( m_comment->userData() ) == 0 ) {
        KMessageBox::error( 0, i18n("The plugin needed for displaying the comment is not compatible with the current version of the comment shape."),
                            i18n("Plugin Incompatible") );
        m_comment->setUserData( new KoTextShapeData );
    }

    m_comment->setSize(commentBoxSize);
    m_comment->setPosition(commentBoxPoint);
    m_comment->setVisible(false);

    QLinearGradient* gradient= new QLinearGradient(commentBoxPoint, QPointF(commentBoxPoint.x(), commentBoxPoint.y() + commentBoxSize.height()));
    gradient->setCoordinateMode(QGradient::ObjectBoundingMode);
    gradient->setColorAt(0.0, Qt::yellow);
    gradient->setColorAt(1.0, QColor(254, 201, 7));
    m_comment->setBackground(new KoGradientBackground(gradient));

    KoLineBorder* border = new KoLineBorder;
    border->setLineBrush(QBrush(Qt::black));
    border->setLineWidth(0.5);
    m_comment->setBorder(border);

    addShape(m_comment);
}

CommentShape::~CommentShape()
{
    delete m_comment;
}

bool CommentShape::loadOdf(const KoXmlElement& element, KoShapeLoadingContext& context)
{
    loadOdfAttributes(element, context, OdfPosition);

    KoXmlElement child;
    forEachElement(child, element)
    {
        if(child.namespaceURI() == KoXmlNS::dc) {
            if(child.localName() == "creator") {
                m_creator = child.text();
                QStringList creatorNames = m_creator.split(' ');
                //TODO fix for RTL
                foreach(QString name, creatorNames) {
                    m_initials += name.left(1);
                }
            }
            else if(child.localName() == "date") {
                m_date = QDate::fromString(child.text(), Qt::ISODate);
            }
        }
        else if(child.namespaceURI() == KoXmlNS::text && child.localName() == "pp") {
            commentData()->document()->setHtml(child.text().replace('\n', "<br>"));
        }
    }

    return true;
}

void CommentShape::saveOdf(KoShapeSavingContext& context) const
{
    KoXmlWriter& writer = context.xmlWriter();

    writer.startElement("officeooo:annotation"); //TODO replace with standarized element name
    saveOdfAttributes(context, OdfPosition);

    writer.startElement("dc:creator");
    writer.addTextSpan(m_creator);
    writer.endElement();//dc:creator

    writer.startElement("dc:date");
    writer.addTextSpan(m_date.toString(Qt::ISODate));
    writer.endElement();//dc:date

    writer.startElement("text:p");
    writer.addTextSpan(commentData()->document()->toPlainText());
    writer.endElement();//text:p

    writer.endElement();//officeooo:annotation
}

void CommentShape::paintComponent(QPainter& painter, const KoViewConverter& converter)
{
    applyConversion(painter, converter);

    QLinearGradient gradient(initialsBoxPoint, QPointF(0, initialsBoxSize.height()));
    qreal lighterPos = 0.0;
    qreal darkerPos = 0.0;
    if(!m_selected){
        darkerPos = 1.0;
    }
    else {
        lighterPos = 1.0;
    }
    gradient.setColorAt(lighterPos, QColor(Qt::yellow));
    gradient.setColorAt(darkerPos, QColor(254, 201, 7));
    const QBrush brush(gradient);
    painter.setBrush(brush);

    painter.setPen(Qt::black);

    const QSizeF size = QSizeF(20,20);
    painter.drawRect(QRectF(initialsBoxPoint, initialsBoxSize));

    painter.drawText( QRectF(initialsBoxPoint, initialsBoxSize), Qt::AlignCenter, m_initials);
}


void CommentShape::setSize(const QSizeF& size)
{
    KoShapeContainer::setSize(initialsBoxSize);
}

void CommentShape::toogleSelect()
{
    m_selected = !m_selected;
    if(!m_selected) {
        KoShapeContainer::setSize(initialsBoxSize);
        m_comment->setVisible(false);
    }
    else {
        KoShapeContainer::setSize(wholeSize);
        m_comment->setVisible(true);
    }
    update();
}

KoTextShapeData* CommentShape::commentData() const
{
    return qobject_cast<KoTextShapeData*>( m_comment->userData() );
}
