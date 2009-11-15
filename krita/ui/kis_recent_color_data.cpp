/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <vla24@sfu.ca>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#include "kis_popup_palette.h"
#include "kis_recent_color_data.h"
#include <QDebug>
#include <QIcon>
#include <QToolButton>
#include <stdio.h>

KisRecentColorData::KisRecentColorData(QColor *newColor)
        : m_button (0)
        , m_data (newColor)
{
    m_button = new QToolButton();

    //setting color
    char str[45];
    sprintf(str, "* { background-color: rgb(%i,%i,%i) }", color()->red(), color()->green(),color()->blue());

    m_button->setStyleSheet(str);

    m_button->setMinimumSize(KisPopupPalette::BUTTON_SIZE, KisPopupPalette::BUTTON_SIZE);
    m_button->setMaximumSize(KisPopupPalette::BUTTON_SIZE, KisPopupPalette::BUTTON_SIZE);
    m_button->connect(m_button, SIGNAL(clicked()), this, SLOT(slotColorButtonClicked()));
}

void KisRecentColorData::slotColorButtonClicked()
{
    qDebug() << "Color: (r)" << color()->red() << "(g)" << color()->green()
            << "(b)" << color()->blue();
}

QColor* KisRecentColorData::color()
{
    return m_data;
}

void KisRecentColorData::setIcon (QIcon* icon)
{
    m_button->setIcon(*icon);
}

QToolButton* KisRecentColorData::colorButton()
{
    return m_button;
}

KisRecentColorData::~KisRecentColorData()
{
    delete m_data;
    if (!m_button->parent()) {
        delete m_button;
    } else {
        m_button->setVisible(false);
    }
}
#include "kis_recent_color_data.moc"
