/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISMENUSTYLEDONTCLOSEONALT_H
#define KISMENUSTYLEDONTCLOSEONALT_H

#include "kritawidgetutils_export.h"
#include <QProxyStyle>


/**
 * A special proxy style for menus that have an input box for non-numeric
 * text inside. Some languages require AltGr for typing special symbols,
 * so we need to ensure that the menu is not closed when that key is
 * pressed.
 */
class KRITAWIDGETUTILS_EXPORT KisMenuStyleDontCloseOnAlt : public QProxyStyle
{
public:
    KisMenuStyleDontCloseOnAlt(QStyle *baseStyle);
    int styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const override;
};

#endif // KISMENUSTYLEDONTCLOSEONALT_H
