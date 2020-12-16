/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    explicit KisAppimageUpdater(QString dummyUpdaterPath);

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
