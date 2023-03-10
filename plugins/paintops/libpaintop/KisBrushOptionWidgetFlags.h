/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISBRUSHOPTIONWIDGETFLAGS_H
#define KISBRUSHOPTIONWIDGETFLAGS_H

#include <QFlags>

enum class KisBrushOptionWidgetFlag
{
    None = 0x0,
    SupportsHSLBrushMode = 0x1,
    SupportsPrecision = 0x2
};

Q_DECLARE_FLAGS(KisBrushOptionWidgetFlags, KisBrushOptionWidgetFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(KisBrushOptionWidgetFlags)

#endif // KISBRUSHOPTIONWIDGETFLAGS_H
