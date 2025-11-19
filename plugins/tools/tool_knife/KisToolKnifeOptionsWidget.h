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
#include "GutterWidthsConfig.h"
#include <kis_node.h>

class KisCanvasResourceProvider;
class KoColor;
class KoUnit;


class KisToolKnifeOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    KisToolKnifeOptionsWidget(KisCanvasResourceProvider *provider, QWidget *parent, QString toolId, qreal resolution);
    ~KisToolKnifeOptionsWidget() override;

    enum GapWidthType {
        Thick,
        Thin,
        Special,
        Automatic,
    };

    enum ToolMode {
        AddGutter,
        RemoveGutter,
        MoveGutterEndPoint,
    };

    GutterWidthsConfig getCurrentWidthsConfig();

    ToolMode getToolMode();

public Q_SLOTS:
    void unitForWidthChanged(int index);
    void currentNodeChanged(const KisNodeSP node);
    void modeChanged();
    void currentWidthSystemChanged();


private:
    friend class KisToolKnife;

    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_TOOL_KNIFE_OPTIONS_WIDGET_H */
