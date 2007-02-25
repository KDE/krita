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

#include "kis_filters_list_dynamic_programs_editor.h"

#include <kdialog.h>
#include <klocale.h>

#include "ui_FiltersListDynamicProgramEditor.h"

#include "kis_filters_list_model.h"
#include "kis_dynamic_brush.h"
#include "kis_dynamic_brush_registry.h"

void KisFiltersListDynamicProgramsEditor::editBrush()
{
   KDialog *dialog = new KDialog( 0 );
   dialog->setCaption( i18n("Dynamic Brush Advanced Editor") );
   dialog->setButtons( KDialog::Ok );
   QWidget* widget = new QWidget(dialog);
   Ui_FiltersListDynamicProgramEditor ae;
   ae.setupUi(widget);
   KisFiltersListModel* filtersModel = new KisFiltersListModel(  (KisFiltersListDynamicProgram*)KisDynamicBrushRegistry::instance()->current()->program() , ae.listViewFilters);
   ae.listViewFilters->setModel( filtersModel );
   filtersModel->connect(ae.comboBoxFilter, SIGNAL(activated(int)), SLOT(setCurrentFilterType(int)));
   filtersModel->connect(ae.pushButtonAdd, SIGNAL(pressed()), SLOT(addNewFilter()));
   filtersModel->connect(ae.listViewFilters->selectionModel(), SIGNAL(currentChanged ( const QModelIndex & , const QModelIndex & )), SLOT(setCurrentFilter(const QModelIndex&)));
   filtersModel->connect(ae.pushButtonRemove, SIGNAL(pressed()), SLOT(deleteCurrentFilter()));
   dialog->setMainWidget( widget );
   dialog->exec();
}
