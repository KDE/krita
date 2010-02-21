/* This file is part of the KDE project
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_tool_select_magnetic_option_widget.h"
#include "ui_kis_tool_select_magnetic_option_widget.h"

KisToolSelectMagneticOptionWidget::KisToolSelectMagneticOptionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KisToolSelectMagneticOptionWidget)
{
    ui->setupUi(this);

    ui->m_colorLimitation->hide(); //hide color limitation, as it doesn't work yet
    ui->label_4->hide();//color limitation label

    //default values
    ui->m_radius->setValue(25);
    ui->m_searchFromLeft->setChecked(true);
    ui->m_threshold->setValue(15);
}

KisToolSelectMagneticOptionWidget::~KisToolSelectMagneticOptionWidget()
{
    delete ui;
}

void KisToolSelectMagneticOptionWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

#include "kis_tool_select_magnetic_option_widget.moc"
