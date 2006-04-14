/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KOUSERSTYLECOLLECTION_H
#define KOUSERSTYLECOLLECTION_H

class KoUserStyle;
#include <koffice_export.h>
#include <QStringList>

/**
 * Generic style collection class, for all "user styles" (see KoUserStyle).
 * To use this for a particular kind of style, derive from KoUserStyle (to add the properties)
 * and derive from KoUserStyleCollection (to add loading, saving, as well as
 * re-defined findStyle and addStyle in order to cast to the correct style class).
 */
class KOTEXT_EXPORT KoUserStyleCollection
{
public:
    /**
     * Constructor
     * @param prefix used by generateUniqueName to prefix new style names
     * (to avoid clashes between different kinds of styles)
     */
    KoUserStyleCollection( const QString& prefix );

    /**
     * Destructor
     * Deletes all styles.
     */
    ~KoUserStyleCollection();

    /**
     * Erase all styles
     */
    void clear();

    /**
     * @return true if the collection is empty
     */
    bool isEmpty() const { return m_styleList.isEmpty(); }
    /**
     * @return the number of items in the collection
     */
    int count() const { return m_styleList.count(); }
    /**
     * @return the index of @p style in the collection
     */
    int indexOf( KoUserStyle* style ) const { return m_styleList.indexOf( style ); }

    /**
     * Return the list of all styles in the collection.
     */
    QList<KoUserStyle *> styleList() const { return m_styleList; }

    /**
     * Generate a new unique name, to create a style whose internal name
     * differs from the internal name of all existing styles.
     * The prefix passed to the constructor is used here.
     */
    QString generateUniqueName() const;

    /**
     * Return the list composed of the display-name of each style in the collection
     */
    QStringList displayNameList() const;

    /**
     * Find style based on the internal name @p name.
     * If the style with that name can't be found, then<br>
     * 1) if @p name equals @p defaultStyleName, return the first one, never 0<br>
     * 2) otherwise return 0
     */
    KoUserStyle* findStyle( const QString & name, const QString& defaultStyleName ) const;

    /**
     * Find style based on the display name @p displayName.
     * There could be 0, 1 or more than 1 style with that name,
     * the method simply returns the first one found, or 0 if none was found.
     * This is mostly useful to detect similar styles when importing styles
     * from another document.
     */
    KoUserStyle* findStyleByDisplayName( const QString& displayName ) const;

    /**
     * Remove @p style from the collection. If the style isn't in the collection, nothing happens.
     * The style mustn't be deleted yet; it is stored into a list of styles to delete in clear().
     */
    void removeStyle( KoUserStyle *style );

    /**
     * Reorder the styles in the collection.
     * @param list the list of internal names of the styles
     * WARNING, if an existing style isn't listed, it will be lost
     */
    void updateStyleListOrder( const QStringList& list );

    /**
     * Try adding @p sty to the collection.
     * From the moment of this call, the collection owns the style.
     *
     * Either this succeeds, and @p sty is returned, or a style with the exact same
     * internal name and display name is present already, in which case the existing style
     * is updated, @p sty is deleted, and the existing style is returned.
     *
     * WARNING: @p sty can be deleted; use the returned value for any further processing.
     */
    KoUserStyle* addStyle( KoUserStyle* sty );

    /**
     * @return true if this collection only holds the default styles provided by the application
     * When true, those styles don't need to be saved.
     */
    bool isDefault() const { return m_default; }
    /**
     * Set whether this collection only holds the default styles provided by the application
     */
    void setDefault( bool d ) { m_default = d; }

protected:
    KoUserStyleCollection( const KoUserStyleCollection& rhs ); ///< forbidden
    void operator=( const KoUserStyleCollection& rhs ); ///< forbidden

    QList<KoUserStyle *> m_styleList;

private:
    QList<KoUserStyle *> m_deletedStyles;
    const QString m_prefix;
    // can become d pointer if needed
    mutable KoUserStyle *m_lastStyle; ///< Last style that was searched
    bool m_default;
    bool m_unused1;
    bool m_unused2;
    bool m_unused3;
};


#endif /* KOUSERSTYLECOLLECTION_H */

