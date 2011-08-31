/*
 *
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_text_tool_option_widget.h"

KisTextToolOptionWidget::KisTextToolOptionWidget(QWidget* parent): QWidget(parent)
{
    setupUi(this);
    
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);
    m_buttonGroup->addButton(buttonArtistic);
    m_buttonGroup->setId(buttonArtistic, 0);
    m_buttonGroup->addButton(buttonMultiline);
    m_buttonGroup->setId(buttonMultiline, 1);
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(modeChanged(int)));
}

KisTextToolOptionWidget::TextMode KisTextToolOptionWidget::mode()
{
    return static_cast<TextMode>(m_buttonGroup->checkedId());
}

KisPainter::FillStyle KisTextToolOptionWidget::style()
{
    return static_cast<KisPainter::FillStyle>(cmbStyle->currentIndex() + 1);
}

void KisTextToolOptionWidget::modeChanged(int mode)
{
    cmbStyle->setEnabled(mode == MODE_ARTISTIC);
}
