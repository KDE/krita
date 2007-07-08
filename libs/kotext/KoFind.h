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
#ifndef KOFIND_H
#define KOFIND_H

#include "kotext_export.h"

#include <QObject>
#include <QList>
#include <QVariant>

class KoCanvasResourceProvider;
class KActionCollection;
class QTextDocument;

class KOTEXT_EXPORT KoFind : public QObject {
    Q_OBJECT
public:
    KoFind(QWidget *parent, KoCanvasResourceProvider *provider, KActionCollection *ac);
    ~KoFind();

private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void resourceChanged(int, const QVariant&) )
    Q_PRIVATE_SLOT(d, void findActivated() )
    Q_PRIVATE_SLOT(d, void findNextActivated() )
    Q_PRIVATE_SLOT(d, void findPreviousActivated() )
    Q_PRIVATE_SLOT(d, void replaceActivated() )
    Q_PRIVATE_SLOT(d, void startFind() )
};

#endif

