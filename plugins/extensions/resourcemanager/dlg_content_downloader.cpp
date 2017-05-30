/*
 *  Copyright (c) 2016 Aniketh Girish anikethgireesh@gmail.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <KNSCore/EntryInternal>
#include <KNSCore/DownloadManager>
#include <KNSCore/Engine>
#include <KNS3/Entry>
#include <KNS3/DownloadWidget>

#include "dlg_content_downloader.h"

class KNSResourceDownloader::Private
{
public:
    Private() : {}
    const QString knsrc;
    KoResourceServer m_resourcesServer;
};

KNSResourceDownloader::KNSResourceDownloader(const KNSCore::EntryInternal& entry, const KNSCore::Author& author, QStringList categories, QObject* parent)
    : KoResourceServer(parent)
    , m_categories(std::move(categories))
    , m_author(author)
    , m_entry(entry)
    , d( new Private(this) )
{
    const QString fileName = QFileInfo(d->knsrc).fileName();
    setName(fileName);
    setObjectName(d->knsrc);

    m_categories = QStringList{ fileName };
}

KNSResourceDownloader::~KNSResourceDownloader()
{
    delete d;
}

//Need to see which is need the name or the packageName.

QString KNSResourceDownloader::name()
{
    return m_entry.name();
}

QString KNSResourceDownloader::packagename()
{
    return m_entry.uniqueId();
}

QString KNSResourceDownloader::author()
{
    return m_author.name();
}

QStringList KNSResourceDownloader::categories()
{
    return m_categories;
}

int KNSResourceDownloader::downloadcount()
{
    const auto downloadInfo = m_entry.downloadLinkInformationList();
    return downloadInfo.isEmpty() ? 0 : downloadInfo.at(0).size;
}

QString KNSResourceDownloader::source()
{
    return m_entry.providerId();
}

QString KNSResourceDownloader::license()
{
    return m_entry.license();
}

QString KNSResourceDownloader::details()
{
    QString ret = m_entry.summary();
    if (m_entry.shortSummary().isEmpty()) { //ret.isEmpty() ??
        const int newLine = ret.indexOf(QLatin1Char('\n'));
        if (newLine < 0)
            ret.clear();
        else
            ret = ret.mid(newLine+1).trimmed();
    }
    ret = ret.replace(QLatin1Char('\r'), QString());
    ret = ret.replace(QStringLiteral("[li]"), QStringLiteral("\n* "));
    ret = ret.replace(QRegularExpression(QStringLiteral("\\[/?[a-z]*\\]")), QString());
    return ret;
}

void KNSResourceDownloader::downloadResources(KoResourceServer *app)
{
    KNS3::DownloadWidget dialog(d->knsrc, this);
    dialog.exec();

    foreach (const KNS3::Entry& e, dialog.changedEntries()) {

        foreach (const QString &file, e.installedFiles()) {
            QFileInfo fi(file);
            d->m_resourcesServer.importResourceFile(fi.absolutePath()+'/'+fi.fileName() , false); //need to rewrite according to MVC.
        }

        foreach (const QString &file, e.uninstalledFiles()) {
            QFileInfo fi(file);
            d->m_resourcesServer.removeResourceFile(fi.absolutePath()+'/'+fi.fileName()); //need to rewrite according to MVC.
        }

   }


}

void KNSResourceDownloader::setKnsrcFile(const QString &knsrcFileArg)
{
    d->knsrc = knsrcFileArg;
}


KoResourceServer * KNSResourceDownloader::koresourceserver() const
{
    return qobject_cast<KoResourceServer*>(parent());
}

KNSCore::EntryInternal KNSResourceDownloader::entry() const
{
    return m_entry;
}
