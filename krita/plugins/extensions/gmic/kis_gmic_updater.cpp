/*
 * Copyright (c) 2014 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_gmic_updater.h"

#include <QUrl>
#include <QFile>
#include <QNetworkRequest>
#include <qsslerror.h>
#include <QTimer>
#include <kis_debug.h>

#include <CImg.h>
#include <gmic.h>

#include <kglobal.h>
#include <kstandarddirs.h>

KisGmicUpdater::KisGmicUpdater(const QString &updateurl, QObject *parent): QObject(parent),m_url(updateurl)
{
    connect(&m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedDownload(QNetworkReply*)));
}

KisGmicUpdater::~KisGmicUpdater()
{

}

void KisGmicUpdater::start()
{
    QUrl url(m_url);
    QNetworkRequest request(url);

    QString userAgent("org.krita.gmic/");

    QString version = QString("%0.%1.%2.%3").arg(gmic_version/1000).arg((gmic_version/100)%10).arg((gmic_version/10)%10).arg(gmic_version%10);

    userAgent.append(version);
    dbgPlugins << "userAgent" << userAgent.toLatin1();

    request.setRawHeader("User-Agent", userAgent.toLatin1());

    QNetworkReply * getReply = m_manager.get(request);

    connect(getReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(reportProgress(qint64,qint64)));
    connect(getReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}


void KisGmicUpdater::finishedDownload(QNetworkReply*reply)
{
    if(reply->error() != QNetworkReply::NoError)
    {
        dbgPlugins << "NetworkReply error : " << reply->errorString();
    }

    dbgPlugins << "bytes available: " << reply->bytesAvailable();
    dbgPlugins << reply->url() << " finished";

    QString path = KGlobal::dirs()->saveLocation("gmic_definitions");
    QString fileName = reply->url().path().split("/").last();

    QByteArray data = reply->readAll();

    QString tmpfilePath = path + fileName + QString(".cimgz");
    QFile out(tmpfilePath);
    out.open(QIODevice::WriteOnly);
    out.write(data);
    out.close();

    QString filePathDst = path + fileName;

    std::FILE *file = std::fopen(tmpfilePath.toUtf8().constData(),"rb");
    cimg_library::CImg<unsigned char> buffer;
    buffer.load_cimg(file);
    buffer.save_raw(filePathDst.toUtf8().constData());
    std::fclose(file);

    if (!QFile::remove(tmpfilePath))
    {
        dbgPlugins << "Cannot delete " << tmpfilePath;
    }

    emit updated();
}

void KisGmicUpdater::reportProgress(qint64 arrived, qint64 total)
{
    dbgPlugins << "arrived: " << arrived << " / " << total;
}

void KisGmicUpdater::slotError(QNetworkReply::NetworkError error)
{
    dbgPlugins << "NetworkError" << error;
}


