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

#ifndef SQLITE_VACUUM_H
#define SQLITE_VACUUM_H

#include <QObject>
#include <QString>
#include <QProcess>

#include <db/tristate.h>

class KProgressDialog;

//! @short Helper class performing interactive compacting (VACUUM) of the SQLite database
/*! Proved SQLite database filename in the constructor.
 Then execute run() should be executed.

 KProgressDialog will be displayed. Its progress bar will be updated whenever another
 table's data compacting is performed. User can click "Cancel" button in any time
 (except the final committing) to cancel the operation. In this case,
 it's guaranteed that the original file remains unchanged.

 This is possible because we rely on SQLite's VACUUM SQL command, which itself temporarily
 creates a copy of the original database file, and replaces the orginal with the new only
 on success.
*/
class SQLiteVacuum : public QObject
{
    Q_OBJECT
public:
    SQLiteVacuum(const QString& filePath);
    ~SQLiteVacuum();

    /*! Performs compacting procedure.
     \return true on success, false on failure and cancelled if user
     clicked "Cancel" button in the progress dialog. */
    tristate run();

public slots:
    void readFromStdErr();
    void dumpProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void sqliteProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void cancelClicked();

protected:
    QString m_filePath;
    QString m_tmpFilePath;
    QProcess *m_dumpProcess;
    QProcess *m_sqliteProcess;
    KProgressDialog* m_dlg;
    int m_percent;
    tristate m_result;
};

#endif
