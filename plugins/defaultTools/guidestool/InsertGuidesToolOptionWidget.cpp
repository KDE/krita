/* This file is part of the KDE project
 * Copyright (C) 2009 Carlos Licea <carlos.licea@kdemail.net>
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

InsertGuidesToolOptionWidget::InsertGuidesToolOptionWidget(QWidget * parent)
: QWidget( parent )
{
    m_widget.setupUi(this);

    //FIXME give real icon names
    m_widget.m_erasePreviousCheckBox->setIcon( KIcon("erase-previous-guides") );
    m_widget.m_horizontalEdgesCheckBox->setIcon( KIcon("add-horizontal-edges") );
    m_widget.m_verticalEdgesCheckBox->setIcon( KIcon("add-vertical-edges") );

    connect( m_widget.m_verticalCheckBox, SIGNAL(toggled(bool)),
             this, SLOT(verticalCheckBoxSlot(bool)));
    connect( m_widget.m_horizontalCheckBox, SIGNAL(toggled(bool)),
             this, SLOT(horizontalCheckBoxSlot(bool)));

    connect( m_widget.m_createButton, SIGNAL(clicked(bool)),
             this, SLOT(createButtonClickedSlot(bool)));
}

InsertGuidesToolOptionWidget::~InsertGuidesToolOptionWidget()
{
}


void InsertGuidesToolOptionWidget::horizontalCheckBoxSlot( bool state )
{
    m_widget.m_horizontalSpinBox->setEnabled( state );
}

void InsertGuidesToolOptionWidget::verticalCheckBoxSlot( bool state )
{
    m_widget.m_verticalSpinBox->setEnabled( state );
}

void InsertGuidesToolOptionWidget::createButtonClickedSlot( bool checked ) {
    Q_UNUSED( checked );

    GuidesTransaction* transaction = new GuidesTransaction;
    transaction->erasePreviousGuides = m_widget.m_erasePreviousCheckBox->isChecked();
    transaction->verticalGuides = (m_widget.m_verticalCheckBox->isChecked())? m_widget.m_verticalSpinBox->value(): 0;
    transaction->insertVerticalEdgesGuides = m_widget.m_verticalEdgesCheckBox->isChecked();
    transaction->horizontalGuides = (m_widget.m_horizontalCheckBox->isChecked())? m_widget.m_horizontalSpinBox->value(): 0;
    transaction->insertHorizontalEdgesGuides = m_widget.m_horizontalEdgesCheckBox->isChecked();

    emit( createGuides( transaction ) );
}

#include "InsertGuidesToolOptionWidget.moc"