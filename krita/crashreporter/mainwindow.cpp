/*
 *  Copyright (c) 2008-2009 Hyves (Startphone Ltd.)
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
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
 */
#include "mainwindow.h"

#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include <kglobal.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kstandarddirs.h>

#include <calligraversion.h>
#include <calligragitversion.h>

#include <cstdlib>

#include <KoConfig.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

//#ifdef Q_WS_WIN
//#include <cstring>
//#include <windows.h>
//#include <shellapi.h>
///**
// * Native Win32 method for starting a process. This is required in order to
// * launch the installer with User Account Control enabled.
// *
// * @param path Path to the process to start.
// * @param parameters Parameters for the process.
// * @return @c true if the process is started successfully, @c false otherwise.
// */
//bool startProcess(LPCWSTR path, char *parameters = 0) {

//    Q_ASSERT(path != 0);

//    SHELLEXECUTEINFO info;
//    memset(&info, '\0', sizeof(info));

//    info.cbSize = sizeof(info);
//    info.fMask = 0;
//    info.hwnd = 0;
//    info.lpVerb = TEXT("open");
//    info.lpFile = path;
//    info.lpParameters = (LPCWSTR)parameters;
//    info.lpDirectory = 0;
//    info.nShow = SW_SHOWNORMAL;
//    return ShellExecuteEx(&info);
//}

//void doRestart(bool resetConfig)
//{
//    if (!startProcess(QString("krita").toStdWString().data())) {
//        QMessageBox::warning(0, "Krita",
//                             i18n("Could not restart %1. Please try to restart %1 manually.").arg("krita"));
//    }
//}

//#else // Q_WS_WIN

void doRestart(MainWindow* mainWindow, bool resetConfig)
{
    if (resetConfig) {
        {
            QString appdata = qgetenv("APPDATA");
            QDir inputDir(appdata + "/krita/share/apps/krita/input/");
            foreach(QString entry, inputDir.entryList(QStringList("*.profile"))) {
                inputDir.remove(entry);
            }
            QDir configDir(appdata + "/krita/share/config/");
            configDir.remove("kritarc");
        }
        {
            QString appdata = qgetenv("LOCALAPPDATA");
            QDir inputDir(appdata + "/krita/share/apps/krita/input/");
            foreach(QString entry, inputDir.entryList(QStringList("*.profile"))) {
                inputDir.remove(entry);
            }
            QDir configDir(appdata + "/krita/share/config/");
            configDir.remove("kritarc");
        }
        {
            QDir inputDir(KGlobal::dirs()->saveLocation("appdata", "input/"));
            foreach(QString entry, inputDir.entryList(QStringList("*.profile"))) {
                inputDir.remove(entry);
            }
            QDir configDir(KGlobal::dirs()->saveLocation("config"));
            configDir.remove("kritarc");
        }
    }

    QString restartCommand;

#ifdef Q_WS_MAC
    QDir bundleDir(qApp->applicationDirPath());

    bundleDir.cdUp();
    bundleDir.cdUp();
    bundleDir.cdUp();

    restartCommand = QString("open \"") + QString(bundleDir.absolutePath() + "/krita.app\"");
#endif

#ifdef Q_WS_WIN
    restartCommand = qApp->applicationDirPath().replace(' ', "\\ ") + "/krita.exe \"";
#endif

#ifdef Q_WS_X11
    restartCommand = "sh -c \"" + qApp->applicationDirPath().replace(' ', "\\ ") + "/krita \"";
#endif

    qDebug() << "restartCommand" << restartCommand;
    QProcess restartProcess;
    if (!restartProcess.startDetached(restartCommand)) {
        QMessageBox::warning(mainWindow, "krita",
                             i18n("Could not restart Krita. Please try to restart manually."));
    }
}
//#endif  // Q_WS_WIN

