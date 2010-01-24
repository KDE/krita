/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "PartDetailsDialog.h"

#include "../MusicTool.h"

#include "../core/Part.h"

#include "../commands/ChangePartDetailsCommand.h"

using namespace MusicCore;

PartDetailsDialog::PartDetailsDialog(Part* part, QWidget* parent)
    : KDialog(parent)
{
    setCaption(i18n("Part details"));
    QWidget* w = new QWidget(this);
    widget.setupUi(w);
    setMainWidget(w);
    
    widget.nameEdit->setText(part->name());
    widget.shortNameEdit->setText(part->shortName());
    widget.staffCount->setValue(part->staffCount());
}

void PartDetailsDialog::showDialog(MusicTool *tool, Part* part, QWidget* parent)
{
    PartDetailsDialog dlg(part, parent);
    if (dlg.exec() == QDialog::Accepted) {
        tool->addCommand(new ChangePartDetailsCommand(tool->shape(), part, dlg.widget.nameEdit->text(),
                                                        dlg.widget.shortNameEdit->text(),
                                                        dlg.widget.staffCount->value()));
    }
}

#include <PartDetailsDialog.moc>
