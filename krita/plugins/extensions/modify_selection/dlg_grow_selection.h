/*
 *  dlg_grow_selection.h -- part of Krita
 *
 *  Copyright (c) 2006 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2013 Juan Palacios <jpalaciosdev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

    virtual void getConfiguration(KisOperationConfiguration* config);

private slots:
    void slotGrowValueChanged(int value);
    void slotGrowValueChanged(double value);
    void slotUnitChanged(int index);

private:
    void updateGrowUIValue(double value);

    double m_resolution;
    int m_growValue;
};

#endif // DLG_GROW_SELECTION_H
