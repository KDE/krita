/*
 * SPDX-FileCopyrightText: 2022 Freya Lupen <penguinflyer2222@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCURVEWIDGETCONNECTIONHELPER_H
#define KISCURVEWIDGETCONNECTIONHELPER_H

#include <kritaui_export.h>

class QObject;
class KisCurveWidget;

namespace KisWidgetConnectionUtils
{
void KRITAUI_EXPORT connectControl(KisCurveWidget *widget, QObject *source, const char *property);
}

#endif // KISCURVEWIDGETCONNECTIONHELPER_H