#ifdef Q_WS_MAC
QString platformToStringMac(QSysInfo::MacVersion version)
{
    switch(version) {
    case QSysInfo::MV_9:
        return "MacOS 9";
    case QSysInfo::MV_10_0:
        return "OSX 10.0 (cheetah)";
    case QSysInfo::MV_10_1:
        return "OSX 10.1 (puma)";
    case QSysInfo::MV_10_2:
        return "OSX 10.2 (jaguar)";
    case QSysInfo::MV_10_3:
        return "OSX 10.3 (panther)";
    case QSysInfo::MV_10_4:
        return "OSX 10.4 (tiger)";
    case QSysInfo::MV_10_5:
        return "OSX 10.5 (leopard)";
    case QSysInfo::MV_10_6:
        return "OSX 10.6 (snow leopard)";
    case QSysInfo::MV_10_7:
        return "OSX 10.6 (Lion)";
    case QSysInfo::MV_10_8:
        return "OSX 10.6 (Mountain Lion)";
    case QSysInfo::MV_Unknown:
    default:
        return "Unknown OSX version";
    };
}
#endif

#ifdef Q_WS_WIN
QString platformToStringWin(QSysInfo::WinVersion version)
{
    switch(version) {
    case QSysInfo::WV_32s:
        return "Windows 3.1 with win32s";
    case QSysInfo::WV_95:
        return "Windows 95";
    case QSysInfo::WV_98:
        return "Windows 98";
    case QSysInfo::WV_Me:
        return "Windows Me";
    case QSysInfo::WV_NT:
        return "Windows NT";
    case QSysInfo::WV_2000:
        return "Windows 2000";
    case QSysInfo::WV_XP:
        return "Windows XP";
    case QSysInfo::WV_2003:
        return "Windows 2003";
    case QSysInfo::WV_VISTA:
        return "Windows Vista";
    case QSysInfo::WV_WINDOWS7:
        return "Windows 7";
    case QSysInfo::WV_WINDOWS8:
        return "Windows 8";
#if QT_VERSION >= 0x040806
    case QSysInfo::WV_WINDOWS8_1:
        return "Windows 8.1";
#endif
    default:
        return "Unknown Windows version";
    };

}
#endif

struct MainWindow::Private {
    QString dumpPath;
    QString id;
    QNetworkAccessManager *networkAccessManager;

    bool doRestart;
    bool uploadStarted;

    Private() :
        doRestart(true),
        uploadStarted(false) {
    }
};


MainWindow::MainWindow(const QString &dumpPath, const QString &id, QWidget *parent)
    : QWidget(parent)
    , m_d(new Private())
{
    setupUi(this);
    progressBar->hide();

    lblKiki->setPixmap(QPixmap(KGlobal::dirs()->findResource("data", "krita/pics/KikiNurse_sm.png")));

    setWindowFlags(Qt::WindowStaysOnTopHint | windowFlags());

    m_d->networkAccessManager = new QNetworkAccessManager(this);
    connect(m_d->networkAccessManager, SIGNAL(finished(QNetworkReply *)), SLOT(uploadDone(QNetworkReply *)));

    connect(bnRestart, SIGNAL(clicked()), this, SLOT(restart()));
    connect(bnClose, SIGNAL(clicked()), this, SLOT(close()));

    m_d->dumpPath = dumpPath;
    m_d->id = id;
}

MainWindow::~MainWindow()
{
    delete m_d;
}

void MainWindow::restart()
{
    m_d->doRestart = true;
    if (chkAllowUpload->isChecked()) {
        startUpload();
    }
    else {
        doRestart(this, chkRemoveSettings->isChecked());
        qApp->quit();
    }
}

void MainWindow::close()
{
    m_d->doRestart = false;
    if (!m_d->uploadStarted && chkAllowUpload->isChecked()) {
        startUpload();
    }
    else {
        qApp->quit();
    }
}

