/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KISUPDATERSTATUS_H
#define KISUPDATERSTATUS_H

#include <QObject>
#include <QString>
#include <QMetaType>

#include "kritaui_export.h"


namespace UpdaterStatus {

    enum class KRITAUI_EXPORT StatusID {
        UPTODATE,
        UPDATE_AVAILABLE,
        CHECK_ERROR,
        UPDATE_ERROR,
        IN_PROGRESS,
        RESTART_REQUIRED,
        INITIALIZED
    };

}

Q_DECLARE_METATYPE(UpdaterStatus::StatusID);


class KRITAUI_EXPORT KisUpdaterStatus : public QObject
{
    Q_OBJECT

public:
    KisUpdaterStatus();
    KisUpdaterStatus(const KisUpdaterStatus& rhs);
    ~KisUpdaterStatus();

    UpdaterStatus::StatusID status();
    QString availableVersion();
    QString downloadLink();
    QString updaterOutput();
    QString details();

    void setStatus(const UpdaterStatus::StatusID& status);
    void setAvailableVersion(const QString& availableVersion);
    void setDownloadLink(const QString& downloadLink);
    void setUpdaterOutput(const QString& updaterOutput);
    void setDetails(const QString& details);

    KisUpdaterStatus& operator=(KisUpdaterStatus& secondArg);
    bool operator==(KisUpdaterStatus& secondArg);

private:
    UpdaterStatus::StatusID m_status { UpdaterStatus::StatusID::INITIALIZED };
    QString m_availableVersion;
    QString m_downloadLink;
    QString m_updaterOutput;
    QString m_details;
};

Q_DECLARE_METATYPE(KisUpdaterStatus);

#endif // KISUPDATERSTATUS_H
