/* This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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

#include "KoDeleteChangeMarker.h"

//KOffice includes
#include <KoTextDocument.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoTextShapeSavingContext.h>
#include <opendocument/KoTextSharedSavingData.h>
#include "KoChangeTrackerElement.h"
#include "KoChangeTracker.h"

//KDE includes
#include <kdebug.h>

//Qt includes
#include <QFontMetrics>
#include <QTextInlineObject>
#include <QPainter>

class KoDeleteChangeMarker::Private
{
public:
    Private() {}

    KoChangeTracker *changeTracker;
    QString text;
    int id;
};

KoDeleteChangeMarker::KoDeleteChangeMarker(KoChangeTracker* changeTracker)
        : d(new Private())
{
    d->changeTracker = changeTracker;
}

KoDeleteChangeMarker::~KoDeleteChangeMarker()
{
    delete d;
}
/*
void KoDeleteChangeMarker::setText (const QString& text)
{
    d->text = text;
}

QString KoDeleteChangeMarker::text() const
{
    return d->text;
}
*/
void KoDeleteChangeMarker::setChangeId (int id)
{
    d->id = id;
}

int KoDeleteChangeMarker::changeId() const
{
    return d->id;
}

bool KoDeleteChangeMarker::loadOdf(const KoXmlElement &element)
{
    Q_UNUSED(element)
    return false;
}

void KoDeleteChangeMarker::paint(QPainter& painter, QPaintDevice *pd, const QTextDocument *document, const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(posInDocument);
    Q_UNUSED(document);

    if (!d->changeTracker)
        return;

    Q_ASSERT(format.isCharFormat());

    if (d->changeTracker->isEnabled() && d->changeTracker->elementById(d->id)->isEnabled() && d->changeTracker->displayDeleted()) {
        QFont font(format.font(), pd);
        QTextLayout layout(d->changeTracker->elementById(d->id)->getDeleteData(), font, pd);
        layout.setCacheEnabled(true);
        QList<QTextLayout::FormatRange> layouts;
        QTextLayout::FormatRange range;
        range.start = 0;
        range.length = d->changeTracker->elementById(d->id)->getDeleteData().length();
        range.format = format;
        range.format.setBackground(QBrush(Qt::red));
        layouts.append(range);
        layout.setAdditionalFormats(layouts);

        QTextOption option(Qt::AlignLeft | Qt::AlignAbsolute);
        option.setTextDirection(object.textDirection());
        layout.setTextOption(option);
        layout.beginLayout();
        layout.createLine();
        layout.endLayout();
        layout.draw(&painter, rect.topLeft());
    }
}

void KoDeleteChangeMarker::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(posInDocument);
    Q_UNUSED(document);

    if (!d->changeTracker)
        return;

    Q_ASSERT(format.isCharFormat());
    QFontMetricsF fm(format.font(), pd);

    if (d->changeTracker->isEnabled() && d->changeTracker->elementById(d->id)->isEnabled() && d->changeTracker->displayDeleted()) {
        object.setWidth(fm.width(d->changeTracker->elementById(d->id)->getDeleteData()));
        object.setAscent(fm.ascent());
        object.setDescent(fm.descent());
    } else {
        object.setWidth(0);
        object.setAscent(fm.ascent());
        object.setDescent(fm.descent());
    }
}

void KoDeleteChangeMarker::updatePosition(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(document);
    Q_UNUSED(object);
    Q_UNUSED(posInDocument);
    Q_UNUSED(format);
}

void KoDeleteChangeMarker::saveOdf(KoShapeSavingContext &context)
{
    KoGenChange change;
    QString changeName;
    KoTextSharedSavingData *sharedData = 0;
    if (context.sharedData(KOTEXT_SHARED_SAVING_ID)) {
        sharedData = dynamic_cast<KoTextSharedSavingData*>(context.sharedData(KOTEXT_SHARED_SAVING_ID));
        if (!sharedData) {
            kWarning(32500) << "There is no KoTextSharedSavingData in the context. This should not be the case";
            return;
        }
    }
    d->changeTracker->saveInlineChange(d->id, change);
    changeName = sharedData->genChanges().insert(change);

    context.xmlWriter().startElement("text:change", false);
    context.xmlWriter().addAttribute("text:change-id", changeName);
    context.xmlWriter().endElement();
}
