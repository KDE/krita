/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISRENDERPASSFLAGS_H
#define KISRENDERPASSFLAGS_H

enum class KisRenderPassFlag
{
    None = 0x0,
    NoTransformMaskUpdates = 0x1
};
Q_DECLARE_FLAGS(KisRenderPassFlags, KisRenderPassFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(KisRenderPassFlags)

#endif // KISRENDERPASSFLAGS_H
