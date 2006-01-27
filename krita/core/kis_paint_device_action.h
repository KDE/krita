/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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
#ifndef KIS_PAINTDEV_ACTION_H_
#define KIS_PAINTDEV_ACTION_H_

#include "kis_paint_device.h"
class QString;

/**
 * Defines an action to do with a paint device. It can be force used by the gui on creation
 * of a layer, for example. Or just appear in a list of actions to do.
 */
class KisPaintDeviceAction {
public:
    virtual ~KisPaintDeviceAction() {}
    /**
     * Do something with the paint device. This can be anything, like, for example, popping
     * up a dialog to choose a texture. The width and height are added because these may
     * be needed in some cases.
     */
    virtual void act(KisPaintDeviceSP paintDev, Q_INT32 w = 0, Q_INT32 h = 0) const = 0;
    /// The name of the action, to be displayed in the GUI
    virtual QString name() const = 0;
    /// A description of the action, to be displayed in the GUI
    virtual QString description() const = 0;
};

#endif // KIS_PAINTDEV_ACTION_H_
