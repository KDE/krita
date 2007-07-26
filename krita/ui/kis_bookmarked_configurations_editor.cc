/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_bookmarked_configurations_editor.h"

#include "ui_wdgbookmarkedconfigurationseditor.h"

#include "kis_bookmarked_configurations_model.h"

struct KisBookmarkedConfigurationsEditor::Private {
    Ui_WdgBookmarkedConfigurationsEditor editorUi;
    KisBookmarkedConfigurationsModel* model;
};


KisBookmarkedConfigurationsEditor::KisBookmarkedConfigurationsEditor(QWidget* parent, KisBookmarkedConfigurationsModel* model) : QDialog(parent), d(new Private)
{
    d->editorUi.setupUi(this);
    d->model = model;
    d->editorUi.listConfigurations->setModel( d->model );
    connect(d->editorUi.pushButtonClose, SIGNAL(pressed()), SLOT(accept()));
}

KisBookmarkedConfigurationsEditor::~KisBookmarkedConfigurationsEditor()
{
    delete d;
}
