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
/**
 * \brief KoFindBase implementation for searching within text shapes.
 *
 * This class provides a link between KoFindBase and QTextDocument for searching.
 * It uses KoText::CurrentTextDocument for determining what text to search through.
 *
 * Matches created by this implementation use QTextDocument for the container and
 * QTextCursor for the location.
 */
class KOMAIN_EXPORT KoFindText : public KoFindBase
{
    Q_OBJECT
public:
    /**
     * Constructor.
     *
     * \param provider The current document's resource manager, used for retrieving
     * the actual text to search through.
     */
    KoFindText(KoResourceManager *provider, QObject *parent = 0);
    virtual ~KoFindText();

    /**
     * Overridden from KoFindBase
     */
    virtual void findNext();
    /**
     * Overridden from KoFindBase
     */
    virtual void findPrevious();


Q_SIGNALS:
    //void findDocumentSetNext(QTextDocument* document);
    //void findDocumentSetPrevious(QTextDocument* document);
    
protected:
    /**
     * The actual implementation of the searching, overridden from KoFindBase.
     */
    virtual void findImpl(const QString& pattern, QList< KoFindMatch >& matchList);
    /**
     * The actual implementation of replacing, overridden from KoFindBase.
     */
    virtual void replaceImpl(const KoFindMatch& match, const QVariant& value);
    /**
     * Clear the list of matches properly. Overridden from KoFindBase.
     */
    virtual void clearMatches();

private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void resourceChanged(int, const QVariant&))
};

Q_DECLARE_METATYPE(QTextDocument*);
Q_DECLARE_METATYPE(QTextCursor);

#endif // KOFINDTEXT_H
