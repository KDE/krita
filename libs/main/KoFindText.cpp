/* This file is part of the KDE project
 *
 * Copyright (c) 2010 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#include "KoFindText.h"

#include <QtGui/QTextDocument>
#include <QtGui/QTextCursor>
#include <QtGui/QTextBlock>
#include <QtGui/QTextLayout>

#include <KDE/KDebug>

#include "KoResourceManager.h"
#include "KoText.h"
#include "KoStyleManager.h"
#include <KoTextBlockData.h>
#include <KoTextBlockPaintStrategyBase.h>
#include <QPalette>
#include <QStyle>
#include <QApplication>


class KoFindText::Private
{
    public:
        Private() : document(0) { }
        void resourceChanged(int key, const QVariant& variant);

        QTextDocument *document;

        KoResourceManager *resourceManager;

        static QTextCharFormat *highlightFormat;
        static QTextCharFormat *currentMatchFormat;

        QTextCursor previousMatch;
        QTextCharFormat previousMatchFormat;

        QList< QPair<QTextCursor, QTextCharFormat> > oldFormats;
};

QTextCharFormat * KoFindText::Private::highlightFormat = 0;
QTextCharFormat * KoFindText::Private::currentMatchFormat = 0;

KoFindText::KoFindText(KoResourceManager* provider, QObject* parent)
    : KoFindBase(parent), d(new Private)
{
    d->resourceManager = provider;
    connect(provider, SIGNAL(resourceChanged(int, const QVariant&)), this, SLOT(resourceChanged(int, const QVariant&)));

    if(!d->highlightFormat) {
        d->highlightFormat = new QTextCharFormat();
        d->highlightFormat->setBackground(Qt::yellow);
    }

    if(!d->currentMatchFormat) {
        d->currentMatchFormat = new QTextCharFormat();
        d->currentMatchFormat->setBackground(qApp->palette().highlight());
        d->currentMatchFormat->setForeground(qApp->palette().highlightedText());
    }
}

KoFindText::~KoFindText()
{

}

void KoFindText::findImpl(const QString& pattern, QList<KoFindMatch> & matchList)
{
    QTextDocument::FindFlags flags = 0; //QTextDocument::FindCaseSensitively;

    if(!d->document) {
        QVariant doc = d->resourceManager->resource(KoText::CurrentTextDocument);
        if(doc.isValid()) {
            d->document = static_cast<QTextDocument*>(doc.value<void*>());
        }
    }

    if(!d->document) {
        kWarning() << "No document available for searching!";
        return;
    }

    QList<QTextLayout::FormatRange> overrides;
    
    QTextCursor cursor = d->document->find(pattern, 0, flags);
    while(!cursor.isNull()) {
        //overrides.clear();
//         QTextLayout::FormatRange override;
//         override.format = *(d->highlightFormat);
//         override.start = cursor.anchor();
//         //< cursor.position() ? cursor.anchor() : cursor.position();
//         //qDebug() << "Override Start:" << override.start;
//         override.length = qAbs(cursor.position() - cursor.anchor());
//         //qDebug() << "Override Length:" << override.length;
//         overrides = cursor.block().layout()->additionalFormats();
//         overrides.append(override);
//         cursor.block().layout()->setAdditionalFormats(overrides);
        d->oldFormats.append(QPair<QTextCursor, QTextCharFormat>(cursor, cursor.charFormat()));
        cursor.mergeCharFormat(*d->highlightFormat);
        
        KoFindMatch match;
        match.setContainer(QVariant::fromValue(d->document));
        match.setLocation(QVariant::fromValue(cursor));
        matchList.append(match);
        cursor = d->document->find(pattern, cursor, flags);

        //qDebug() << "==========";
    }

//      if (start) {
//         start = false;
//         restarted = false;
//         strategy->reset();
//         startDocument = document;
//         lastKnownPosition = QTextCursor(document);
//         if (selectedText) {
//             int selectionStart = provider->intResource(KoText::CurrentTextPosition);
//             int selectionEnd = provider->intResource(KoText::CurrentTextAnchor);
//             if (selectionEnd < selectionStart) {
//                 qSwap(selectionStart, selectionEnd);
//             }
//             // TODO the SelectedTextPosition and SelectedTextAnchor are not highlighted yet
//             // it would be cool to have the highlighted ligher when searching in selected text
//             provider->setResource(KoText::SelectedTextPosition, selectionStart);
//             provider->setResource(KoText::SelectedTextAnchor, selectionEnd);
//             if ((options & KFind::FindBackwards) != 0) {
//                 lastKnownPosition.setPosition(selectionEnd);
//                 endPosition.setPosition(selectionStart);
//             } else {
//                 lastKnownPosition.setPosition(selectionStart);
//                 endPosition.setPosition(selectionEnd);
//             }
//             startPosition = lastKnownPosition;
//         } else {
//             if ((options & KFind::FromCursor) != 0) {
//                 lastKnownPosition.setPosition(provider->intResource(KoText::CurrentTextPosition));
//             } else {
//                 lastKnownPosition.setPosition(0);
//             }
//             endPosition = lastKnownPosition;
//             startPosition = lastKnownPosition;
//         }
//         //kDebug() << "start" << lastKnownPosition.position();
//     }
// 
//     QRegExp regExp;
//     //QString pattern = strategy->dialog()->pattern();
//     //if (options & KFind::RegularExpression) {
//     //    regExp = QRegExp(pattern);
//     //}
// 
//     QTextCursor cursor;
//     if (!regExp.isEmpty() && regExp.isValid()) {
//         cursor = document->find(regExp, lastKnownPosition, flags);
//     } else {
//         cursor = document->find(pattern, lastKnownPosition, flags);
//     }
// 
//     //kDebug() << "r" << restarted << "c > e" << ( document == startDocument && cursor > endPosition ) << ( startDocument == document && findDirection->positionReached(  cursor, endPosition ) )<< "e" << cursor.atEnd() << "n" << cursor.isNull();
//     if ((((document == startDocument) && restarted) || selectedText)
//             && (cursor.isNull() || findDirection->positionReached(cursor, endPosition))) {
//         restarted = false;
//         strategy->displayFinalDialog();
//         lastKnownPosition = startPosition;
//         return;
//     } else if (cursor.isNull()) {
//         restarted = true;
//         findDirection->nextDocument(document, this);
//         lastKnownPosition = QTextCursor(document);
//         findDirection->positionCursor(lastKnownPosition);
//         // restart from the beginning
//         parseSettingsAndFind();
//         return;
//     } else {
//         // found something
//         bool goOn = strategy->foundMatch(cursor, findDirection);
//         lastKnownPosition = cursor;
//         if (goOn) {
//             parseSettingsAndFind();
//         }
//     }
}

void KoFindText::Private::resourceChanged(int key, const QVariant& variant)
{
    if (key == KoText::CurrentTextDocument) {
        document = static_cast<QTextDocument*>(variant.value<void*>());
    }
    /*else if (key == KoText::CurrentTextPosition || key == KoText::CurrentTextAnchor) {
        if (!inFind) {
            const int selectionStart = provider->intResource(KoText::CurrentTextPosition);
            const int selectionEnd = provider->intResource(KoText::CurrentTextAnchor);
            findStrategy.dialog()->setHasSelection(selectionEnd != selectionStart);
            replaceStrategy.dialog()->setHasSelection(selectionEnd != selectionStart);

            start = true;
            provider->clearResource(KoText::SelectedTextPosition);
            provider->clearResource(KoText::SelectedTextAnchor);
        }
    }*/
}

