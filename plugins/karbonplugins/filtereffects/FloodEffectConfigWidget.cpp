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

#include "FloodEffectConfigWidget.h"
#include "FloodEffect.h"
#include "KoFilterEffect.h"

#include <KoColorPopupAction.h>

#include <klocalizedstring.h>

#include <QGridLayout>
#include <QLabel>
#include <QToolButton>

FloodEffectConfigWidget::FloodEffectConfigWidget(QWidget *parent)
    : KoFilterEffectConfigWidgetBase(parent)
    , m_effect(0)
{
    QGridLayout *g = new QGridLayout(this);

    g->addWidget(new QLabel(i18n("Flood color"), this), 0, 0);
    QToolButton *button = new QToolButton(this);
    g->addWidget(button, 0, 1);
    m_actionStopColor = new KoColorPopupAction(this);
    button->setDefaultAction(m_actionStopColor);
    setLayout(g);

    connect(m_actionStopColor, SIGNAL(colorChanged(KoColor)), this, SLOT(colorChanged()));
}

bool FloodEffectConfigWidget::editFilterEffect(KoFilterEffect *filterEffect)
{
    m_effect = dynamic_cast<FloodEffect *>(filterEffect);
    if (!m_effect) {
        return false;
    }

    m_actionStopColor->setCurrentColor(m_effect->floodColor());
    return true;
}

void FloodEffectConfigWidget::colorChanged()
{
    if (!m_effect) {
        return;
    }

    m_effect->setFloodColor(m_actionStopColor->currentColor());
    emit filterChanged();
}
