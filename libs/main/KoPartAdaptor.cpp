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

#include "KoPartAdaptor.h"

#include <QList>

#include "KoDocument.h"
#include "KoPart.h"
#include "KoDocumentInfoDlg.h"
#include "KoDocumentInfo.h"
#include "KoView.h"
#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>


KoPartAdaptor::KoPartAdaptor(KoPart *doc)
        : QDBusAbstractAdaptor(doc)
{
    setAutoRelaySignals(true);
    m_pDoc = doc;
}

KoPartAdaptor::~KoPartAdaptor()
{
}

void KoPartAdaptor::openUrl(const QString & url)
{
    m_pDoc->openUrl(KUrl(url));
}

bool KoPartAdaptor::isLoading()
{
    return m_pDoc->document()->isLoading();
}

QString KoPartAdaptor::url()
{
    return m_pDoc->url().url();
}

bool KoPartAdaptor::isModified()
{
    return m_pDoc->isModified();
}

int KoPartAdaptor::viewCount()
{
    return m_pDoc->viewCount();
}

QString KoPartAdaptor::view(int idx)
{
    QList<KoView*> views = m_pDoc->views();
    KoView *v = views.at(idx);
    if (!v)
        return QString();

    return v->objectName();
}

QStringList KoPartAdaptor::actions()
{
    QStringList tmp_actions;
    QList<QAction*> lst = m_pDoc->actionCollection()->actions();
    foreach(QAction* it, lst) {
        if (it->isEnabled())
            tmp_actions.append(it->objectName());
    }
    return tmp_actions;
}

void KoPartAdaptor::save()
{
    m_pDoc->save();
}

void KoPartAdaptor::saveAs(const QString & url)
{
    m_pDoc->saveAs(KUrl(url));
    m_pDoc->waitSaveComplete(); // see ReadWritePart
}

void KoPartAdaptor::setOutputMimeType(const QByteArray& mimetype)
{
    m_pDoc->document()->setOutputMimeType(mimetype);
}

QString KoPartAdaptor::documentInfoAuthorName() const
{
    return m_pDoc->document()->documentInfo()->authorInfo("creator");
}

QString KoPartAdaptor::documentInfoEmail() const
{
    return m_pDoc->document()->documentInfo()->authorInfo("email");
}

QString KoPartAdaptor::documentInfoCompanyName() const
{
    return m_pDoc->document()->documentInfo()->authorInfo("company");
}

QString KoPartAdaptor::documentInfoTelephone() const
{
    kDebug(30003) << " Keep compatibility with calligra <= 1.3 : use documentInfoTelephoneWork";
    return documentInfoTelephoneWork();
}

QString KoPartAdaptor::documentInfoTelephoneWork() const
{
    return m_pDoc->document()->documentInfo()->authorInfo("telephone-work");
}

QString KoPartAdaptor::documentInfoTelephoneHome() const
{
    return m_pDoc->document()->documentInfo()->authorInfo("telephone-home");
}


QString KoPartAdaptor::documentInfoFax() const
{
    return m_pDoc->document()->documentInfo()->authorInfo("fax");

}
QString KoPartAdaptor::documentInfoCountry() const
{
    return m_pDoc->document()->documentInfo()->authorInfo("country");

}
QString KoPartAdaptor::documentInfoPostalCode() const
{
    return m_pDoc->document()->documentInfo()->authorInfo("postal-code");

}
QString KoPartAdaptor::documentInfoCity() const
{
    return m_pDoc->document()->documentInfo()->authorInfo("city");
}

QString KoPartAdaptor::documentInfoInitial() const
{
    return m_pDoc->document()->documentInfo()->authorInfo("initial");
}

QString KoPartAdaptor::documentInfoAuthorPostion() const
{
    return m_pDoc->document()->documentInfo()->authorInfo("position");
}

QString KoPartAdaptor::documentInfoStreet() const
{
    return m_pDoc->document()->documentInfo()->authorInfo("street");
}

QString KoPartAdaptor::documentInfoTitle() const
{
    return m_pDoc->document()->documentInfo()->aboutInfo("title");
}

QString KoPartAdaptor::documentInfoAbstract() const
{
    return m_pDoc->document()->documentInfo()->aboutInfo("comments");
}

QString KoPartAdaptor::documentInfoKeywords() const
{
    return m_pDoc->document()->documentInfo()->aboutInfo("keywords");
}

QString KoPartAdaptor::documentInfoSubject() const
{
    return m_pDoc->document()->documentInfo()->aboutInfo("subject");
}
void KoPartAdaptor::setDocumentInfoKeywords(const QString & text)
{
    m_pDoc->document()->documentInfo()->setAboutInfo("keywords", text);
}

void KoPartAdaptor::setDocumentInfoSubject(const QString & text)
{
    m_pDoc->document()->documentInfo()->setAboutInfo("subject", text);
}

void KoPartAdaptor::setDocumentInfoAuthorName(const QString & text)
{
    m_pDoc->document()->documentInfo()->setAuthorInfo("creator", text);
}

void KoPartAdaptor::setDocumentInfoEmail(const QString &text)
{
    m_pDoc->document()->documentInfo()->setAuthorInfo("email", text);
}

void KoPartAdaptor::setDocumentInfoCompanyName(const QString &text)
{
    m_pDoc->document()->documentInfo()->setAuthorInfo("company", text);
}

void KoPartAdaptor::setDocumentInfoAuthorPosition(const QString &text)
{
    m_pDoc->document()->documentInfo()->setAuthorInfo("position", text);
}


void KoPartAdaptor::setDocumentInfoTelephone(const QString &text)
{
    kDebug(30003) << "Keep compatibility with calligra <= 1.3 : use setDocumentInfoTelephoneWork";
    setDocumentInfoTelephoneWork(text);
}

void KoPartAdaptor::setDocumentInfoTelephoneWork(const QString &text)
{
    m_pDoc->document()->documentInfo()->setAuthorInfo("telephone-work", text);
}

void KoPartAdaptor::setDocumentInfoTelephoneHome(const QString &text)
{
    m_pDoc->document()->documentInfo()->setAuthorInfo("telephone", text);
}

void KoPartAdaptor::setDocumentInfoFax(const QString &text)
{
    m_pDoc->document()->documentInfo()->setAuthorInfo("fax", text);
}

void KoPartAdaptor::setDocumentInfoCountry(const QString &text)
{
    m_pDoc->document()->documentInfo()->setAuthorInfo("country", text);
}

void KoPartAdaptor::setDocumentInfoTitle(const QString & text)
{
    m_pDoc->document()->documentInfo()->setAboutInfo("title", text);
}

void KoPartAdaptor::setDocumentInfoPostalCode(const QString &text)
{
    m_pDoc->document()->documentInfo()->setAuthorInfo("postal-code", text);
}

void KoPartAdaptor::setDocumentInfoCity(const QString & text)
{
    m_pDoc->document()->documentInfo()->setAuthorInfo("city", text);
}

void KoPartAdaptor::setDocumentInfoInitial(const QString & text)
{
    m_pDoc->document()->documentInfo()->setAuthorInfo("initial", text);
}

void KoPartAdaptor::setDocumentInfoStreet(const QString &text)
{
    m_pDoc->document()->documentInfo()->setAuthorInfo("street", text);
}

void KoPartAdaptor::setDocumentInfoAbstract(const QString &text)
{
    m_pDoc->document()->documentInfo()->setAboutInfo("comments", text);
}


#include <KoPartAdaptor.moc>
