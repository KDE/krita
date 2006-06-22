/*
 *  Copyright (c) 2006 Gábor Lehel <illissius@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KO_DOCUMENT_SECTION_MODEL_H
#define KO_DOCUMENT_SECTION_MODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QList>
#include <QString>
#include <QVariant>

/** Inherit from this class for use with @see KoDocumentSectionView. */

class KoDocumentSectionModel: public QAbstractItemModel
{
    public:

    KoDocumentSectionModel( QObject *parent = 0 ): QAbstractItemModel( parent ) { }

    /// Extensions to Qt::ItemDataRole.
    enum ItemDataRole
    {
        /// A thumbnail for displaying in a list. Probably 64x64.
        ThumbnailRole = 33,

        /// A larger thumbnail for displaying in a tooltip. 200x200 or so.
        LargeThumbnailRole,

        /// A list of properties the part has.
        PropertiesRole
    };

    /// describes a single property of a document partition
    struct Property
    {
        /** i18n-ed name, suitable for displaying */
        QString name;

        /** Whether the property is a boolean which can be toggled directly from widget itself. */
        bool isMutable;

        /** Provide these if the property isMutable. */
        QIcon onIcon;
        QIcon offIcon;

        /** If the property isMutable, provide a boolean. Otherwise, a string suitable for displaying. */
        QVariant state;

        /// Default constructor. Use if you want to assign the members manually.
        Property(): isMutable( false ) { }

        /// Constructor for a mutable property.
        Property( const QString &n, const QIcon &on, const QIcon &off, bool isOn )
            : name( n ), isMutable( true ), onIcon( on ), offIcon( off ), state( isOn ) { }

        /// Constructor for a nonmutable property.
        Property( const QString &n, const QString &s )
            : name( n ), isMutable( false ), state( s ) { }
    };

    /** Return this type for PropertiesRole. */
    typedef QList<Property> PropertyList;
};

Q_DECLARE_METATYPE( KoDocumentSectionModel::PropertyList )

#endif
