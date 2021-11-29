/*
 *  dlg_border_selection.h -- part of Krita
 *
 *  SPDX-FileCopyrightText: 2006 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  SPDX-FileCopyrightText: 2013 Juan Palacios <jpalaciosdev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef DLG_BORDER_SELECTION_H
#define DLG_BORDER_SELECTION_H

#include "ui_wdg_border_selection.h"
#include <operations/kis_operation_ui_widget.h>

class KisViewManager;
class WdgBorderSelection : public KisOperationUIWidget, public Ui::WdgBorderSelection
{
    Q_OBJECT

public:
    WdgBorderSelection(QWidget *parent, KisViewManager* view, KisOperationConfigurationSP config);

    void getConfiguration(KisOperationConfigurationSP config) override;

private Q_SLOTS:
    void slotWidthChanged(int width);
    void slotWidthChanged(double width);
    void slotUnitChanged(int index);
    void slotAntialiasingChanged(bool value);
    void slotUpdateAntialiasingAvailability();

private:
    void updateWidthUIValue(double value);

    double m_resolution;
    int m_width;
    bool m_antialiasing = false;
};

#endif // DLG_BORDER_SELECTION_H
