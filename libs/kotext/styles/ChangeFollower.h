/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2012 C. Boemann <cbo@boemann.dk>
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
#include <QSet>
#include <QTextBlockFormat>
#include <QTextCharFormat>

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
     * getting deleted when the parent gets deleted to track the document, which
     * we use as the parent document.
     */
    ChangeFollower(QTextDocument *parent, KoStyleManager *manager);

    /// Destructor, called when the parent is deleted.
    ~ChangeFollower();

    /**
     * Will collect the needed info from the document finding the autostyles that we
     * need to preserve during theprocessUpdates
     *
     * @param changedStyles a list of styleIds. from KoParagraphStyle::styleId
     *      and KoCharacterStyle::styleId
     */
    void collectNeededInfo(const QSet<int> &changedStyles);

    /**
     * Will update all the text in the document that were identified during the collect
     * stage.
     * during the apply stage the updated styles are applied.
     */
    void processUpdates();

    /// return the document this follower is following.
    const QTextDocument *document() const {
        return m_document;
    }

private:
    /**
     * Helper function for clearing common properties.
     *
     * Clears properties in @a firstFormat that have the same value in @a secondFormat.
     */
    void clearCommonProperties(QTextFormat *firstFormat, const QTextFormat &secondFormat);

private:
    struct Memento
    {
        int blockPosition;
        int paragraphStyleId;
        QTextBlockFormat blockDirectFormat;
        QTextBlockFormat blockParentFormat;
        QTextCharFormat blockDirectCharFormat;
        QTextCharFormat blockParentCharFormat;
        QList<QTextCharFormat> fragmentDirectFormats;
        QList<QTextCursor> fragmentCursors;
        QList<int> fragmentStyleId;
    };
    QList<Memento *> m_mementos;
    QTextDocument *m_document;
    QWeakPointer<KoStyleManager> m_styleManager;
};

#endif
