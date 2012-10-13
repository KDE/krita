/* This file is part of the KDE project
   Copyright (C) 2006-2012 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <db/utils.h>
#include "sqlitevacuum.h"

#include <kstandarddirs.h>
#include <kprogressdialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qprogressbar.h>
#include <kio/global.h>
#include <kde_file.h>

#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QCursor>
#include <QTemporaryFile>

#include <unistd.h>

SQLiteVacuum::SQLiteVacuum(const QString& filePath)
        : m_filePath(filePath)
{
    m_dumpProcess = 0;
    m_sqliteProcess = 0;
    m_percent = 0;
    m_dlg = 0;
    m_result = true;
}

SQLiteVacuum::~SQLiteVacuum()
{
    if (m_dumpProcess) {
        m_dumpProcess->waitForFinished();
        delete m_dumpProcess;
    }
    if (m_sqliteProcess) {
        m_sqliteProcess->waitForFinished();
        delete m_sqliteProcess;
    }
    if (m_dlg)
        m_dlg->close();
    delete m_dlg;
    QFile::remove(m_tmpFilePath);
}

tristate SQLiteVacuum::run()
{
    const QString dump_app = KStandardDirs::findExe("kexi_sqlite3_dump");
    kDebug() << dump_app;
    if (dump_app.isEmpty()) {
        KexiDBDrvWarn << "Could not find tool" << "kexi_sqlite3_dump";
        m_result = false;
        return m_result;
    }

    const QString sqlite_app = KexiDB::sqlite3ProgramPath();
    kDebug() << sqlite_app;
    if (sqlite_app.isEmpty()) {
        m_result = false;
        return m_result;
    }

    QFileInfo fi(m_filePath);
    if (!fi.isReadable()) {
        KexiDBDrvWarn << "No readable file" << m_filePath;
        return false;
    }

    kDebug() << fi.absoluteFilePath() << fi.absoluteDir().path();

    delete m_dumpProcess;
    m_dumpProcess = new QProcess(this);
    m_dumpProcess->setWorkingDirectory(fi.absoluteDir().path());
    m_dumpProcess->setReadChannel(QProcess::StandardError);
    connect(m_dumpProcess, SIGNAL(readyReadStandardError()), this, SLOT(readFromStdErr()));
    connect(m_dumpProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(dumpProcessFinished(int,QProcess::ExitStatus)));
            
    delete m_sqliteProcess;
    m_sqliteProcess = new QProcess(this);
    m_sqliteProcess->setWorkingDirectory(fi.absoluteDir().path());
    connect(m_sqliteProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(sqliteProcessFinished(int,QProcess::ExitStatus)));

    m_dumpProcess->setStandardOutputProcess(m_sqliteProcess);
    m_dumpProcess->start(dump_app, QStringList() << fi.absoluteFilePath());
    if (!m_dumpProcess->waitForStarted()) {
        delete m_dumpProcess;
        m_dumpProcess = 0;
        m_result = false;
        return m_result;
    }
    
    QTemporaryFile *tempFile = new QTemporaryFile(fi.absoluteFilePath());
    tempFile->open();
    m_tmpFilePath = tempFile->fileName();
    delete tempFile;
    kDebug() << m_tmpFilePath;
    m_sqliteProcess->start(sqlite_app, QStringList() << m_tmpFilePath);
    if (!m_sqliteProcess->waitForStarted()) {
        delete m_dumpProcess;
        m_dumpProcess = 0;
        delete m_sqliteProcess;
        m_sqliteProcess = 0;
        m_result = false;
        return m_result;
    }
    
    m_dlg = new KProgressDialog(0, i18n("Compacting database"),
                                "<qt>" + i18n("Compacting database \"%1\"...",
                                              "<nobr>" + QDir::convertSeparators(fi.fileName()) + "</nobr>")
                               );
    m_dlg->adjustSize();
    m_dlg->resize(300, m_dlg->height());
    connect(m_dlg, SIGNAL(cancelClicked()), this, SLOT(cancelClicked()));
    m_dlg->setMinimumDuration(1000);
    m_dlg->setAutoClose(true);
    m_dlg->progressBar()->setRange(0, 100);
    m_dlg->exec();
    while (m_dumpProcess->state() == QProcess::Running
           && m_sqliteProcess->state()  == QProcess::Running)
    {
        readFromStdErr();
        qApp->processEvents(QEventLoop::AllEvents, 50000);
        //kDebug() << "READ..." << ~m_result;
    }

    readFromStdErr();

    return m_result;
}

void SQLiteVacuum::readFromStdErr()
{
    while (true) {
        QString s(m_dumpProcess->readLine(1000));
        if (s.isEmpty())
            break;
        kDebug() << s;
//  KexiDBDrvDbg << m_percent << " " << s;
        if (s.startsWith("DUMP: ")) {
            //set previously known progress
            m_dlg->progressBar()->setValue(m_percent);
            //update progress info
            if (s.mid(6, 4) == "100%") {
                m_percent = 100;
                m_dlg->setAllowCancel(false);
                m_dlg->setCursor(QCursor(Qt::WaitCursor));
            } else if (s.mid(7, 1) == "%") {
                m_percent = s.mid(6, 1).toInt();
            } else if (s.mid(8, 1) == "%") {
                m_percent = s.mid(6, 2).toInt();
            }
            m_dlg->progressBar()->setValue(m_percent);
        }
    }
}

void SQLiteVacuum::dumpProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    KexiDBDrvDbg << exitCode << exitStatus;
    if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
        cancelClicked();
        m_result = false;
    }

    if (m_dlg) {
        m_dlg->close();
        delete m_dlg;
        m_dlg = 0;
    }

    if (true != m_result) {
        return;
    }

    QFileInfo fi(m_filePath);
    const uint origSize = fi.size();

    if (0 != KDE::rename(m_tmpFilePath, fi.absoluteFilePath())) {
        kWarning() << "Rename" << m_tmpFilePath << "to" << fi.absoluteFilePath() << "failed.";
        m_result = false;
    }

    if (m_result == true) {
        const uint newSize = fi.size();
        const uint decrease = 100 - 100 * newSize / origSize;
        KMessageBox::information(0, i18n("The database has been compacted. Current size decreased by %1% to %2.", decrease, KIO::convertSize(newSize)));
    }
}

void SQLiteVacuum::sqliteProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    KexiDBDrvDbg << exitCode << exitStatus;

    if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
        m_result = false;
        return;
    }
}

void SQLiteVacuum::cancelClicked()
{
        //m_dumpProcess->terminate();
        m_sqliteProcess->terminate();
        m_result = cancelled;
        QFile::remove(m_tmpFilePath);
}

#include "sqlitevacuum.moc"
