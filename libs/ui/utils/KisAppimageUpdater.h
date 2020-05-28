/*
 *  Copyright (c) 2019 Anna Medonosova <anna.medonosova@gmail.com>
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


#ifndef KISAPPIMAGEUPDATER_H
#define KISAPPIMAGEUPDATER_H

#include <KisUpdaterBase.h>
#include <QScopedPointer>
#include <QProcess>

#include <kritagitversion.h>

#include "kritaui_export.h"


class QString;

class KRITAUI_EXPORT KisAppimageUpdater : public KisUpdaterBase
{
    Q_OBJECT

public:
    KisAppimageUpdater();
#ifdef KRITA_GIT_SHA1_STRING
    explicit KisAppimageUpdater(QString dummyUpdaterPath);
#endif

    void checkForUpdate() override;
    bool hasUpdateCapability() override;
    void doUpdate() override;

private Q_SLOTS:
    void slotUpdateCheckFinished(int result, QProcess::ExitStatus exitStatus);
    void slotUpdateCheckStarted();
    void slotUpdateCheckErrorOccurred(QProcess::ProcessError error);

    void slotUpdateFinished(int result, QProcess::ExitStatus exitStatus);
    void slotUpdateErrorOccurred(QProcess::ProcessError error);

    void slotAppendCheckOutput();
    void slotAppendUpdateOutput();

private:
    void initialize(QString& updaterPath);

    bool findUpdaterBinary();
    QString m_updaterBinary;
    QString m_appimagePath;
    bool m_updateCapability;
    bool m_updaterInProgress {false};
    QString m_checkOutput;
    QString m_updateOutput;

    QScopedPointer<QProcess> m_checkProcess;
    QScopedPointer<QProcess> m_updateProcess;
};

#endif // KISAPPIMAGEUPDATER_H
