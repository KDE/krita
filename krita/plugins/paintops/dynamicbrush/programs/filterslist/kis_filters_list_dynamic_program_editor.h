/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_FILTERS_LIST_DYNAMIC_PROGRAM_EDITOR_H_
#define _KIS_FILTERS_LIST_DYNAMIC_PROGRAM_EDITOR_H_

#include <QWidget>

class QModelIndex;
class Ui_FiltersListDynamicProgramEditor;

class KisFiltersListDynamicProgram;
class KisFiltersListModel;

class KisFiltersListDynamicProgramEditor : public QWidget {
    Q_OBJECT
    public:
        KisFiltersListDynamicProgramEditor(KisFiltersListDynamicProgram* program);
        ~KisFiltersListDynamicProgramEditor();
    private slots:
        void setCurrentFilter(const QModelIndex&);
    private:
        KisFiltersListModel* m_filtersModel;
        Ui_FiltersListDynamicProgramEditor* m_filtersListDynamicProgramEditor;
        KisFiltersListDynamicProgram* m_program;
        QWidget* m_currentFilterEditor;
};

#endif
