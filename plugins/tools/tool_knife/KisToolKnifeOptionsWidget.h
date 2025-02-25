/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TOOL_KNIFE_OPTIONS_WIDGET_H
#define __KIS_TOOL_KNIFE_OPTIONS_WIDGET_H

#include <QScopedPointer>
#include <QWidget>
#include <QModelIndex>

#include "kis_types.h"

class KisCanvasResourceProvider;
class KoColor;


class KisToolKnifeOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    KisToolKnifeOptionsWidget(KisCanvasResourceProvider *provider, QWidget *parent);
    ~KisToolKnifeOptionsWidget() override;

    enum GapWidthType {
        Thick,
        Thin,
        Special
    };

    enum ToolMode {
        AddGutter,
        RemoveGutter
    };

    int getThickGapWidth(void);
    int getThinGapWidth(void);
    int getSpecialGapWidth(void);
    GapWidthType getWidthType();
    int getCurrentWidth();
    int getWidthForType(GapWidthType type);

    ToolMode getToolMode();


private:
    friend class KisToolKnife;

    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_TOOL_KNIFE_OPTIONS_WIDGET_H */
