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



#ifndef KOFINDBASE_H
#define KOFINDBASE_H

#include <QtCore/QObject>

#include "KoFindMatch.h"
#include "komain_export.h"

/**
 * Base class for searching and finding strings in a document.
 *
 * This class provides the base API that needs to be implemented
 * when searching in a document.
 */
class KOMAIN_EXPORT KoFindBase : public QObject
{
    Q_OBJECT
public:
    typedef QList<KoFindMatch> KoFindMatchList;
    
    explicit KoFindBase(QObject* parent = 0);
    virtual ~KoFindBase();

    /**
     * Retrieve a list of all matches that were found since the
     * last call to find().
     *
     * \return A list of all matches that were found.
     */
    const KoFindMatchList& matches() const;

    /**
     * Check whether any matches were found since the last call to
     * find().
     *
     * \return true if any matches were found, false if otherwise.
     */
    bool hasMatches() const;

    /**
     * Retrieve the current active match.
     *
     * The current match is the match which has currently been centred
     * on in the GUI.
     *
     * \return The current active match.
     */
    KoFindMatch currentMatch() const;

public Q_SLOTS:
    /**
     * Search for a string in the document associated with this
     * KoFindBase instance.
     *
     * \param pattern The string to search for.
     */
    virtual void find(const QString& pattern);
    /**
     * Find the next match, if there is more than one match.
     * This function will wrap around if the end of the document
     * was reached.
     */
    virtual void findNext();
    /**
     * Find the previous match, if there is more than one match.
     * This function will wrap around if the end of the document
     * was reached.
     */
    virtual void findPrevious();
    /**
     * Finished with searching.
     *
     * This clears all highlighting and other markers.
     */
    virtual void finished();

Q_SIGNALS:
    /**
     * Emitted when searching has finished and the matches should be
     * highlighted. Use matches() to retrieve the list of matches.
     */
    void highlight();
    /**
     * Emitted whenever a match is found. The argument is the first
     * match if there are more then one.
     */
    void matchFound(KoFindMatch match);
    /**
     * Emitted when there is no match found for the current pattern.
     */
    void noMatchFound();
    /**
     * Emitted whenever a call to findNext/findPrevious wraps around.
     */
    void wrapAround();
    /**
     * Emitted after find() was called and matches were found.
     */
    void hasMatchesChanged(bool hasMatches);

protected:
    /**
     * Set the list of matches.
     *
     * \param matches The new list of matches to set.
     */
    void setMatches(const KoFindMatchList& matches);
    
    /**
     * Set the current index.
     *
     * This index indicates the currently active match.
     */
    void setCurrentMatch(int index);

    /**
     * This method should be implemented to do the actual searching.
     *
     * \param pattern The pattern to search for.
     */
    virtual void findImpl(const QString &pattern, KoFindMatchList &matchList) = 0;

    /**
     * Clean up the current matches.
     *
     * This can be used to remove highlighting or other
     * modifications and will be called whenever the list of matches
     * needs to be cleaned.
     */
    virtual void clearMatches();

private:
    class Private;
    Private * const d;
};

#endif // KOFINDBASE_H
