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

#include "kis_paintop_option.h"

#include <klocalizedstring.h>

struct KisPaintOpOption::Private
{
public:
    bool checked;
    QString label;
    KisPaintOpOption::PaintopCategory category;
    QWidget *configurationPage;

    bool updatesBlocked;
    bool isWritingSettings;
};

KisPaintOpOption::KisPaintOpOption(PaintopCategory category, bool checked)
    : m_checkable(true)
    , m_d(new Private())

{
    m_d->checked = checked;
    m_d->category = category;
    m_d->updatesBlocked = false;
    m_d->isWritingSettings = false;
    m_d->configurationPage = 0;
}

KisPaintOpOption::~KisPaintOpOption()
{
    delete m_d;
}

void KisPaintOpOption::emitSettingChanged()
{
    KIS_ASSERT_RECOVER_RETURN(!m_d->isWritingSettings);

    if (!m_d->updatesBlocked) {
        emit sigSettingChanged();
    }
}

void KisPaintOpOption::emitCheckedChanged()
{
    KIS_ASSERT_RECOVER_RETURN(!m_d->isWritingSettings);

    if (!m_d->updatesBlocked) {
        emit sigCheckedChanged(isChecked());
    }
}

void KisPaintOpOption::startReadOptionSetting(const KisPropertiesConfigurationSP setting)
{
    m_d->updatesBlocked = true;
    readOptionSetting(setting);
    m_d->updatesBlocked = false;
}

void KisPaintOpOption::startWriteOptionSetting(KisPropertiesConfigurationSP setting) const
{
    m_d->isWritingSettings = true;
    writeOptionSetting(setting);
    m_d->isWritingSettings = false;
}

void KisPaintOpOption::lodLimitations(KisPaintopLodLimitations *l) const
{
    Q_UNUSED(l);
}

KisPaintOpOption::PaintopCategory KisPaintOpOption::category() const
{
    return m_d->category;
}

bool KisPaintOpOption::isChecked() const
{
    return m_d->checked;
}

bool KisPaintOpOption::isCheckable() const {
    return m_checkable;
}

void KisPaintOpOption::setChecked(bool checked)
{
    m_d->checked = checked;

    emitCheckedChanged();
    emitSettingChanged();
}

void KisPaintOpOption::setImage(KisImageWSP image)
{
    Q_UNUSED(image);
}

void KisPaintOpOption::setNode(KisNodeWSP node)
{
    Q_UNUSED(node);
}

void KisPaintOpOption::setConfigurationPage(QWidget * page)
{
    m_d->configurationPage = page;
}

QWidget* KisPaintOpOption::configurationPage() const
{
    return m_d->configurationPage;
}
void KisPaintOpOption::setLocked(bool value)
{
    m_locked = value;
}

bool KisPaintOpOption::isLocked ()const
{
    return m_locked;
}



