/*
 *  SPDX-FileCopyrightText: 2017 Eugene Ingerman
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TOOL_SMART_PATCH_OPTIONS_WIDGET_H
#define __KIS_TOOL_SMART_PATCH_OPTIONS_WIDGET_H

#include <QScopedPointer>
#include <QWidget>
#include <QModelIndex>

#include "kis_types.h"

class KisCanvasResourceProvider;
class KoColor;


class KisToolSmartPatchOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    KisToolSmartPatchOptionsWidget(KisCanvasResourceProvider *provider, QWidget *parent);
    ~KisToolSmartPatchOptionsWidget() override;

    int getPatchRadius(void);
    int getAccuracy(void);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_TOOL_SMART_PATCH_OPTIONS_WIDGET_H */
