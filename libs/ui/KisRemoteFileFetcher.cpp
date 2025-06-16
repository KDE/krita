/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "KisRemoteFileFetcher.h"

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QCheckBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <kis_config.h>

#include <klocalizedstring.h>

KisRemoteFileFetcher::KisRemoteFileFetcher(QObject *parent)
    : QObject(parent)
    , m_request(nullptr)
    , m_reply(nullptr)
{
}

KisRemoteFileFetcher::~KisRemoteFileFetcher()
{
    delete m_request;
    delete m_reply;
}

bool KisRemoteFileFetcher::fetchFile(const QUrl &remote, QIODevice *io)
{
    Q_ASSERT(!remote.isLocalFile());

    if (remote.scheme() != "data") {

        bool showMessage = KisConfig(true).readEntry("FetchingRemoteImage/ShowMessagebox", true);
        if (showMessage) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(i18nc("@title:window", "Krita"));
            msgBox.setIcon(QMessageBox::Question);
            msgBox.setText(i18nc("Fetching remote image",
                                 "Do you want to download the image from %1?\nClick \"Show Details\" to view the full link "
                                 "to the image.")
                               .arg(remote.host()));
            msgBox.setDetailedText(remote.toDisplayString());
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);

            QCheckBox *cb = new QCheckBox(i18n("Don't aks this again."));
            msgBox.setCheckBox(cb);

            const int res = msgBox.exec();

            KisConfig(false).writeEntry("FetchingRemoteImage/ShowMessagebox", cb->checkState() == Qt::CheckState::Unchecked);

            if (res != QMessageBox::Yes) {
                return false;
            }
        }
    }

    QNetworkAccessManager manager(this);
    m_request = new QNetworkRequest(remote);
    m_request->setRawHeader("User-Agent", QString("Krita-%1").arg(qApp->applicationVersion()).toUtf8());
    m_reply = manager.get(*m_request);

    QLocale loc;

    QProgressDialog progress;
    progress.setWindowTitle(i18nc("@title:window", "Krita"));
    progress.setLabelText(i18nc("Fetching remote image", "Downloading image from %1...").arg(remote.host()));
    progress.setMinimum(0);
    progress.setMaximum(0);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setWindowFlag(Qt::CustomizeWindowHint, true);
    progress.setWindowFlag(Qt::WindowCloseButtonHint, false);
    connect(m_reply, &QNetworkReply::finished, &progress, &QProgressDialog::accept);
    connect(m_reply, &QNetworkReply::errorOccurred, &progress, &QProgressDialog::cancel);
    connect(m_reply, &QNetworkReply::downloadProgress, &progress, [&](const int ist, const int max) {
        progress.setMaximum(max);
        progress.setValue(ist);
        progress.setLabelText(i18nc("Fetching remote image", "Downloading image from %1... (%2 / %3)")
                                  .arg(remote.host())
                                  .arg(loc.formattedDataSize(ist))
                                  .arg(loc.formattedDataSize(max)));
    });

    connect(&progress, &QProgressDialog::canceled, m_reply, &QNetworkReply::abort);

    progress.exec();

           // avoid double free on manager destruction
    m_reply->setParent(nullptr);

    if (m_reply->error() != QNetworkReply::NoError) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(i18nc("@title:window", "Krita"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(i18nc("Fetching remote image", "Could not download %1.").arg(remote.toDisplayString()));
        msgBox.setDetailedText(m_reply->errorString());
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return false;
    }

    if (!io->isOpen()) {
        io->open(QIODevice::WriteOnly);
    }
    io->write(m_reply->readAll());
    io->close();

    return true;
}

QByteArray KisRemoteFileFetcher::fetchFile(const QUrl &remote)
{
    QByteArray ba;
    QEventLoop loop;

    QNetworkAccessManager manager(nullptr);
    connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

    QNetworkRequest *request = new QNetworkRequest(remote);
    request->setRawHeader("User-Agent", QString("Krita-%1").arg(qApp->applicationVersion()).toUtf8());

    QNetworkReply *reply = manager.get(*request);

    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        ba = reply->readAll();
    }

    reply->setParent(nullptr);

    return ba;

}

void KisRemoteFileFetcher::error(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);

    qDebug() << "error" << m_reply->errorString();
}
