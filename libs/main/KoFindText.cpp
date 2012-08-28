/* This file is part of the KDE project
 *
 * Copyright (c) 2010 Arjen Hiemstra <ahiemstra@heimr.nl>
 * Copyright (C) 2011 Thorsten Zachmann <zachmann@kde.org>
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

#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextLayout>
#include <QPalette>
#include <QStyle>
#include <QApplication>
#include <QAbstractTextDocumentLayout>

#include <KDE/KDebug>
#include <KDE/KLocalizedString>

#include <KoText.h>
#include <KoTextDocument.h>
#include <KoShape.h>
#include <KoShapeContainer.h>
#include <KoTextShapeData.h>

#include "KoFindOptionSet.h"
#include "KoFindOption.h"
#include "KoDocument.h"

class KoFindText::Private
{
public:
    Private(KoFindText* qq) : q(qq), selectionStart(-1), selectionEnd(-1) { }

    void updateSelections();
    void updateDocumentList();
    void documentDestroyed(QObject *document);
    void updateCurrentMatch(int position);
    static void initializeFormats();

    KoFindText *q;

    QList<QTextDocument*> documents;

    QTextCursor currentCursor;
    QTextCursor selection;
    QHash<QTextDocument*, QVector<QAbstractTextDocumentLayout::Selection> > selections;

    int selectionStart;
    int selectionEnd;

    static QTextCharFormat highlightFormat;
    static QTextCharFormat currentMatchFormat;
    static QTextCharFormat currentSelectionFormat;
    static QTextCharFormat replacedFormat;
    static bool formatsInitialized;

    QPair<QTextDocument*, int> currentMatch;
};

QTextCharFormat KoFindText::Private::highlightFormat;
QTextCharFormat KoFindText::Private::currentMatchFormat;
QTextCharFormat KoFindText::Private::currentSelectionFormat;
QTextCharFormat KoFindText::Private::replacedFormat;
bool KoFindText::Private::formatsInitialized = false;

KoFindText::KoFindText(QObject* parent)
    : KoFindBase(parent), d(new Private(this))
{
    d->initializeFormats();

    KoFindOptionSet *options = new KoFindOptionSet();
    options->addOption("caseSensitive", i18n("Case Sensitive"), i18n("Match cases when searching"), QVariant::fromValue<bool>(false));
    options->addOption("wholeWords", i18n("Whole Words Only"), i18n("Match only whole words"), QVariant::fromValue<bool>(false));
    options->addOption("fromCursor", i18n("Find from Cursor"), i18n("Start searching from the current cursor"), QVariant::fromValue<bool>(true));
    setOptions(options);
}

KoFindText::~KoFindText()
{
    delete d;
}

void KoFindText::findImplementation(const QString &pattern, QList<KoFindMatch> & matchList)
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

    if(d->documents.size() == 0) {
        kWarning() << "No document available for searching!";
        return;
    }

    bool before = opts->option("fromCursor")->value().toBool() && !d->currentCursor.isNull();
    QList<KoFindMatch> matchBefore;
    foreach(QTextDocument* document, d->documents) {
        QTextCursor cursor = document->find(pattern, start, flags);
        QVector<QAbstractTextDocumentLayout::Selection> selections;
        while(!cursor.isNull()) {
            if(findInSelection && d->selectionEnd <= cursor.position()) {
                break;
            }

            if (before && document == d->currentCursor.document() && d->currentCursor < cursor) {
                before = false;
            }

            QAbstractTextDocumentLayout::Selection selection;
            selection.cursor = cursor;
            selection.format = d->highlightFormat;
            selections.append(selection);

            KoFindMatch match;
            match.setContainer(QVariant::fromValue(document));
            match.setLocation(QVariant::fromValue(cursor));
            if (before) {
                matchBefore.append(match);
            }
            else {
                matchList.append(match);
            }

            cursor = document->find(pattern, cursor, flags);
        }
        if (before && document == d->currentCursor.document()) {
            before = false;
        }
        d->selections.insert(document, selections);
    }
    matchList.append(matchBefore);

    if (hasMatches()) {
        setCurrentMatch(0);
        d->updateCurrentMatch(0);
    }

    d->updateSelections();
}

void KoFindText::replaceImplementation(const KoFindMatch &match, const QVariant &value)
{
    if (!match.isValid() || !match.location().canConvert<QTextCursor>() || !match.container().canConvert<QTextDocument*>()) {
        return;
    }

    QTextCursor cursor = match.location().value<QTextCursor>();

    //Search for the selection matching this match.
    QVector<QAbstractTextDocumentLayout::Selection> selections = d->selections.value(match.container().value<QTextDocument*>());
    int index = 0;
    foreach(QAbstractTextDocumentLayout::Selection sel, selections) {
        if(sel.cursor == cursor) {
            break;
        }
        index++;
    }

    cursor.insertText(value.toString());
    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, value.toString().length());

    selections[index].cursor = cursor;
    selections[index].format = d->replacedFormat;
    d->selections.insert(match.container().value<QTextDocument*>(), selections);

    d->updateCurrentMatch(0);
    d->updateSelections();
}

void KoFindText::clearMatches()
{
    d->selections.clear();
    foreach(QTextDocument* doc, d->documents) {
        d->selections.insert(doc, QVector<QAbstractTextDocumentLayout::Selection>());
    }
    d->updateSelections();

    d->selectionStart = -1;
    d->selectionEnd = -1;

    setCurrentMatch(0);
    d->currentMatch.first = 0;
}

QList< QTextDocument* > KoFindText::documents() const
{
    return d->documents;
}

void KoFindText::findNext()
{
    if(d->selections.size() == 0) {
        return;
    }

    KoFindBase::findNext();
    d->updateCurrentMatch(currentMatchIndex());
    d->updateSelections();
}

void KoFindText::findPrevious()
{
    if(d->selections.size() == 0) {
        return;
    }

    KoFindBase::findPrevious();
    d->updateCurrentMatch(currentMatchIndex());
    d->updateSelections();
}

void KoFindText::setCurrentCursor(const QTextCursor &cursor)
{
    d->currentCursor = cursor;
}

void KoFindText::setDocuments(const QList<QTextDocument*> &documents)
{
    clearMatches();
    d->documents = documents;
    d->updateDocumentList();
}

void KoFindText::findTextInShapes(const QList<KoShape*> &shapes, QList<QTextDocument*> &append)
{
    foreach(KoShape* shape, shapes) {
        KoShapeContainer *container = dynamic_cast<KoShapeContainer*>(shape);
        if(container) {
            findTextInShapes(container->shapes(), append);
        }

        KoTextShapeData *shapeData = dynamic_cast<KoTextShapeData*>(shape->userData());
        if (!shapeData)
            continue;

        if(shapeData->document()) {
            if(!append.contains(shapeData->document())) {
                append.append(shapeData->document());
            }
        }
    }
}

void KoFindText::Private::updateSelections()
{
    QHash< QTextDocument*, QVector<QAbstractTextDocumentLayout::Selection> >::iterator itr;
    for(itr = selections.begin(); itr != selections.end(); ++itr) {
        KoTextDocument doc(itr.key());
        doc.setSelections(itr.value());
    }
}

void KoFindText::Private::updateDocumentList()
{
    foreach(QTextDocument *document, documents) {
        connect(document, SIGNAL(destroyed(QObject*)), q, SLOT(documentDestroyed(QObject*)), Qt::UniqueConnection);
    }
}

void KoFindText::Private::documentDestroyed(QObject *document)
{
    QTextDocument* doc = qobject_cast<QTextDocument*>(document);
    if(doc) {
        selections.remove(doc);
        documents.removeOne(doc);
    }
}

void KoFindText::Private::updateCurrentMatch(int position)
{
    Q_UNUSED(position);
    if (currentMatch.first != 0) {
        QVector<QAbstractTextDocumentLayout::Selection> sel = selections.value(currentMatch.first);
        Q_ASSERT(currentMatch.second < sel.count());
        if(sel[currentMatch.second].format == currentMatchFormat) {
            sel[currentMatch.second].format = highlightFormat;
        }
        selections.insert(currentMatch.first, sel);
    }

    const KoFindMatch match = q->currentMatch();
    if (match.isValid() && match.location().canConvert<QTextCursor>() && match.container().canConvert<QTextDocument*>()) {
        QTextCursor cursor = match.location().value<QTextCursor>();
        QTextDocument *document = match.container().value<QTextDocument*>();
        QVector<QAbstractTextDocumentLayout::Selection> sel = selections.value(document);
        for (int i = 0; i < sel.size(); ++i) {
            if (sel[i].cursor == cursor) {
                sel[i].format = currentMatchFormat;
                selections.insert(document, sel);
                currentMatch.first = document;
                currentMatch.second = i;
                break;
            }
        }
    }
}

void KoFindText::Private::initializeFormats()
{
    if (!formatsInitialized) {
        highlightFormat.setBackground(Qt::yellow);
        currentMatchFormat.setBackground(qApp->palette().highlight());
        currentMatchFormat.setForeground(qApp->palette().highlightedText());
        currentSelectionFormat.setBackground(qApp->palette().alternateBase());
        replacedFormat.setBackground(Qt::green);
        formatsInitialized = true;
    }
}

void KoFindText::setFormat(FormatType formatType, const QTextCharFormat &format)
{
    KoFindText::Private::initializeFormats();

    switch (formatType) {
    case HighlightFormat:
        KoFindText::Private::highlightFormat = format;
        break;
    case CurrentMatchFormat:
        KoFindText::Private::currentMatchFormat = format;
        break;
    case SelectionFormat:
        KoFindText::Private::currentSelectionFormat = format;
        break;
    case ReplacedFormat:
        KoFindText::Private::replacedFormat = format;
        break;
    }
}

#include "KoFindText.moc"