void KoFindText::clearMatches()
{
//     KoFindMatchList matchList = matches();
//     foreach(const KoFindMatch &match, matchList) {
//         QTextCursor cursor = match.location().value<QTextCursor>();
//         //cursor.block().layout()->clearAdditionalFormats();
//         if(d->oldFormats.contains(cursor)) {
//             cursor.setCharFormat(d->oldFormats.value(cursor));
//         }
//     }
    for(QList< QPair<QTextCursor, QTextCharFormat> >::iterator itr = d->oldFormats.begin(); itr != d->oldFormats.end(); ++itr) {
        itr->first.setCharFormat(itr->second);
    }
    
    d->oldFormats.clear();
}

void KoFindText::highlightMatch(const KoFindMatch& match)
{
//     d->previousMatch.setCharFormat(d->previousMatchFormat);
// 
    QTextCursor cursor = match.location().value<QTextCursor>();
// 
    d->previousMatch = cursor;
//     d->previousMatchFormat = cursor.charFormat();

//     QTextCharFormat format;
//     format.setBackground(qApp->palette().highlight());
//     format.setForeground(qApp->palette().highlightedText());
//     cursor.mergeCharFormat(format);
    
    QTextLayout::FormatRange override;
    override.format = *(d->currentMatchFormat);
    override.start = cursor.anchor();
    override.length = qAbs(cursor.position() - cursor.anchor());
    QList<QTextLayout::FormatRange> overrides = cursor.block().layout()->additionalFormats();
    overrides.append(override);
    cursor.block().layout()->setAdditionalFormats(overrides);
}

void KoFindText::repaintCanvas()
{
//     const int startPosition = cursor->selectionStart();
//     const int endPosition = cursor->selectionEnd();
//     QList<KoShape *> shapes;
//     KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(cursor->document()->documentLayout());
// 
//     Q_ASSERT(lay);
//     foreach (KoShape* shape, lay->shapes()) {
//         KoTextShapeData *shapeData = dynamic_cast<KoTextShapeData *>(shape->userData());
//         if (shapeData == 0)
//             continue;
// 
//         const int from = shapeData->position();
//         const int end = shapeData->endPosition();
//         if ((from <= startPosition && end >= startPosition && end <= endPosition)
//             || (from >= startPosition && end <= endPosition) // shape totally included
//             || (from <= endPosition && end >= endPosition)
//            )
//             shapes.append(shape);
//     }
// 
//     // loop over all shapes that contain the text and update per shape.
//     QRectF repaintRect = lay->selectionBoundingBox(*cursor);
//     foreach (KoShape *shape, shapes) {
//         QRectF rect = repaintRect;
//         KoTextShapeData *shapeData = static_cast<KoTextShapeData *>(shape->userData());
//         rect.moveTop(rect.y() - shapeData->documentOffset());
//         rect = shape->absoluteTransformation(0).mapRect(rect);
//         canvas()->updateCanvas(shape->boundingRect().intersected(rect));
//     }
}

// uint qHash(const QTextCursor& cursor)
// {
//     qDebug() << cursor.blockNumber() + cursor.anchor() + cursor.position() + cursor.positionInBlock();
//     return cursor.blockNumber() + cursor.anchor() + cursor.position() + cursor.positionInBlock();
// }

#include "KoFindText.moc"
