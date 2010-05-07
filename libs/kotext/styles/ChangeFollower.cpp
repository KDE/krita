/* This file is part of the KDE project
 * Copyright (C) 2006, 2009 Thomas Zander <zander@kde.org>
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

#include "ChangeFollower.h"
#include "KoCharacterStyle.h"
#include "KoParagraphStyle.h"

#include <QVector>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <kdebug.h>

ChangeFollower::ChangeFollower(QTextDocument *parent, KoStyleManager *manager)
        : QObject(parent),
        m_document(parent),
        m_styleManager(manager)
{
}

ChangeFollower::~ChangeFollower()
{
    KoStyleManager *sm = m_styleManager.data();
    if (sm)
        sm->remove(this);
}

void ChangeFollower::processUpdates(const QList<int> &changedStyles)
{
    KoStyleManager *sm = m_styleManager.data();
    if (sm) {
        // since the stylemanager would be the one calling this method, I doubt this
        // will ever happen.  But better safe than sorry..
        deleteLater();
        return;
    }

    // optimization strategy;  store the formatid of the formats we checked into
    // a qset for 'hits' and 'ignores' and avoid the copying of the format
    // (fragment.charFormat() / block.blockFormat()) when the formatId is
    // already checked previosly

    QTextCursor cursor(m_document);
    QTextBlock block = cursor.block();
    while (block.isValid()) {
        QTextBlockFormat bf = block.blockFormat();
        int id = bf.intProperty(KoParagraphStyle::StyleId);
        if (id > 0 && changedStyles.contains(id)) {
            cursor.setPosition(block.position());
            KoParagraphStyle *style = sm->paragraphStyle(id);
            Q_ASSERT(style);

            style->applyStyle(bf);
            cursor.setBlockFormat(bf);
        }
        QTextCharFormat cf = block.charFormat();
        id = cf.intProperty(KoCharacterStyle::StyleId);
        if (id > 0 && changedStyles.contains(id)) {
            KoCharacterStyle *style = sm->characterStyle(id);
            Q_ASSERT(style);
            style->applyStyle(block);
        }

        QTextBlock::iterator iter = block.begin();
        while (! iter.atEnd()) {
            QTextFragment fragment = iter.fragment();
            cf = fragment.charFormat();
            id = cf.intProperty(KoCharacterStyle::StyleId);
            if (id > 0 && changedStyles.contains(id)) {
                // create selection
                cursor.setPosition(fragment.position());
                cursor.setPosition(fragment.position() + fragment.length(), QTextCursor::KeepAnchor);
                KoCharacterStyle *style = sm->characterStyle(id);
                Q_ASSERT(style);

                style->applyStyle(cf);
                cursor.mergeCharFormat(cf);
            }
            iter++;
        }
        block = block.next();
    }
}

#include <ChangeFollower.moc>

