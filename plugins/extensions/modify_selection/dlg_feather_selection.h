/*
 *  dlg_feather_selection.h -- part of Krita
 *
 *  SPDX-FileCopyrightText: 2009 Edward Apap <schumifer@hotmail.com>
 *  SPDX-FileCopyrightText: 2013 Juan Palacios <jpalaciosdev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef DLG_FEATHER_SELECTION_H
#define DLG_FEATHER_SELECTION_H

#include "ui_wdg_feather_selection.h"
#include <operations/kis_operation_ui_widget.h>

class KisViewManager;
class WdgFeatherSelection : public KisOperationUIWidget, public Ui::WdgFeatherSelection
{
    Q_OBJECT

public:
    WdgFeatherSelection(QWidget *parent, KisViewManager *view);

    void getConfiguration(KisOperationConfigurationSP config) override;

private Q_SLOTS:
    void slotRadiusChanged(int radius);
    void slotRadiusChanged(double radius);
    void slotUnitChanged(int index);

private:
    void updateRadiusUIValue(double value);

    double m_resolution;
    int m_radius;
};

#endif // DLG_GROW_SELECTION_H
