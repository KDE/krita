/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCOMPOSITEOPLISTCONNECTIONHELPER_H
#define KISCOMPOSITEOPLISTCONNECTIONHELPER_H

#include <kritaui_export.h>

class QObject;
class KisCompositeOpListWidget;

namespace KisWidgetConnectionUtils
{
void KRITAUI_EXPORT connectControl(KisCompositeOpListWidget *widget, QObject *source, const char *property);
}


#endif // KISCOMPOSITEOPLISTCONNECTIONHELPER_H
