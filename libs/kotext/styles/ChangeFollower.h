/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
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
#include <QPointer>

/**
 * This object dies when the parent QTextDocument dies and in the destructor
 * it will deregister itself from the stylemanager.
 * If the stylemanager died before that time, it will not do anything.
 */
class ChangeFollower : public QObject {
public:
    ChangeFollower(QTextDocument *parent, KoStyleManager *manager);
    ~ChangeFollower();

    void processUpdates(const QList<int> &changedStyles);
    const QTextDocument *document() const { return m_document; }

private:
    QTextDocument *m_document;
    QPointer<KoStyleManager> m_styleManager;
};

#endif
