/*
 *  dlg_shrink_selection.h -- part of Krita
 *
 *  SPDX-FileCopyrightText: 2006 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  SPDX-FileCopyrightText: 2013 Juan Palacios <jpalaciosdev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef DLG_SHRINK_SELECTION_H
#define DLG_SHRINK_SELECTION_H

#include "ui_wdg_shrink_selection.h"
#include <operations/kis_operation_ui_widget.h>

class KisViewManager;
class WdgShrinkSelection : public KisOperationUIWidget, public Ui::WdgShrinkSelection
{
    Q_OBJECT

public:
    WdgShrinkSelection(QWidget *parent, KisViewManager* view);

    void getConfiguration(KisOperationConfigurationSP config) override;

private Q_SLOTS:
    void slotShrinkValueChanged(int value);
    void slotShrinkValueChanged(double value);
    void slotUnitChanged(int index);

private:
    void updateShrinkUIValue(double value);

    double m_resolution;
    int m_shrinkValue;
};

#endif // DLG_SHRINK_SELECTION_H
