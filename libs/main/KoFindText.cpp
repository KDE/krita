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
#include <KDE/KLocalizedString>

#include "KoResourceManager.h"
#include "KoText.h"
#include "KoTextDocumentLayout.h"
#include "KoShape.h"
#include "KoTextShapeData.h"
#include "KoCanvasBase.h"
#include "KoShapeManager.h"
#include "KoFindOptionSet.h"
#include "KoFindOption.h"

class KoFindText::Private
{
    public:
        Private() : document(0), selectionStart(-1), selectionEnd(-1) { }

        void resourceChanged(int key, const QVariant& variant);
        void updateSelections();

        QTextDocument *document;

        KoResourceManager *resourceManager;

        QTextCursor selection;
        QVector<QAbstractTextDocumentLayout::Selection> selections;

        int selectionStart;
        int selectionEnd;

        static QTextCharFormat *highlightFormat;
        static QTextCharFormat *currentMatchFormat;
        static QTextCharFormat *currentSelectionFormat;
};

QTextCharFormat * KoFindText::Private::currentSelectionFormat = 0;
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

    if(!d->currentSelectionFormat) {
        d->currentSelectionFormat = new QTextCharFormat();
        d->currentSelectionFormat->setBackground(qApp->palette().alternateBase());
    }

    KoFindOptionSet *options = new KoFindOptionSet();
    options->addOption("caseSensitive", i18n("Case Sensitive"), i18n("Match cases when searching"), QVariant::fromValue<bool>(false));
    options->addOption("wholeWords", i18n("Whole Words Only"), i18n("Match only whole words"), QVariant::fromValue<bool>(false));
    setOptions(options);
}

KoFindText::~KoFindText()
{

}

void KoFindText::findImpl(const QString& pattern, QList<KoFindMatch> & matchList)
{
    KoFindOptionSet *opts = options();
    QTextDocument::FindFlags flags = 0;

    if(opts->option("caseSensitive")->value().toBool()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if(opts->option("wholeWords")->value().toBool()) {
        flags |= QTextDocument::FindWholeWords;
    }

    int start = 0;
    bool findInSelection = false;
//     if(opts & FindWithinSelection && d->selectionStart != d->selectionEnd) {
//         QAbstractTextDocumentLayout::Selection selection;
//         QTextCursor cursor;
//         cursor.setPosition(d->selectionStart);
//         cursor.setPosition(d->selectionEnd, QTextCursor::KeepAnchor);
//         selection.cursor = cursor;
//         selection.format = *(d->currentSelectionFormat);
//         d->selections.append(selection);
// 
//         findInSelection = true;
//         start = d->selectionStart;
//     }

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

    int position = 0;
//     if(opts & FindFromCursor) {
//         position = d->resourceManager->intResource(KoText::CurrentTextPosition);
//     }

    int currentMatch = 0;
    bool matchFound;
    QTextCursor cursor = d->document->find(pattern, start, flags);
    while(!cursor.isNull()) {
        if(findInSelection && d->selectionEnd <= cursor.position()) {
            break;
        }

        QAbstractTextDocumentLayout::Selection selection;
        selection.cursor = cursor;
        selection.format = *(d->highlightFormat);
        d->selections.append(selection);

        KoFindMatch match;
        match.setContainer(QVariant::fromValue(d->document));
        match.setLocation(QVariant::fromValue(cursor));
        matchList.append(match);

        if(position <= qMin(cursor.anchor(), cursor.position())) {
            matchFound = true;
        }

        if(!matchFound) {
            currentMatch++;
        }

        cursor = d->document->find(pattern, cursor, flags);
    }

    if(d->selections.size() > 0) {
        if(position >= d->selections.size()) {
            position = 0;
        }

        setCurrentMatch(position);
        d->selections[position].format = *(d->currentMatchFormat);
    }

    d->updateSelections();
}

void KoFindText::clearMatches()
{
    d->selections.clear();
    d->updateSelections();

    d->selectionStart = -1;
    d->selectionEnd = -1;

    setCurrentMatch(0);
}

void KoFindText::findNext()
{
    if(d->selections.size() == 0)
        return;

    d->selections[currentMatchIndex()].format = *(d->highlightFormat);
    KoFindBase::findNext();
    d->selections[currentMatchIndex()].format = *(d->currentMatchFormat);
    d->updateSelections();
}

void KoFindText::findPrevious()
{
    if(d->selections.size() == 0)
        return;

    d->selections[currentMatchIndex()].format = *(d->highlightFormat);
    KoFindBase::findPrevious();
    d->selections[currentMatchIndex()].format = *(d->currentMatchFormat);
    d->updateSelections();
}

void KoFindText::Private::resourceChanged(int key, const QVariant& variant)
{
    if (key == KoText::CurrentTextDocument) {
        document = static_cast<QTextDocument*>(variant.value<void*>());
    } else if(key == KoText::SelectedTextPosition) {
       // qDebug() << "SelectionChanged";
//         if(selectionStart == -1) {
        selectionStart = variant.toInt();
//         }
    } else if(key == KoText::CurrentTextAnchor) {
       // qDebug() << "SelectionChanged";
//         if(selectionEnd == -1) {
        selectionEnd = variant.toInt();
//         }
    }
}

void KoFindText::Private::updateSelections()
{
    KoTextDocument doc(document);
    doc.setSelections(selections);
}

#include "KoFindText.moc"
