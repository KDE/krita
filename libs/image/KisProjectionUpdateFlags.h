/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISPROJECTIONUPDATEFLAGS_H
#define KISPROJECTIONUPDATEFLAGS_H

enum class KisProjectionUpdateFlag
{
    None = 0x0,
    /**
     * NoFilthy:
     *
     * Initiate a stack regeneration skipping the recalculation of the
     * filthy node's projection.
     *
     * Works exactly as pseudoFilthy->setDirty() with the only
     * exception that pseudoFilthy::updateProjection() will not be
     * called. That is used by KisRecalculateTransformMaskJob to avoid
     * cyclic dependencies.
     */
    NoFilthy = 0x1,

    /**
     * DontInvalidateFrames
     *
     * This update pass will not trigger any frame cache invalidations
     */
    DontInvalidateFrames = 0x2
};
Q_DECLARE_FLAGS(KisProjectionUpdateFlags, KisProjectionUpdateFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(KisProjectionUpdateFlags)

#endif // KISPROJECTIONUPDATEFLAGS_H
