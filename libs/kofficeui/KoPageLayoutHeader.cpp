/* This file is part of the KDE project
 * Copyright (C) 2005 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <KoPageLayoutHeader.h>
#include <KoPageLayoutHeader.moc>
#include <KoUnitWidgets.h>

#include <qlayout.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <Q3HBoxLayout>

KoPageLayoutHeader::KoPageLayoutHeader(QWidget *parent, KoUnit::Unit unit, const KoKWHeaderFooter &kwhf)
    : KoPageLayoutHeaderBase(parent) {
    m_headerFooters = kwhf;
    Q3HBoxLayout *lay = new Q3HBoxLayout(headerSpacingPane);
    m_headerSpacing = new KoUnitDoubleSpinBox( headerSpacingPane, 0.0, 999.0, 0.5, kwhf.ptHeaderBodySpacing, unit );
    lay->addWidget(m_headerSpacing);

    lay = new Q3HBoxLayout(footerSpacingPane);
    m_footerSpacing = new KoUnitDoubleSpinBox( footerSpacingPane, 0.0, 999.0, 0.5, kwhf.ptFooterBodySpacing, unit );
    lay->addWidget(m_footerSpacing);

    lay = new Q3HBoxLayout(footnotePane);
    m_footnoteSpacing = new KoUnitDoubleSpinBox( footnotePane, 0.0, 999.0, 0.5, kwhf.ptFootNoteBodySpacing, unit );
    lay->addWidget(m_footnoteSpacing);

    if ( kwhf.header == HF_FIRST_DIFF || kwhf.header == HF_FIRST_EO_DIFF )
        rhFirst->setChecked( true );
    if ( kwhf.header == HF_EO_DIFF || kwhf.header == HF_FIRST_EO_DIFF )
        rhEvenOdd->setChecked( true );
    if ( kwhf.footer == HF_FIRST_DIFF || kwhf.footer == HF_FIRST_EO_DIFF )
        rfFirst->setChecked( true );
    if ( kwhf.footer == HF_EO_DIFF || kwhf.footer == HF_FIRST_EO_DIFF )
        rfEvenOdd->setChecked( true );
}

const KoKWHeaderFooter& KoPageLayoutHeader::headerFooter() {
    if ( rhFirst->isChecked() && rhEvenOdd->isChecked() )
        m_headerFooters.header = HF_FIRST_EO_DIFF;
    else if ( rhFirst->isChecked() )
        m_headerFooters.header = HF_FIRST_DIFF;
    else if ( rhEvenOdd->isChecked() )
        m_headerFooters.header = HF_EO_DIFF;
    else
        m_headerFooters.header = HF_SAME;

    m_headerFooters.ptHeaderBodySpacing = m_headerSpacing->value();
    m_headerFooters.ptFooterBodySpacing = m_footerSpacing->value();
    m_headerFooters.ptFootNoteBodySpacing = m_footnoteSpacing->value();
    if ( rfFirst->isChecked() && rfEvenOdd->isChecked() )
        m_headerFooters.footer = HF_FIRST_EO_DIFF;
    else if ( rfFirst->isChecked() )
        m_headerFooters.footer = HF_FIRST_DIFF;
    else if ( rfEvenOdd->isChecked() )
        m_headerFooters.footer = HF_EO_DIFF;
    else
        m_headerFooters.footer = HF_SAME;
    return m_headerFooters;
}
