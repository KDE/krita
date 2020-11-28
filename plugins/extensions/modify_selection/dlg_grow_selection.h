/*
 *  dlg_grow_selection.h -- part of Krita
 *
 *  SPDX-FileCopyrightText: 2006 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  SPDX-FileCopyrightText: 2013 Juan Palacios <jpalaciosdev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef DLG_GROW_SELECTION_H
#define DLG_GROW_SELECTION_H

#include "ui_wdg_grow_selection.h"
#include <operations/kis_operation_ui_widget.h>

class KisViewManager;
class WdgGrowSelection : public KisOperationUIWidget, public Ui::WdgGrowSelection
{
    Q_OBJECT

public:
    WdgGrowSelection(QWidget *parent, KisViewManager* view);

    void getConfiguration(KisOperationConfigurationSP config) override;

private Q_SLOTS:
    void slotGrowValueChanged(int value);
    void slotGrowValueChanged(double value);
    void slotUnitChanged(int index);

private:
    void updateGrowUIValue(double value);

    double m_resolution;
    int m_growValue;
};

#endif // DLG_GROW_SELECTION_H
