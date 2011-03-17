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



#ifndef KOFINDTEXT_H
#define KOFINDTEXT_H

#include "KoFindBase.h"
#include "komain_export.h"

#include <QtCore/QMetaTypeId>

class QTextDocument;
class QTextCursor;
class KoResourceManager;
class KoCanvasBase;
class KOMAIN_EXPORT KoFindText : public KoFindBase
{
    Q_OBJECT
public:
    KoFindText(KoResourceManager *provider, QObject *parent = 0);
    virtual ~KoFindText();

    void highlightMatch(const KoFindMatch& match);

Q_SIGNALS:
    //void findDocumentSetNext(QTextDocument* document);
    //void findDocumentSetPrevious(QTextDocument* document);

protected:
    virtual void findImpl(const QString& pattern, QList< KoFindMatch >& matchList);
    virtual void clearMatches();

private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void resourceChanged(int, const QVariant&))
};

Q_DECLARE_METATYPE(QTextDocument*);
Q_DECLARE_METATYPE(QTextCursor);

#endif // KOFINDTEXT_H
