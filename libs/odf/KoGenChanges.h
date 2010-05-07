/* This file is part of the KDE project
   Copyright (C) 2008 Pierre Stirnweiss <pierre.stirnweiss_koffice@gadz.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOGENCHANGES_H
#define KOGENCHANGES_H

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QMultiMap>
#include <QtCore/QSet>
#include <QtCore/QString>
#include "koodf_export.h"
#include "KoGenStyle.h"

#include <KoGenChange.h>

/**
 * @brief Repository of changes used during saving of OASIS/OOo file.
 *
 * Inspired from KoGenStyles.h
 *
 * Is used to store all the change regions, which will be saved at the beginning of <office:body><office:text> elements
 * We use a container outside the changeTracker, as the change tracker is linked to the document of a TextShapeData and is then not aware of the other TextShapeData.
 *
 */
class KOODF_EXPORT KoGenChanges
{
public:

    struct NamedChange {
        const KoGenChange* change; ///< @note owned by the collection
        QString name;
    };


    typedef QMap<KoGenChange, QString> ChangeMap;
    typedef QSet<QString> NameMap;
    typedef QList<NamedChange> ChangeArray;

    KoGenChanges();
    ~KoGenChanges();

    /**
     * Look up a change in the collection, inserting it if necessary.
     * This assigns a name to the change and returns it.
     *
     * @param change the change to look up.
     * @param name proposed internal name for the change.
     * If this name is already in use (for another change), then a number is appended
     * to it until unique.
     *
     * @return the name for this change
     */
    QString insert(const KoGenChange &change, const QString &name = QString());

    /**
     * Return the entire collection of styles
     * Use this for saving the styles
     */
    ChangeMap changes() const;

    /**
     * @return an existing change by name
     */
    const KoGenChange *change(const QString &name) const;

    /**
     * Save changes.
     *
     * This creates the text:changed-region tag containing all
     * changes.
     *
     * @param xmlWriter
     * @param stylesDotXml
     */
    void saveOdfChanges(KoXmlWriter *xmlWriter) const;

private:
    QString makeUniqueName(const QString &base) const;

    class Private;
    Private * const d;
};

#endif /* KOGENCHANGES_H */
