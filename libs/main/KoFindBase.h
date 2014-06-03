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

#include <QObject>

#include "KoFindMatch.h"
#include "komain_export.h"

class KoFindOptionSet;
/**
 * Base class for searching and finding strings in a document.
 *
 * This class provides the base API that needs to be implemented
 * when searching in a document. Each application should create an
 * instance of a concrete implementation of this class to support
 * searching. Which concrete implementation to create depends on
 * the type of data that is being searched.
 */
class KOMAIN_EXPORT KoFindBase : public QObject
{
    Q_OBJECT
public:
    typedef QList<KoFindMatch> KoFindMatchList;

    explicit KoFindBase(QObject *parent = 0);
    virtual ~KoFindBase();

    /**
     * Retrieve a list of all matches that were found since the
     * last call to find().
     *
     * \return A list of all matches that were found.
     */
    const KoFindMatchList &matches() const;

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

    /**
     * Retrieve the current set of options.
     */
    KoFindOptionSet *options() const;

public Q_SLOTS:
    /**
     * Search for a string in the document associated with this
     * KoFindBase instance.
     *
     * \param pattern The string to search for.
     */
    virtual void find(const QString &pattern);
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
    /**
     * Replace the current match with a new value.
     *
     * \param value The new value to change the current match to.
     */
    virtual void replaceCurrent(const QVariant &value);
    /**
     * Replace all occurrences of the currently matched text
     * with the new value.
     *
     * \param value The new value to change the matches to.
     */
    virtual void replaceAll(const QVariant &value);

Q_SIGNALS:
    /**
     * Emitted whenever a match is found. The argument is the first
     * match if there are more then one.
     */
    void matchFound(KoFindMatch match);
    /**
     * Emitted when the canvas should be redrawn due to a change in
     * the underlying list of matches.
     */
    void updateCanvas();
    /**
     * Emitted when there is no match found for the current pattern.
     */
    void noMatchFound();
    /**
     * Emitted whenever a call to findNext/findPrevious wraps around.
     *
     * \param direction True if searched wrapped while searching forward,
     * false if searching backward.
     */
    void wrapAround(bool direction);
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
    void setMatches(const KoFindMatchList &matches);

    /**
     * Set the current index.
     *
     * This index indicates the currently active match.
     */
    void setCurrentMatch(int index);

    /**
     * Retrieve the index of the current match.
     *
     * \return The index of the current match.
     */
    int currentMatchIndex();

    /**
     * This method should be implemented to do the actual searching.
     *
     * \param pattern The pattern to search for.
     */
    virtual void findImplementation(const QString &pattern, KoFindMatchList &matchList) = 0;

    /**
     * This method should be implemented to do the actual replacing.
     *
     * \param match The match that should be replaced.
     * \param value The new value to replace the match with.
     */
    virtual void replaceImplementation(const KoFindMatch &match, const QVariant &value) = 0;

    /**
     * Clean up the current matches.
     *
     * This can be used to remove highlighting or other
     * modifications and will be called whenever the list of matches
     * needs to be cleaned.
     */
    virtual void clearMatches();

    /**
     * Set the current option set.
     *
     * \param newOptions The new option set.
     */
    void setOptions(KoFindOptionSet *newOptions);

private:
    class Private;
    Private * const d;
};

#endif // KOFINDBASE_H
