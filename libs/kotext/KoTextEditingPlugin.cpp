/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoTextEditingPlugin.h"

#include <QTextDocument>
#include <QTextBlock>
#include <QTextCursor>

#include <KDebug>

class KoTextEditingPlugin::Private
{
public:
    QHash<QString, KAction*> actionCollection;
};

KoTextEditingPlugin::KoTextEditingPlugin()
        : d(new Private())
{
}

KoTextEditingPlugin::~KoTextEditingPlugin()
{
    delete d;
}

void KoTextEditingPlugin::selectWord(QTextCursor &cursor, int cursorPosition) const
{
    cursor.setPosition(cursorPosition);
    QTextBlock block = cursor.block();
    cursor.setPosition(block.position());
    cursorPosition -= block.position();
    QString string = block.text();
    int pos = 0;
    bool space = false;
    QString::Iterator iter = string.begin();
    while (iter != string.end()) {
        if (iter->isSpace()) {
            if (space) ;// double spaces belong to the previous word
            else if (pos < cursorPosition)
                cursor.setPosition(pos + block.position() + 1); // +1 because we don't want to set it on the space itself
            else
                space = true;
        } else if (space)
            break;
        pos++;
        iter++;
    }
    cursor.setPosition(pos + block.position(), QTextCursor::KeepAnchor);
}

QString KoTextEditingPlugin::paragraph(QTextDocument *document, int cursorPosition) const
{
    QTextBlock block = document->findBlock(cursorPosition);
    return block.text();
}

void KoTextEditingPlugin::addAction(const QString &name, KAction *action)
{
    d->actionCollection.insert(name, action);
}

void KoTextEditingPlugin::checkSection(QTextDocument *document, int startPosition, int endPosition)
{
    QTextBlock block = document->findBlock(startPosition);
    int pos = block.position();
    while (true) {
        if (!block.contains(startPosition - 1) && !block.contains(endPosition + 1)) // only parags that are completely in
            finishedParagraph(document, block.position());

        QString text = block.text();
        bool space = true;
        QString::Iterator iter = text.begin();
        while (pos < endPosition && iter != text.end()) {
            bool isSpace = iter->isSpace();
            if (pos >= startPosition && space && !isSpace) // for each word, call finishedWord
                finishedWord(document, pos);
            else if (!isSpace && pos == startPosition)
                finishedWord(document, startPosition);
            space = isSpace;
            pos++;
            iter++;
        }

        if (!(block.isValid() && block.position() + block.length() < endPosition))
            break;
        block = block.next();
    }
}

QHash<QString, KAction*> KoTextEditingPlugin::actions() const
{
    return d->actionCollection;
}

void KoTextEditingPlugin::setCurrentCursorPosition(QTextDocument *document, int cursorPosition)
{
    Q_UNUSED(cursorPosition);
    Q_UNUSED(document);
}

#include <KoTextEditingPlugin.moc>

