/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SELECT_LAYER_ACTION_H
#define KIS_SELECT_LAYER_ACTION_H

#include "kis_abstract_input_action.h"

/**
 * \brief Select Layer implementation of KisAbstractInputAction.
 *
 * The Select Layer action selects a layer under a cursor.
 */
class KisSelectLayerAction : public KisAbstractInputAction
{
public:
    enum LayerSelectionMode
    {
        LayerSelectionMode_TopLayer,
        LayerSelectionMode_AllLayers,
        LayerSelectionMode_Ask
    };

    enum SelectionOverrideMode
    {
        SelectionOverrideMode_Replace,
        SelectionOverrideMode_Add
    };

    explicit KisSelectLayerAction();
    ~KisSelectLayerAction() override;

    int priority() const override;

    void activate(int shortcut) override;
    void deactivate(int shortcut) override;

    void begin(int shortcut, QEvent *event) override;
    void inputEvent(QEvent *event) override;

private:
    class Private;
    Private * const d;
};

#endif // KIS_SELECT_LAYER_ACTION_H
