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
#include <KoShapeLoadingContext.h>
#include <opendocument/KoTextSharedSavingData.h>
#include "KoChangeTrackerElement.h"
#include "KoChangeTracker.h"

//KDE includes
#include <kdebug.h>

//Qt includes
#include <QFontMetrics>
#include <QTextDocument>
#include <QTextInlineObject>
#include <QPainter>

/*********************************** ODF Bug Work-around code **********************************************/
const QString KoDeleteChangeMarker::RDFListName("http://www.koffice.org/list#");
const QString KoDeleteChangeMarker::RDFListItemName("http://www.koffice.org/list-item#");
const QString KoDeleteChangeMarker::RDFListValidity("http://www.kofficde.org/list-status#valid");
const QString KoDeleteChangeMarker::RDFListItemValidity("http://www.koffice.org/list-item-status#valid");
const QString KoDeleteChangeMarker::RDFListLevel("http://www.koffice.org/list-status#level");
const QString KoDeleteChangeMarker::RDFDeleteChangeContext("http://www.koffice.org/deleteChangeMetadata");
/***********************************************************************************************************/

class KoDeleteChangeMarker::Private
{
public:
    Private() {}

    KoChangeTracker *changeTracker;
    QTextDocument *document;
    QString text;
    int id;
    int position;
    QString deleteChangeXml;
    QHash<KoListStyle::ListIdType, KoListStyle *> deletedListStyles;
};

KoDeleteChangeMarker::KoDeleteChangeMarker(KoChangeTracker* changeTracker)
        : d(new Private())
{
    d->changeTracker = changeTracker;
    d->document = 0;
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

int KoDeleteChangeMarker::position() const
{
    return d->position;
}

void KoDeleteChangeMarker::setDeleteChangeXml(QString &deleteChangeXml)
{
    d->deleteChangeXml = deleteChangeXml;
}

bool KoDeleteChangeMarker::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element)
    Q_UNUSED(context);
    return false;
}

void KoDeleteChangeMarker::paint(QPainter& painter, QPaintDevice *pd, const QTextDocument *document, const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(painter);
    Q_UNUSED(pd);
    Q_UNUSED(document);
    Q_UNUSED(rect);
    Q_UNUSED(posInDocument);
    Q_UNUSED(object);
    Q_UNUSED(format);
}

void KoDeleteChangeMarker::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(document);
    Q_UNUSED(object);
    Q_UNUSED(format);
    Q_UNUSED(pd);
    Q_UNUSED(document);

    d->position = posInDocument;
    object.setWidth(0);
}

void KoDeleteChangeMarker::updatePosition(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format)
{
    d->position = posInDocument;
    if (document != d->document)
        d->document = const_cast<QTextDocument*>(document); //TODO: when we get rid of the current visualisation of deleted changes (ie inserting them in the doc), we can get rid of this.

    Q_UNUSED(object);
    Q_UNUSED(format);
}

QTextDocument* KoDeleteChangeMarker::document() const
{
    return d->document;
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
    change.addChildElement("deleteChangeXml", d->deleteChangeXml);
    changeName = sharedData->genChanges().insert(change);

    context.xmlWriter().startElement("text:change", false);
    context.xmlWriter().addAttribute("text:change-id", changeName);
    context.xmlWriter().endElement();
}

void KoDeleteChangeMarker::setDeletedListStyle(KoListStyle::ListIdType id, KoListStyle *style)
{
    d->deletedListStyles.insert(id, style);
}

KoListStyle *KoDeleteChangeMarker::getDeletedListStyle(KoListStyle::ListIdType id)
{
    return d->deletedListStyles.value(id);
}

