/* This file is part of the KDE project
 * Copyright (C) 2009 Carlos Licea <carlos.licea@kdemail.net>
 * Copyright 2012 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#include "InsertGuidesToolOptionWidget.h"

#include <KoIcon.h>

InsertGuidesToolOptionWidget::InsertGuidesToolOptionWidget(QWidget *parent)
    : QWidget(parent)
{
    m_widget.setupUi(this);

    m_widget.m_horizontalEdgesCheckBox->setIcon(koIconNeeded("add guides at top & bottom side of page", "add-horizontal-edges"));
    m_widget.m_verticalEdgesCheckBox->setIcon(koIconNeeded("add guides at left & right side of page", "add-vertical-edges"));

    connect(m_widget.m_createButton, SIGNAL(clicked(bool)),
            this, SLOT(onCreateButtonClicked(bool)));
}

InsertGuidesToolOptionWidget::~InsertGuidesToolOptionWidget()
{
}

void InsertGuidesToolOptionWidget::onCreateButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    GuidesTransaction *transaction = new GuidesTransaction;
    transaction->erasePreviousGuides = m_widget.m_erasePreviousCheckBox->isChecked();
    transaction->verticalGuides = m_widget.m_verticalSpinBox->value();
    transaction->insertVerticalEdgesGuides = m_widget.m_verticalEdgesCheckBox->isChecked();
    transaction->horizontalGuides = m_widget.m_horizontalSpinBox->value();
    transaction->insertHorizontalEdgesGuides = m_widget.m_horizontalEdgesCheckBox->isChecked();

    emit createGuides(transaction);
}
