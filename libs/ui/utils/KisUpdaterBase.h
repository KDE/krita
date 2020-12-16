/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KISUPDATERBASE_H
#define KISUPDATERBASE_H

#include <QObject>
#include <KisUpdaterStatus.h>

class QString;

class KisUpdaterBase : public QObject
{
    Q_OBJECT

public:
    KisUpdaterBase();

    /**
     * @brief start the process checking whether there is an update available or not
     * When the check is done, the updater emits sigUpdateCheckStateChange with the check result
     */
    virtual void checkForUpdate() = 0;

    /**
     * @brief Returns true if this updater can actually perform an update.
     * If it can only check for new versions, return false.
     * @return bool
     */
    virtual bool hasUpdateCapability() = 0;

    /**
     * @brief if the updater has update capability, start the update process
     * When the update is done, the updater emits sigUpdateCheckStateChange with the check result
     */
    virtual void doUpdate() = 0;

Q_SIGNALS:
    void sigUpdateCheckStateChange(KisUpdaterStatus);

protected:
    KisUpdaterStatus m_updaterStatus;
};

#endif // KISUPDATERBASE_H
