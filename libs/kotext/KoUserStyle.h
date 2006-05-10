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

#ifndef KOUSERSTYLE_H
#define KOUSERSTYLE_H

#include <koffice_export.h>
#include <QString>

/**
 * Base class for any kind of style that the user can create/modify/delete/display.
 * Use in conjunction with KoUserStyleCollection.
 *
 * For every style provided by the application itself and named "Foo", ensure that
 * i18n("Style name", "Foo") is available in a .cpp file, and create KoUserStyle("Foo").
 * The display name of the style will be set automatically to the translation of "Foo".
 */
class KOTEXT_EXPORT KoUserStyle
{
public:
    KoUserStyle( const QString & name );

    /// The internal name (used for loading/saving, but not shown to the user)
    /// Should be unique in a given style collection.
    QString name() const { return m_name; }
    /// Set the internal name - see generateUniqueName() if needed
    /// Should be unique in a given style collection.
    void setName( const QString & name ) { m_name = name; }

    /// The user-visible name (e.g. translated, or set by the user)
    QString displayName() const;
    /// Set the user-visible name
    void setDisplayName( const QString& name );

protected:
    QString m_name;
    QString m_displayName;
};

#endif