void MainWindow::startUpload()
{
    bnRestart->setEnabled(false);
    bnClose->setEnabled(false);
    progressBar->show();

    m_d->uploadStarted = true;

    // Upload minidump
    QNetworkRequest request(QUrl("http://krita-breakpad.kogmbh.net:1127/post"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=9876543210");

    QString boundary = "--9876543210";

    typedef QPair<QByteArray, QByteArray> Field;
    QList<Field> fields;

    QString calligraVersion(CALLIGRA_VERSION_STRING);
    QString version;


#ifdef CALLIGRA_GIT_SHA1_STRING
    QString gitVersion(CALLIGRA_GIT_SHA1_STRING);
    version = QString("%1-%2").arg(calligraVersion).arg(gitVersion).toLatin1();
#else
    version = calligraVersion;
#endif

    fields << Field("BuildID", CALLIGRA_GIT_SHA1_STRING)
           << Field("ProductName", "krita")
           << Field("Version", version.toLatin1())
           << Field("Vendor", "KO GmbH")
           << Field("Email", txtEmail->text().toLatin1())
           << Field("timestamp", QByteArray::number(QDateTime::currentDateTime().toTime_t()));

#ifdef Q_WS_WIN

    QString platform = platformToStringWin(QSysInfo::WindowsVersion);

#ifdef ENV32BIT
    platform += "_x86";
#else
    platform += "_x64";
#endif

    fields << Field("Platform", platform.toLatin1());
#endif
#ifdef Q_WS_X11
    fields << Field("Platform", "Linux/X11");
#endif
#ifdef Q_WS_MAC
    fields << Field("Platform", platformToStringMac(QSysInfo::MacintoshVersion).toLatin1());
#endif

    QFile f(QDesktopServices::storageLocation(QDesktopServices::TempLocation) + "/krita-opengl.txt");
    qDebug() << KGlobal::dirs()->saveLocation("config") + "/krita-opengl.txt" << f.exists();

    if (f.exists()) {
        f.open(QFile::ReadOnly);
        fields << Field("OpenGL", f.readAll());
    }

    QString body;
    foreach(Field const field, fields) {
        body += boundary + "\r\n";
        body += "Content-Disposition: form-data; name=\"" + field.first + "\"\r\n\r\n";
        body += field.second + "\r\n";
    }
    body += boundary + "\r\n";

    // add minidump file
    QString dumpfile = m_d->dumpPath + "/" + m_d->id + ".dmp";
    qDebug() << "dumpfile" << dumpfile;
    body += "Content-Disposition: form-data; name=\"upload_file_minidump\"; filename=\""
            + QFileInfo(dumpfile).fileName().toLatin1() + "\"\r\n";
    body += "Content-Type: application/octet-stream\r\n\r\n";
    QFile file(dumpfile);
    if (file.exists()) {
        file.open(QFile::ReadOnly);
        QByteArray ba = file.readAll();
        body += ba.toBase64();
        file.remove();
    }
    body += "\r\n";

    // add description
    body += boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"description\"\r\n";
    body += "\r\n";
    body +=	txtDescription->toPlainText();

    body += "\r\n";
    body += boundary + "--" + "\r\n";

    QFile report(QDir::homePath() + "/krita-" + m_d->id + ".report");
    report.open(QFile::WriteOnly);
    report.write(body.toLatin1());
    report.close();

    QNetworkReply *reply = m_d->networkAccessManager->post(request, body.toLatin1());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(uploadError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(uploadProgress(qint64,qint64)));
}

void MainWindow::uploadDone(QNetworkReply *reply)
{
    qDebug() << "updloadDone";
    if (reply && reply->error() != QNetworkReply::NoError) {
        qCritical() << "uploadDone: Error uploading crash report: " << reply->errorString();
    }
    if (m_d->doRestart) {
        doRestart(this, chkRemoveSettings->isChecked());
    }
    qApp->quit();

}

void MainWindow::uploadProgress(qint64 received, qint64 total)
{
    qDebug() << "updloadProgress";
    progressBar->setMaximum(total);
    progressBar->setValue(received);
}

void MainWindow::uploadError(QNetworkReply::NetworkError error)
{
    qDebug() << "updloadError" << error;
    // Fake success...
    progressBar->setRange(0, 100);
    progressBar->setValue(100);
    qCritical() << "UploadError: Error uploading crash report: " << error;

    uploadDone(0);
}
