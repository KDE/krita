/* This file is part of the KDE project
 * Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
 * Copyright (C) 2005 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
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

#include <KoPageLayoutColumns.h>
#include <KoPageLayoutDia.h>
#include <KoUnit.h>
#include <KoUnitWidgets.h>

#include <QLabel>
#include <QLayout>
//Added by qt3to4:
#include <Q3HBoxLayout>

KoPageLayoutColumns::KoPageLayoutColumns(QWidget *parent, const KoColumns& columns, KoUnit::Unit unit, const KoPageLayout& layout)
    : KoPageLayoutColumnsBase(parent) {
    m_columns = columns;
    Q3HBoxLayout *lay = new Q3HBoxLayout(previewPane);
    m_preview = new KoPagePreview( previewPane, "Preview", layout );
    lay->addWidget(m_preview);
    lay = new Q3HBoxLayout(columnSpacingPane);
    m_spacing = new KoUnitDoubleSpinBox( columnSpacingPane );
    m_spacing->setValue(  m_columns.ptColumnSpacing );
    m_spacing->setUnit( unit );
    double dStep = KoUnit::fromUserValue( 0.2, unit );
    m_spacing->setMinMaxStep( 0, layout.ptWidth/2, dStep );
    lay->addWidget(m_spacing);
    labelSpacing->setBuddy( m_spacing );
    nColumns->setValue( m_columns.columns );
    m_preview->setPageColumns( m_columns );

    connect( nColumns, SIGNAL( valueChanged( int ) ), this, SLOT( nColChanged( int ) ) );
    connect( m_spacing, SIGNAL( valueChangedPt(double) ), this, SLOT( nSpaceChanged( double ) ) );
}

void KoPageLayoutColumns::setEnableColumns(bool on) {
    nColumns->setEnabled(on);
    m_spacing->setEnabled(on);
    nColChanged(on ? nColumns->value(): 1 );
}

void KoPageLayoutColumns::nColChanged( int columns ) {
    m_columns.columns = columns;
    m_preview->setPageColumns( m_columns );
    emit propertyChange(m_columns);
}

void KoPageLayoutColumns::nSpaceChanged( double spacing ) {
    m_columns.ptColumnSpacing = spacing;
    emit propertyChange(m_columns);
}

void KoPageLayoutColumns::setLayout(KoPageLayout &layout) {
    m_preview->setPageLayout( layout );
}

#include <KoPageLayoutColumns.moc>
