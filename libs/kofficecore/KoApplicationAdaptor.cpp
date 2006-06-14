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

#include "KoApplicationAdaptor.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "KoApplication.h"

#include "KoDocument.h"
#include "KoDocumentIface.h"
#include "KoMainWindow.h"
#include "KoQueryTrader.h"
#include "KoView.h"

KoApplicationAdaptor::KoApplicationAdaptor(QObject *parent)
   : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

KoApplicationAdaptor::~KoApplicationAdaptor()
{
    // destructor
}

QString KoApplicationAdaptor::createDocument( const QString &nativeFormat )
{
    KoDocumentEntry entry = KoDocumentEntry::queryByMimeType( nativeFormat );
    if ( entry.isEmpty() )
    {
        KMessageBox::questionYesNo( 0, i18n( "Unknown KOffice MimeType %s. Check your installation.", nativeFormat ) );
        return QString();
    }
    KoDocument* doc = entry.createDoc( 0 );
    if ( doc )
        return '/' + doc->objectName();
    else
        return QString();
}

QStringList KoApplicationAdaptor::getDocuments()
{
    QStringList lst;
    Q3PtrList<KoDocument> *documents = KoDocument::documentList();
    if ( documents )
    {
      Q3PtrListIterator<KoDocument> it( *documents );
      for (; it.current(); ++it )
        lst.append( '/' + it.current()->objectName() );
    }
    return lst;
}

QStringList KoApplicationAdaptor::getViews()
{
    QStringList lst;
    Q3PtrList<KoDocument> *documents = KoDocument::documentList();
    if ( documents )
    {
      Q3PtrListIterator<KoDocument> it( *documents );
      for (; it.current(); ++it )
      {
          foreach ( KoView* view, it.current()->views() )
              lst.append( '/' + view->objectName() );
      }
    }
    return lst;
}

QStringList KoApplicationAdaptor::getWindows()
{
    QStringList lst;
    QList<KMainWindow*> mainWindows = KMainWindow::memberList();
    if ( !mainWindows.isEmpty() )
    {
        foreach ( KMainWindow* mainWindow, mainWindows )
            lst.append( static_cast<KoMainWindow*>(mainWindow)->objectName() );
    }
    return lst;
}

#include "KoApplicationAdaptor.moc"
