/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2006 Fredrik Edemar <f_edemar@linux.se>

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

#ifndef __KoApplicationAdaptor_h__
#define __KoApplicationAdaptor_h__

#include <QtCore/QObject>
#include <dbus/qdbus.h>
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;


/**
 * D-BUS interface for any KOffice application (entry point)
 */
class KoApplicationAdaptor : public QDBusAbstractAdaptor
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.koffice.application")
public:
   KoApplicationAdaptor(QObject *parent);
   virtual ~KoApplicationAdaptor();

public Q_SLOTS: // METHODS
  /**
   * Creates a new document for the given native mimetype
   * Use it to create a shell and to load an existing file, if any
   */
  QString createDocument( const QString &nativeFormat );

  /**
   * @return a list of references to all the documents
   * (see KoDocumentIface)
   */
  QStringList getDocuments();

  /**
   * @return a list of references to all the views
   * (see KoViewIface)
   * Convenience method to avoid iterating over all documents to get all the views.
   */
  QStringList getViews();

  /**
   * @return a list of references to all the windows
   * (see KoMainWindowIface)
   */
  QStringList getWindows();
};

#endif

