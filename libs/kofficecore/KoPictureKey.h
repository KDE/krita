/* This file is part of the KDE project
   Copyright (c) 2001 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002, 2004 Nicolas GOUTTE <goutte@kde.org>

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
#ifndef __koPictureKey_h__
#define __koPictureKey_h__

#include <QString>
#include <QDateTime>
#include <koffice_export.h>
/**
 * \file koPictureKey.h
 * \todo correct documentation (for example: sed "s/image/picture/g")
 */

class QDomElement;

namespace KoPictureType
{
    /**
     * QPicture version used by KoPictureClipart
     *
     * Possible values:
     * \li 3 for Qt 2.1.x and later Qt 2.x
     * \li 4 for Qt 3.0
     * \li 5 for Qt 3.1 and later Qt 3.x
     * \li -1 for current Qt
     */
    const int formatVersionQPicture=-1;

    enum Type
    {
        TypeUnknown = 0,    ///< Unknown or not-an-image @see KoPictureBase
        TypeImage,          ///< Image, QImage-based @see KoPictureImage
        TypeEps,            ///< Encapsulated Postscript @see KoPictureEps
        TypeClipart,        ///< Clipart, QPicture-based @see KoPictureClipart
        TypeWmf             ///< WMF (Windows Meta File) @see KoPictureWmf
    };
}

/**
 * KoPictureKey is the structure describing a picture in a unique way.
 * It currently includes the original path to the picture and the modification
 * date.
 *
 * @short Structure describing a picture on disk
 *
 * @note We use the *nix epoch (1970-01-01) as a time base because it is a valid date.
 * That way we do not depend on a behaviour of the current QDateTime that might change in future versions of Qt
 * and we are also nice to non-Qt programs wanting to read KOffice's files.
 *
 * @note This behaviour is also needed for re-saving KWord files having \<FORMAT id="2"\>. When saving again,
 * these files get a \<KEY\> element as child of \<PIXMAPS\> but not one as child of \<FORMAT\> and \<IMAGE\>.
 * Therefore we need to be careful that the key remains compatible to default values 
 * (another good reason for the *NIX epoch)
 *
 * @note In case of a remote path, the "original path" is the name of the temporary file that was
 *  used to download the file.
 */
class KOFFICEUI_EXPORT KoPictureKey
{
public:
    /**
     * Default constructor. Creates a null key
     */
    KoPictureKey();

    /**
     * @brief Constructs a key, from a filename and a modification date
     *
     * Storing the modification date as part of the key allows the user
     * to update the file and import it into the application again, without
     * the application reusing the old copy from the collection.
     */
    KoPictureKey( const QString &fn, const QDateTime &mod );

    /**
     * Constructs a key from a filename 
     * @note The modification date is set to 1970-01-01
     */
    KoPictureKey( const QString &fn );

    /**
     * Copy constructor
     */
    KoPictureKey( const KoPictureKey &key );

    /**
     * Assignment operator
     */
    KoPictureKey &operator=( const KoPictureKey &key );

    /**
     * Comparison operator
     */
    bool operator==( const KoPictureKey &key ) const;

    /**
     * Comparison operator 
     * @note Used for sorting in the collection's map
     */
    bool operator<( const KoPictureKey &key ) const;

    /**
     * Convert this key into a string representation of it
     */
    QString toString() const;

    /**
     * Save this key in XML (as %KOffice 1.3)
     */
    void saveAttributes( QDomElement &elem ) const;

    /**
     * Load this key from XML (as %KOffice 1.3)
     */
    void loadAttributes( const QDomElement &elem );

    /**
     * First part of the key: the filename
     */
    QString filename() const { return m_filename; }

    /**
     * Second part of the key: the modification date
     */
    QDateTime lastModified() const { return m_lastModified; }

    /**
     * Sets the key according to @p filename, including modification time
     */
    void setKeyFromFile (const QString& filename);

protected:
    QString m_filename;
    QDateTime m_lastModified;
};

#endif /* __koPictureKey_h__ */
