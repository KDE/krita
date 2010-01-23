/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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

#include "widgets/kis_preset_widget.h"

#include <QPainter>
#include <QIcon>
#include <QWidget>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_paintop_preset.h>

KisPresetWidget::KisPresetWidget(QWidget *parent, const char *name)
        : KisPopupButton(parent)
{
    setObjectName(name);
    m_preset = 0;
    m_drawArrow = true;
}

void KisPresetWidget::setPreset(KisPaintOpPresetSP preset)
{
    Q_ASSERT(preset);
    m_preset = preset.data();
    updatePreview();
}

void KisPresetWidget::setDrawArrow(bool v)
{
    m_drawArrow = v;
}

void KisPresetWidget::updatePreview()
{
    if (m_preset) {
        if((width() < 100) || (height() < 100))
            m_image = m_preset->generatePreviewImage(width() * 3, height() * 3);
        else
            m_image = m_preset->generatePreviewImage(width(), height());
        update();
    }
}

void KisPresetWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    qint32 cw = width() - 1;
    qint32 ch = height() - 1;
    p.fillRect(0, 0, cw, ch, Qt::white);  // XXX: use a palette for this instead of white?

    if (!m_image.isNull()) {
        p.drawImage(2, 2, m_image.scaled(QSize(cw - 2, ch - 2), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    p.setPen(Qt::gray);
    p.drawRect(0, 0, cw + 1, ch + 1);
    (void)p.end();

    if (m_drawArrow) {
        paintPopupArrow();
    }
}


#include "kis_preset_widget.moc"
