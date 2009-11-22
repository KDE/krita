/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <shichan.karachu@gmail.com>

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

#include "kis_favorite_brush_data.h"
#include "kis_popup_palette.h"
#include "ko_favorite_resource_manager.h"
#include <kis_paintop_preset.h>
#include <kis_types.h>
#include <KoID.h>
#include <QDebug>
#include <QIcon>
#include <QToolButton>
#include <QColor>

KisFavoriteBrushData::KisFavoriteBrushData(KoFavoriteResourceManager* resourceManager, KisPaintOpPresetSP  newdata, QIcon* icon)
        : m_favoriteResourceManager (resourceManager)
        , m_button (0)
        , m_data (newdata)
{
    m_button = new QToolButton();
    m_button->setMinimumSize(KisPopupPalette::BUTTON_SIZE, KisPopupPalette::BUTTON_SIZE);
    m_button->setMaximumSize(KisPopupPalette::BUTTON_SIZE, KisPopupPalette::BUTTON_SIZE);
    m_button->setToolTip(m_data->paintOp().id());
    if (icon) m_button->setIcon(*icon);
    m_button->setAutoFillBackground(false);

//    QPalette p(m_button->palette());
//    p.setColor(QPalette::Button, QColor (232,231,230,255));
//    m_button->setPalette(p);
//    m_button->setAutoFillBackground(false);
//    m_button->setStyleSheet("* { background-color: rgba(232,231,230,255) }");

    connect(m_button, SIGNAL(clicked()), this, SLOT(slotBrushButtonClicked()));
    connect(this, SIGNAL(signalPaintOpChanged(KisPaintOpPresetSP)), m_favoriteResourceManager, SLOT(slotChangeCurrentPaintOp(KisPaintOpPresetSP)));
}

void KisFavoriteBrushData::slotBrushButtonClicked()
{
    qDebug() << "Brush name:" << m_data->paintOp();
    emit signalPaintOpChanged(m_data);
}

KisPaintOpPresetSP KisFavoriteBrushData::paintopPreset()
{
    return m_data;
}

void KisFavoriteBrushData::setIcon (QIcon* icon)
{
    m_button->setIcon(*icon);
}

QToolButton* KisFavoriteBrushData::paintopButton()
{
    return m_button;
}

KisFavoriteBrushData::~KisFavoriteBrushData()
{
    m_favoriteResourceManager = 0;
    qDebug() << "Brush name:" << m_data->paintOp() << "deleting";
    // don't delete m_data, it's a shared pointer
    // only delete the button if it hasn't got a parent object, otherwise
    // we have a double delete
    if (!m_button->parent()) {
        delete m_button;
    }
}

#include "kis_favorite_brush_data.moc"
