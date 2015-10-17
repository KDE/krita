/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
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

#include "kis_node_query_path_editor.h"

#include "ui_wdgnodequerypatheditor.h"
#include <QWhatsThis>
#include <recorder/kis_node_query_path.h>
#include <kis_icon.h>

struct KisNodeQueryPathEditor::Private
{
    Ui_WdgNodeQueryPathEditor form;
};

KisNodeQueryPathEditor::KisNodeQueryPathEditor(QWidget* parent) : QWidget(parent), d(new Private)
{
    d->form.setupUi(this);
  
    connect(d->form.radioButtonCurrentLayer, SIGNAL(clicked(bool)), SLOT(currentLayerEnabled(bool)));
    connect(d->form.radioButtonCustomPath, SIGNAL(clicked(bool)), SLOT(customPathEnabled(bool)));
    
    d->form.kpushbutton->setIcon(KisIconUtils::loadIcon("system-help"));
    connect(d->form.kpushbutton, SIGNAL(clicked()), this, SLOT(slotPopupQuickHelp()));
    currentLayerEnabled(true);

    connect(d->form.klineeditPath, SIGNAL(textChanged(QString)), SIGNAL(nodeQueryPathChanged()));
}

KisNodeQueryPathEditor::~KisNodeQueryPathEditor()
{
    delete d;
}

void KisNodeQueryPathEditor::setNodeQueryPath(const KisNodeQueryPath& path)
{
  if(path.toString() == ".")
  {
    d->form.radioButtonCurrentLayer->setChecked(true);
    currentLayerEnabled(true);
  } else {
    d->form.radioButtonCustomPath->setChecked(true);
    customPathEnabled(true);
    d->form.klineeditPath->setText(path.toString());
  }
}

KisNodeQueryPath KisNodeQueryPathEditor::nodeQueryPath() const
{
    return KisNodeQueryPath::fromString(d->form.klineeditPath->text());
}

void KisNodeQueryPathEditor::currentLayerEnabled(bool v)
{
    if(!v) return;
    d->form.klineeditPath->setEnabled(false);
    d->form.kpushbutton->setEnabled(false);
    d->form.klineeditPath->setText(".");
}

void KisNodeQueryPathEditor::customPathEnabled(bool v)
{
    if(!v) return;
    d->form.klineeditPath->setEnabled(true);
    d->form.kpushbutton->setEnabled(true);
}

void KisNodeQueryPathEditor::slotPopupQuickHelp()
{
    QWhatsThis::showText(QCursor::pos(), i18n(
          "<b>/</b> represents the root of the image, or a separator<br/>\n"
          "<b>a number</b> represents a layer<br/>\n"
          "<b>.</b> represents the current layer<br/>\n"
          "<b>..</b> represents the parent layer<br/>\n\n"
          "<b>Examples:</b><br/>\n"
          "<i>/0</i> represents the bottom layer of the image<br/>\n"
          "<i>../1</i> represents the second layer from the bottom of the parent of the current layer<br/>\n"
          "<i>./0</i> represents the first child of the current layer" ) );
}

