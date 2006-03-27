/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef __KoApplicationIface_h__
#define __KoApplicationIface_h__

#include <dcopobject.h>
#include <q3valuelist.h>
#include <dcopref.h>

/**
 * DCOP interface for any KOffice application (entry point)
 */
class KoApplicationIface : public DCOPObject
{
  K_DCOP
public:

  KoApplicationIface();
  ~KoApplicationIface();

k_dcop:
  /**
   * Creates a new document for the given native mimetype
   * Use it to create a shell and to load an existing file, if any
   */
  DCOPRef createDocument( const QString &nativeFormat );

  /**
   * @return a list of references to all the documents
   * (see KoDocumentIface)
   */
  Q3ValueList<DCOPRef> getDocuments();

  /**
   * @return a list of references to all the views
   * (see KoViewIface)
   * Convenience method to avoid iterating over all documents to get all the views.
   */
  Q3ValueList<DCOPRef> getViews();

  /**
   * @return a list of references to all the windows
   * (see KoMainWindowIface)
   */
  Q3ValueList<DCOPRef> getWindows();
};

#endif

