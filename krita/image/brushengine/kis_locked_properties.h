/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KISLOCKEDPROPERTIES_H
#define KISLOCKEDPROPERTIES_H

#include "kis_properties_configuration.h"
#include "kis_paintop_settings.h"

/**
 * This class maintains a list of all the PaintOp Options that are supposed to be
 * constant across all paintops and presets.
 * addToLockedProperties() adds all the settings mentioned in the parameter to the list
 * removeFromLockedProperties() removes a particular set of properties from the list
 * hasProperty() checks for a particular property in the list
 */
class KisLockedProperties
{
public:
    KisLockedProperties();
    KisLockedProperties(KisPropertiesConfiguration *p);

    /**Whenever any setting is made locked**/
    void addToLockedProperties(KisPropertiesConfiguration *p);

    /**Whenever any property is unlocked**/
    void removeFromLockedProperties(KisPropertiesConfiguration *p);
    bool hasProperty(const QString &p);




    KisPropertiesConfiguration *lockedProperties();


private:
    KisPropertiesConfiguration* m_lockedProperties;


};

#endif // KISLOCKEDPROPERTIES_H
