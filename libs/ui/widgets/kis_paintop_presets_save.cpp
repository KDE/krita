/* This file is part of the KDE project
 * Copyright (C) 2017 Scott Petrovic <scottpetrovic@gmail.com>
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

#include "widgets/kis_paintop_presets_save.h"
#include <QDebug>



KisPresetSaveWidget::KisPresetSaveWidget(QWidget * parent)
    : KisPaintOpPresetSaveDialog(parent)
{

    setModal(true);



    QDialog::DialogCode result = (QDialog::DialogCode)this->exec();

    if(result) {
        qDebug() << "stuff ran in the dialog";
    }

    // TODO:
    // might be able to just add a scratchpad widget for that part to the UI
    //  <widget class="KisScratchPad" name="scratchPad" native="true">
    // maybe then hide the "load preset" icon as that will be done elsewhere
}



KisPresetSaveWidget::~KisPresetSaveWidget()
{

}
