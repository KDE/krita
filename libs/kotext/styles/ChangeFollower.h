/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef CHANGEFOLLOWER_H
#define CHANGEFOLLOWER_H

#include "KoStyleManager.h"

#include <QObject>
#include <QTextDocument>
#include <QWeakPointer>

/**
 * This object dies when the parent QTextDocument dies and in the destructor
 * it will deregister itself from the stylemanager.
 * If the stylemanager died before that time, it will not do anything.
 */
class ChangeFollower : public QObject
{

    Q_OBJECT

public:
    /**
     * Create a new ChangeFollower that can update the document with
     * any changes made in the styles managed by the style manager.
     * This class is created by the KoStyleManager to proxy for a document.
     * The reason this is a proxy class instead of simply a couple of methods
     * inside KoStyleManager is for memory management. A stylemanager can
     * maintain a lot of documents and these documents can be deleted without
     * telling the styleManager.  We use the QObject principle of children
     * getting deleted when the parant gets deleted to track the document, which
     * we use as the parant document.
     */
    ChangeFollower(QTextDocument *parent, KoStyleManager *manager);

    /// Destructor, called when the parent is deleted.
    ~ChangeFollower();

    /**
     * Will update all the text in the document with the changes.
     * The document this follower is associated with is scanned for
     * text that has one of the changed styles and on those portions of the text
     * the style will be (re)applied.
     * @param changedStyles a list of styleIds. from KoParagraphStyle::styleId
     *      and KoCharacterStyle::styleId
     */
    void processUpdates(const QList<int> &changedStyles);
    /// return the document this follower is following.
    const QTextDocument *document() const {
        return m_document;
    }

private:
    QTextDocument *m_document;
    QWeakPointer<KoStyleManager> m_styleManager;
};

#endif
