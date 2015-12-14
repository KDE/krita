/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "BlendEffectConfigWidget.h"
#include "BlendEffect.h"
#include "KoFilterEffect.h"

#include <kcombobox.h>
#include <klocalizedstring.h>

#include <QGridLayout>
#include <QLabel>

BlendEffectConfigWidget::BlendEffectConfigWidget(QWidget *parent)
    : KoFilterEffectConfigWidgetBase(parent)
    , m_effect(0)
{
    QGridLayout *g = new QGridLayout(this);

    g->addWidget(new QLabel(i18n("Blend mode"), this), 0, 0);
    m_mode = new KComboBox(this);
    m_mode->addItem(i18n("Normal"));
    m_mode->addItem(i18n("Multiply"));
    m_mode->addItem(i18n("Screen"));
    m_mode->addItem(i18n("Darken"));
    m_mode->addItem(i18n("Lighten"));
    g->addWidget(m_mode, 0, 1);
    g->addItem(new QSpacerItem(0, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 1, 0);

    setLayout(g);

    connect(m_mode, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged(int)));
}

bool BlendEffectConfigWidget::editFilterEffect(KoFilterEffect *filterEffect)
{
    m_effect = dynamic_cast<BlendEffect *>(filterEffect);
    if (!m_effect) {
        return false;
    }

    m_mode->blockSignals(true);

    switch (m_effect->blendMode()) {
    case BlendEffect::Normal:
        m_mode->setCurrentIndex(0);
        break;
    case BlendEffect::Multiply:
        m_mode->setCurrentIndex(1);
        break;
    case BlendEffect::Screen:
        m_mode->setCurrentIndex(2);
        break;
    case BlendEffect::Darken:
        m_mode->setCurrentIndex(3);
        break;
    case BlendEffect::Lighten:
        m_mode->setCurrentIndex(4);
        break;
    }

    m_mode->blockSignals(false);

    return true;
}

void BlendEffectConfigWidget::modeChanged(int index)
{
    if (!m_effect) {
        return;
    }

    m_effect->setBlendMode(static_cast<BlendEffect::BlendMode>(index));

    emit filterChanged();
}
