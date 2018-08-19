/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISINPUTACTIONGROUP_H
#define KISINPUTACTIONGROUP_H

#include <QFlags>

enum KisInputActionGroup {
    NoActionGroup = 0x0,
    ViewTransformActionGroup = 0x1,
    ModifyingActionGroup = 0x2,
    AllActionGroup = ViewTransformActionGroup | ModifyingActionGroup
};

Q_DECLARE_FLAGS(KisInputActionGroupsMask, KisInputActionGroup)
Q_DECLARE_OPERATORS_FOR_FLAGS(KisInputActionGroupsMask)


/**
 * A special interface class for accessing masking properties using
 * KisInputActionGroupsMaskGuard
 */
struct KisInputActionGroupsMaskInterface
{
    virtual ~KisInputActionGroupsMaskInterface();

    /**
     * Return the mask of currently available input action groups
     */
    virtual KisInputActionGroupsMask inputActionGroupsMask() const = 0;

    /**
     * Set the mask of currently available action groups
     */
    virtual void setInputActionGroupsMask(KisInputActionGroupsMask mask) = 0;
};

/**
 * A RAII wrapper for setting the input actions mask on the masking interface
 * (which is usually a canvas). In constructor the guard saves the previous mask
 * value and resets it to the new one. In destructor the guard restores the old
 * mask value.
 */
class KisInputActionGroupsMaskGuard
{
public:
    /**
     * Create a guard and set a new mask \p mask onto \p object. The old mask value is
     * saved in the guard itself.
     */
    KisInputActionGroupsMaskGuard(KisInputActionGroupsMaskInterface *object, KisInputActionGroupsMask mask);

    /**
     * Destroy the guard and reset the mask value to the old value
     */
    ~KisInputActionGroupsMaskGuard();

private:
    KisInputActionGroupsMaskInterface *m_object;
    KisInputActionGroupsMask m_oldMask;
};

#endif // KISINPUTACTIONGROUP_H
