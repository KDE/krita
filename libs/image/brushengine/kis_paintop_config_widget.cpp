/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_paintop_config_widget.h"

#include <brushengine/kis_paintop_settings.h>

KisPaintOpConfigWidget::KisPaintOpConfigWidget(QWidget * parent, Qt::WFlags f)
    : KisConfigWidget(parent, f, 10),
      m_userAllowedLod(true)
{
}

KisPaintOpConfigWidget::~KisPaintOpConfigWidget() {
}

void KisPaintOpConfigWidget::writeConfiguration(KisPropertiesConfiguration *config) const {
    KisPaintOpSettings::setLodUserAllowed(config, m_userAllowedLod);
}

void KisPaintOpConfigWidget::setConfiguration(const KisPropertiesConfiguration * config) {
    m_userAllowedLod = KisPaintOpSettings::isLodUserAllowed(config);
    emit sigUserChangedLodAvailability(m_userAllowedLod);
}

void KisPaintOpConfigWidget::setImage(KisImageWSP image) {
    m_image = image;
}

void KisPaintOpConfigWidget::setNode(KisNodeWSP node) {
    m_node = node;
}

void KisPaintOpConfigWidget::changePaintOpSize(qreal x, qreal y) {
    Q_UNUSED(x);
    Q_UNUSED(y);
}

QSizeF KisPaintOpConfigWidget::paintOpSize() const {
    return QSizeF(1.0, 1.0);
}

bool KisPaintOpConfigWidget::presetIsValid() {
    return true;
}

bool KisPaintOpConfigWidget::supportScratchBox() {
    return true;
}

void KisPaintOpConfigWidget::slotUserChangedLodAvailability(bool value) {
    m_userAllowedLod = value;
    emit sigConfigurationItemChanged();
}

void KisPaintOpConfigWidget::coldInitExternalLodAvailabilityWidget()
{
    emit sigUserChangedLodAvailability(m_userAllowedLod);
    emit sigConfigurationItemChanged();
}
