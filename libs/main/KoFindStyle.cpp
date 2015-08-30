/* This file is part of the KDE project
 *
 * Copyright (c) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#include "KoFindStyle.h"
#include "KoFindOptionSet.h"
#include "KoFindOption.h"

#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>
#include <KoTextDocument.h>

#include <QTextDocument>
#include <QTextBlock>
#include <QTextCursor>
#include <QAbstractTextDocumentLayout>

Q_DECLARE_METATYPE(QTextDocument *)
Q_DECLARE_METATYPE(QTextCursor)

class Q_DECL_HIDDEN KoFindStyle::Private
{
public:
    QList<QTextDocument*> documents;
    QHash<QTextDocument*, QVector<QAbstractTextDocumentLayout::Selection> > selections;

    static QTextCharFormat highlightFormat;
    void updateSelections();
};

QTextCharFormat KoFindStyle::Private::highlightFormat;

KoFindStyle::KoFindStyle(QObject* parent)
    : KoFindBase(parent), d(new Private)
{
    KoFindOptionSet *options = new KoFindOptionSet();
    options->addOption("paragraphStyle", "Paragraph Style", QString(), QVariant::fromValue<int>(0));
    options->addOption("characterStyle", "Character Style", QString(), QVariant::fromValue<int>(0));
    setOptions(options);

    d->highlightFormat.setBackground(Qt::yellow);
}

KoFindStyle::~KoFindStyle()
{
    delete d;
}

QList< QTextDocument* > KoFindStyle::documents() const
{
    return d->documents;
}

void KoFindStyle::setDocuments(const QList< QTextDocument* >& list)
{
    clearMatches();
    d->documents = list;
}

void KoFindStyle::clearMatches()
{
    d->selections.clear();
    foreach(QTextDocument* doc, d->documents) {
        d->selections.insert(doc, QVector<QAbstractTextDocumentLayout::Selection>());
    }
    d->updateSelections();
}

void KoFindStyle::replaceImplementation(const KoFindMatch& /*match*/, const QVariant& /*value*/)
{

}

void KoFindStyle::findImplementation(const QString& /*pattern*/, KoFindBase::KoFindMatchList& matchList)
{
    int charStyle = options()->option("characterStyle")->value().toInt();
    int parStyle = options()->option("paragraphStyle")->value().toInt();

    foreach(QTextDocument *document, d->documents) {
        QTextBlock block = document->firstBlock();
        QVector<QAbstractTextDocumentLayout::Selection> selections;
        while(block.isValid()) {
            if(block.blockFormat().intProperty(KoParagraphStyle::StyleId) == parStyle) {
                for(QTextBlock::iterator itr = block.begin(); itr != block.end(); ++itr) {
                    if(itr.fragment().charFormat().intProperty(KoCharacterStyle::StyleId) == charStyle) {
                        QTextCursor cursor(document);
                        cursor.setPosition(itr.fragment().position());
                        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, itr.fragment().length());
                        matchList.append(KoFindMatch(QVariant::fromValue(document), QVariant::fromValue(cursor)));

                        QAbstractTextDocumentLayout::Selection selection;
                        selection.cursor = cursor;
                        selection.format = d->highlightFormat;
                        selections.append(selection);
                    }
                }
            }
            block = block.next();
        }
        d->selections.insert(document, selections);
    }

    d->updateSelections();
}

void KoFindStyle::Private::updateSelections()
{
    QHash< QTextDocument*, QVector<QAbstractTextDocumentLayout::Selection> >::iterator itr;
    for(itr = selections.begin(); itr != selections.end(); ++itr) {
        KoTextDocument doc(itr.key());
        doc.setSelections(itr.value());
    }
}
