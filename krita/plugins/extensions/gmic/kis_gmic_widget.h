/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef _KIS_GMIC_WIDGET_H_
#define _KIS_GMIC_WIDGET_H_

#include <QTreeView>
#include <QGridLayout>
#include "kis_gmic_filter_model.h"
#include "kis_gmic_filter_settings.h"

class KisGmicInputOutputWidget;
class QPushButton;
class KisGmicWidget : public QWidget
{
    Q_OBJECT

public:
    KisGmicWidget(KisGmicFilterModel * filters);
    ~KisGmicWidget();

    void createMainLayout();

signals:
    void sigApplyCommand(KisGmicFilterSetting * setting);

private slots:
    void selectionChangedSlot(const QItemSelection & newSelection, const QItemSelection & oldSelection);
    void applyFilterSlot();
    void resetFilterSlot();
    void okFilterSlot();
    void maximizeSlot();
    void cancelFilterSlot();

private:
    QGridLayout * m_filterConfigLayout;

    QTreeView * m_filterTree;
    QWidget * m_filterOptions;
    KisGmicInputOutputWidget * m_inputOutputOptions;

    KisGmicFilterModel * m_filterModel;

    int m_filterOptionsRow;
    int m_filterOptionsColumn;
};

#endif
