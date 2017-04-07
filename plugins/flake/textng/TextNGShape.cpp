/* This file is part of the KDE project
 *
 * Copyright 2017 Boudewijn Rempt <boud@valdyas.org>
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

// Own
#include "TextNGShape.h"

// Posix
#include <math.h>

// Qt
#include <QFontDatabase>
#include <QPen>
#include <QPainter>
#include <QBuffer>
#include <QDataStream>
#include <QMutexLocker>
#include <QThreadPool>
#include <QSvgRenderer>

// Calligra
#include "KoUnit.h"
#include "KoStore.h"
#include "KoXmlNS.h"
#include "KoXmlReader.h"
#include "KoXmlWriter.h"
#include <KoEmbeddedDocumentSaver.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoViewConverter.h>

#include "kis_painting_tweaks.h"

#include <QTextLayout>
#include <QTextBlock>
#include <QTextDocument>


class TextNGShape::Private
{
public:
    QTextDocument *document;
};

TextNGShape::TextNGShape()
    : KoShape()
    , d(new TextNGShape::Private())
{
    setShapeId(TextNGShape_SHAPEID);
    // Default size of the shape.
    KoShape::setSize(QSizeF(CM_TO_POINT(8), CM_TO_POINT(5)));
    d->document = new QTextDocument();
    d->document->setHtml("<body><p>M'enfin!<br/>Ils sont fous, ces <b>Romains</b>!</p></body>");

}

TextNGShape::~TextNGShape()
{
    delete d;
}

void TextNGShape::paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintContext)
{
    QTextLayout *layout= d->document->firstBlock().layout();
    layout->beginLayout();
    qreal textY = 0;
    while (true) {
        QTextLine line = layout->createLine();
        if (!line.isValid()) {
            break;
        }
        line.setLineWidth(boundingRect().width());
        line.setPosition(QPointF(0, textY));
        textY+=line.height();
    }
    layout->endLayout();
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(QRectF(boundingRect().topLeft()-position(), boundingRect().size()), Qt::yellow);
    painter.setBrush(QBrush(Qt::black));
    painter.setPen(QPen(Qt::black));
    layout->draw(&painter, boundingRect().topLeft()-position());
    painter.restore();
}

void TextNGShape::saveOdf(KoShapeSavingContext &Context) const
{
}

bool TextNGShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &Context)
{
    return true;
}

bool TextNGShape::loadOdfFrameElement(const KoXmlElement &element,
                                      KoShapeLoadingContext &Context)
{
    return true;
}

bool TextNGShape::saveSvg(SvgSavingContext &context)
{
    return false;
}

bool TextNGShape::loadSvg(const KoXmlElement &element, SvgLoadingContext &context)
{
    return false;
}
