/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KISLOCKEDPROPERTIES_H
#define KISLOCKEDPROPERTIES_H

#include "kis_properties_configuration.h"

/**
 * This class maintains a list of all the PaintOp Options that are supposed to be
 * constant across all paintops and presets.
 * addToLockedProperties() adds all the settings mentioned in the parameter to the list
 * removeFromLockedProperties() removes a particular set of properties from the list
 * hasProperty() checks for a particular property in the list
 */
class KisLockedProperties : public KisShared
{
public:
    KisLockedProperties();
    ~KisLockedProperties();

    /**Whenever any setting is made locked**/
    void addToLockedProperties(KisPropertiesConfigurationSP p);
    void addToLockedProperties(const KisPropertiesConfiguration *p);

    /**Whenever any property is unlocked**/
    void removeFromLockedProperties(KisPropertiesConfigurationSP p);
    void removeFromLockedProperties(const KisPropertiesConfiguration *p);
    bool hasProperty(const QString &p);

    KisPropertiesConfigurationSP lockedProperties();

private:
    Q_DISABLE_COPY(KisLockedProperties)
    KisPropertiesConfigurationSP m_lockedProperties;
};

#endif // KISLOCKEDPROPERTIES_H
