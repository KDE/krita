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
