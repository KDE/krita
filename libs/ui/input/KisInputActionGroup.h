/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISINPUTACTIONGROUP_H
#define KISINPUTACTIONGROUP_H

#include <QFlags>
#include <QSharedPointer>

enum KisInputActionGroup {
    NoActionGroup = 0x0,
    ViewTransformActionGroup = 0x1,
    ModifyingActionGroup = 0x2,
    AllActionGroup = ViewTransformActionGroup | ModifyingActionGroup
};

Q_DECLARE_FLAGS(KisInputActionGroupsMask, KisInputActionGroup)
Q_DECLARE_OPERATORS_FOR_FLAGS(KisInputActionGroupsMask)

class KisInputActionGroupsMaskGuard;

/**
 * A special interface class for accessing masking properties using
 * KisInputActionGroupsMaskGuard
 */
struct KisInputActionGroupsMaskInterface
{
    /**
     * Unregister ourselves from all KisInputActionGroupsMaskGuard
     */
    virtual ~KisInputActionGroupsMaskInterface();

    /**
     * Return the mask of currently available input action groups
     */
    virtual KisInputActionGroupsMask inputActionGroupsMask() const = 0;

    /**
     * Set the mask of currently available action groups
     */
    virtual void setInputActionGroupsMask(KisInputActionGroupsMask mask) = 0;

    struct Reference
    {
        /**
         * Pointer to interface that can be modified by `this` and be read by guards
         */
        KisInputActionGroupsMaskInterface *m_ref;
    };
    using SharedReference=QSharedPointer<Reference>;
    /**
     * Get a shared copy of `m_sharedReference`
     */
    SharedReference getSharedReference();


private:
    /**
     * Keep a redundant reference to this
     * In case `this` is deleted, we will un-register ourselves from here so guards will *not* try to update `this` after we've been deleted
     */
    SharedReference m_sharedReference;
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
     * @param interfaceReference Shared reference that can be updated by original interface (taken from KisInputActionGroupsMaskInterface::getSharedReference())
     */
    KisInputActionGroupsMaskGuard(KisInputActionGroupsMaskInterface::SharedReference interfaceReference, KisInputActionGroupsMask mask);

    /**
     * Destroy the guard and reset the mask value to the old value (if masking interface wasn't deleted)
     */
    ~KisInputActionGroupsMaskGuard();

private:
    /**
     * Reference the interface to be updated on delete
     * In case interface is deleted, it will unregister itself here so any guard will *not* try to update it on deletion
     */
    KisInputActionGroupsMaskInterface::SharedReference m_interfaceReference;
    KisInputActionGroupsMask m_oldMask;
};

#endif // KISINPUTACTIONGROUP_H
