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


#ifndef KIS_GMIC_UPDATER
#define KIS_GMIC_UPDATER

#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <qsslerror.h>
#include <QList>

class KisGmicUpdater : public QObject
{
    Q_OBJECT
public:
    KisGmicUpdater(const QString &updateurl, QObject *parent = 0);
    virtual ~KisGmicUpdater();

    void start();

Q_SIGNALS:
    void updated();

private Q_SLOTS:
    void finishedDownload(QNetworkReply *);
    void reportProgress(qint64 arrived,qint64 total);
    void slotError(QNetworkReply::NetworkError error);

private:
    QNetworkAccessManager m_manager;
    QString m_url;

};

#endif // KIS_GMIC_UPDATER
