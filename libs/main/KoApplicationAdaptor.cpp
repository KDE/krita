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

#include "KoApplication.h"
#include "KoPart.h"
#include "KoDocument.h"
#include "KoMainWindow.h"
#include "KoDocumentEntry.h"
#include "KoView.h"
#include "KoPart.h"

KoApplicationAdaptor::KoApplicationAdaptor(KoApplication *parent)
        : QDBusAbstractAdaptor(parent)
        , m_application(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

KoApplicationAdaptor::~KoApplicationAdaptor()
{
    // destructor
}

QString KoApplicationAdaptor::createDocument(const QString &nativeFormat)
{
    KoDocumentEntry entry = KoDocumentEntry::queryByMimeType(nativeFormat);
    if (entry.isEmpty()) {
        KMessageBox::questionYesNo(0, i18n("Unknown Calligra MimeType %1. Check your installation.", nativeFormat));
        return QString();
    }
    KoPart *part = entry.createKoPart(0);
    if (part) {
        m_application->addPart(part);
        return '/' + part->document()->objectName();
    }
    else {
        return QString();
    }
}

QStringList KoApplicationAdaptor::getDocuments()
{
    QStringList lst;
    QList<KoPart*> parts = m_application->partList();
    foreach(KoPart *part, parts) {
        lst.append('/' + part->document()->objectName());
    }
    return lst;
}

QStringList KoApplicationAdaptor::getViews()
{
    QStringList lst;
    QList<KoPart*> parts = m_application->partList();
    foreach(KoPart *part, parts) {
        foreach(KoView* view, part->views()) {
            lst.append('/' + view->objectName());
        }
    }

    return lst;
}

QStringList KoApplicationAdaptor::getWindows()
{
    QStringList lst;
    QList<KMainWindow*> mainWindows = KMainWindow::memberList();
    if (!mainWindows.isEmpty()) {
        foreach(KMainWindow* mainWindow, mainWindows) {
            lst.append(static_cast<KoMainWindow*>(mainWindow)->objectName());
        }
    }
    return lst;
}

#include <KoApplicationAdaptor.moc>
