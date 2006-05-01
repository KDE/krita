/*
 *  Copyright (c) 2006 Adrian Page <adrian@pagenet.plus.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.g
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_matrix_widget.h"

KisMatrixWidget::KisMatrixWidget(QWidget *parent, const char *name)
    : QWidget(parent)
{
    setObjectName(name);
    setupUi(this);

    connect(m11, SIGNAL(valueChanged(int)), SLOT(slotSpinBoxValueChanged()));
    connect(m12, SIGNAL(valueChanged(int)), SLOT(slotSpinBoxValueChanged()));
    connect(m13, SIGNAL(valueChanged(int)), SLOT(slotSpinBoxValueChanged()));
    connect(m21, SIGNAL(valueChanged(int)), SLOT(slotSpinBoxValueChanged()));
    connect(m22, SIGNAL(valueChanged(int)), SLOT(slotSpinBoxValueChanged()));
    connect(m23, SIGNAL(valueChanged(int)), SLOT(slotSpinBoxValueChanged()));
    connect(m31, SIGNAL(valueChanged(int)), SLOT(slotSpinBoxValueChanged()));
    connect(m32, SIGNAL(valueChanged(int)), SLOT(slotSpinBoxValueChanged()));
    connect(m33, SIGNAL(valueChanged(int)), SLOT(slotSpinBoxValueChanged()));
}

void KisMatrixWidget::slotSpinBoxValueChanged()
{
    emit valueChanged();
}

#include "kis_matrix_widget.moc"

