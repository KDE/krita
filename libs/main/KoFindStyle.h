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

#ifndef KOFINDSTYLE_H
#define KOFINDSTYLE_H

#include "KoFindBase.h"

#include <QMetaType>
#include <QTextDocument>
#include <QTextCursor>

#include "komain_export.h"

/**
 * \brief Implementation of KoFindBase that can find style usage.
 *
 * This class will search through a set of QTextDocuments for uses of  a paragraph
 * and character style. It will highlight the character style.
 *
 * The following options are required to be set before it works:
 * <ul>
 *      <li><strong>characterStyle</strong>: Int. Default undefined. The ID of a
 *              character style to search for.</li>
 *      <li><strong>paragraphStyle</strong>: Int. Default undefined. The ID of a
 *              paragraph style to search for.</li>
 * </ul>
 *
 * \note Before you can use this class, be sure to set a list of QTextDocuments
 * using setDocuments().
 *
 * Matches created by this implementation use QTextDocument for the container and
 * QTextCursor for the location.
 */
class KOMAIN_EXPORT KoFindStyle : public KoFindBase
{
public:
    /**
     * Constructor.
     */
    explicit KoFindStyle(QObject* parent = 0);
    /**
     * Destructor.
     */
    virtual ~KoFindStyle();

    /**
     * Return the list of documents currently being used for searching.
     */
    QList<QTextDocument*> documents();
    /**
     * Set the list of documents to use for searching.
     *
     * \param list A list of document to search through.
     */
    void setDocuments(const QList<QTextDocument*> &list);

    /**
     * Reimplemented from KoFindBase::clearMatches()
     */
    virtual void clearMatches();

protected:
    /**
     * Reimplemented from KoFindBase::replaceImplementation().
     *
     * \note Replace is currently not supported in this class.
     */
    virtual void replaceImplementation(const KoFindMatch& match, const QVariant& value);
    /**
     * Reimplemented from KoFindBase::findImplementation()
     */
    virtual void findImplementation(const QString& pattern, KoFindBase::KoFindMatchList& matchList);

private:
    class Private;
    Private * const d;
};

#endif // KOFINDSTYLE_H
