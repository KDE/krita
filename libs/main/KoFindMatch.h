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

#ifndef KOFINDMATCH_H
#define KOFINDMATCH_H

#include <QSharedDataPointer>
#include "komain_export.h"

class QVariant;

/**
 * \brief Encapsulation of a search result.
 *
 * Due to the fact that Calligra handles many types of documents,
 * we cannot rely on something like QTextCursor working for each
 * document. Thus, we need to encapsulate the results into
 * something more generic. This class serves that function.
 *
 * This generalisation means this class only stores QVariant
 * values. The contents of these variants is dependent on the
 * KoFindBase-derived search implementation and the documentation
 * of these classes should be consulted for details regarding
 * implementation specifics.
 *
 * This class is implicitly-shared and thus safe to copy around.
 *
 * \see KoFindBase
 */
class KOMAIN_EXPORT KoFindMatch
{
public:
    /**
     * Default constructor. Creates a match with an invalid
     * document and location.
     */
    KoFindMatch();
    /**
     * Constructor. Creates a match with the given values used.
     *
     * \param container The container containing the match.
     * \param location The location of the match within the container.
     *
     * \see container()
     * \see location()
     */
    KoFindMatch(const QVariant &container, const QVariant &location);
    /**
     * Copy constructor.
     *
     * \param other The match to copy.
     */
    KoFindMatch(const KoFindMatch &other);
    /**
     * Destructor.
     */
    ~KoFindMatch();

    KoFindMatch &operator=(const KoFindMatch &other);

    /**
     * Equals operator.
     *
     * \param other The other match to compare to.
     *
     * \return True if this.container == other.container
     * and this.location == other.location.
     */
    bool operator==(const KoFindMatch &other) const;

    /**
     * Check whether this is a valid match.
     *
     * \return True if this.container is valid and
     * this.location is valid.
     */
    bool isValid() const;

    /**
     * Retrieve the container of this match.
     *
     * The container is the first structure that has a clear
     * "contains" relationship with a match. For example, for a
     * block of text, this will be a QTextDocument. For a
     * spreadsheet, it will instead be a sheet. See the
     * implementation documentation for specific details.
     *
     * \return The container of this match.
     */
    QVariant container() const;

    /**
     * Set the container of this match.
     *
     * \param container The new container to set.
     */
    void setContainer(const QVariant &container);

    /**
     * Retrieve the location of this match.
     *
     * \return The location of this match.
     */
    QVariant location() const;

    /**
     * Set the location of this match.
     *
     * \param location The new location to set.
     */
    void setLocation(const QVariant &location);

private:
    class Private;
    QSharedDataPointer<Private> d;
};

#endif // KOFINDMATCH_H
