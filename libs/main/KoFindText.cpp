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
#include <QtGui/QPalette>
#include <QtGui/QStyle>
#include <QtGui/QApplication>

#include <KDE/KDebug>

#include "KoResourceManager.h"
#include "KoText.h"
#include "KoTextDocumentLayout.h"
#include "KoShape.h"
#include "KoTextShapeData.h"
#include "KoCanvasBase.h"
#include <KoShapeManager.h>

class KoFindText::Private
{
    public:
        Private() : document(0) { }
        
        void resourceChanged(int key, const QVariant& variant);
        void updateSelections();

        QTextDocument *document;

        KoResourceManager *resourceManager;

        QTextCursor selection;

        static QTextCharFormat *highlightFormat;
        static QTextCharFormat *currentMatchFormat;

        QVector<QAbstractTextDocumentLayout::Selection> selections;
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
    KoFindOptions opts = options();
    QTextDocument::FindFlags flags;

    if(opts & FindCaseSensitive) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if(opts & FindWholeWords) {
        flags |= QTextDocument::FindWholeWords;
    }

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

    QTextCursor cursor = d->document->find(pattern, 0, flags);
    while(!cursor.isNull()) {
        QAbstractTextDocumentLayout::Selection selection;
        selection.cursor = cursor;
        selection.format = *(d->highlightFormat);
        d->selections.append(selection);
        
        KoFindMatch match;
        match.setContainer(QVariant::fromValue(d->document));
        match.setLocation(QVariant::fromValue(cursor));
        matchList.append(match);
        
        cursor = d->document->find(pattern, cursor, flags);
    }

    d->updateSelections();
}

void KoFindText::clearMatches()
{
    d->selections.clear();
    d->updateSelections();
    
    setCurrentMatch(0);
}

void KoFindText::highlightMatch(const KoFindMatch& match)
{
    d->selections[currentMatchIndex()].format = *(d->currentMatchFormat);
    d->updateSelections();
}

void KoFindText::findNext()
{
    d->selections[currentMatchIndex()].format = *(d->highlightFormat);
    KoFindBase::findNext();
}

void KoFindText::findPrevious()
{
    d->selections[currentMatchIndex()].format = *(d->highlightFormat);
    KoFindBase::findPrevious();
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

void KoFindText::Private::updateSelections()
{
    QList<KoShape*> shapes = qobject_cast<KoTextDocumentLayout*>(document->documentLayout())->shapes();
    foreach(KoShape* shape, shapes)
    {
        KoTextShapeData *textShapeData = qobject_cast<KoTextShapeData*>(shape->userData());
        textShapeData->setSelections(selections);
    }
}

#include "KoFindText.moc"
