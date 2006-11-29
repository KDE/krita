/* This file is part of the KDE project
   Copyright (C) 2002 David Faure <faure@kde.org>

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

#ifndef koStoreDrag_h
#define koStoreDrag_h

#include <q3dragobject.h>
//Added by qt3to4:
#include <QByteArray>
#include <koffice_export.h>
/**
 * A generic drag object that holds a store (e.g. KoZipStore) in memory.
 * This allows to drag-n-drop and copy-paste complex koffice objects.
 * As per usual with dragobjects, an instance of KoStoreDrag must be
 * created on the "sending" side (dragging or copying). The "receiving"
 * side (dropping or pasting) only uses provides()/canDecode() and encodedData().
 *
 * To create the data in memory, create a QBuffer,
 * then KoStore::createStore( theBuffer, .... ), save the
 * data into the store and delete it. Finally, call setEncodedData().
 */
class KSTORE_EXPORT KoStoreDrag : public Q3StoredDrag
{
public:
    /** Constructor.
     * @param nativeMimeType the app's native mimetype.
     * @param dragSource must be 0 when copying to the clipboard.
     * @param name object name for this drag.
     */
    explicit KoStoreDrag( const char* nativeMimeType, QWidget *dragSource = 0L, const char *name = 0L );

    static bool canDecode( const char* nativeMimeType, QMimeSource* e );

    /**
     * Returns the mimetype of the clipboard data for a given application,
     * depending on the application's native mimetype.
     */
    static QByteArray mimeType( const char* nativeMimeType );
};

#endif
