/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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
#include "kis_preset_chooser.h"
#include "KoResourceItemChooser.h"
#include "kis_paintop_preset.h"
#include "QLabel"


KisPresetChooser::KisPresetChooser(QWidget * parent)
        : KisItemChooser(parent)
{
    setObjectName("KisPresetChooser");
    //setMinimumSize(chooserWidget()->sizeHint());
}


KisPresetChooser::~KisPresetChooser()
{
}


void KisPresetChooser::update(QTableWidgetItem* i)
{
    Q_UNUSED(i);
//    KoResourceItem *item = dynamic_cast<KoResourceItem *>(i);
//
//    if (item) {
//        KisPaintOpPreset *preset = dynamic_cast<KisPaintOpPreset*>(item->resource());
//        if (preset) {
//            parent()->findChild<QLabel*>("lblName")->setText(preset->name());
//        }
//    }
}
