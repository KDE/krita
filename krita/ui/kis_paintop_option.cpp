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

#include <klocale.h>

QString KisPaintOpOption::generalCategory()
{
    return i18n("General");
}

QString KisPaintOpOption::colorCategory()
{
    return i18n("Color");
}

QString KisPaintOpOption::textureCategory()
{
    return i18n("Texture");
}


struct KisPaintOpOption::Private
{
public:
    bool checked;
    QString label;
    QString category;
    QWidget * configurationPage;

    bool updatesBlocked;
    bool isWritingSettings;
};

KisPaintOpOption::KisPaintOpOption(const QString & label, const QString& category, bool checked)
        : m_checkable(true)
        , m_d(new Private())

{
    m_d->checked = checked;
    m_d->label = label;
    m_d->category = category;
    m_d->configurationPage = 0;
    m_d->updatesBlocked = false;
    m_d->isWritingSettings = false;

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

void KisPaintOpOption::startReadOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_d->updatesBlocked = true;
    readOptionSetting(setting);
    m_d->updatesBlocked = false;
}

void KisPaintOpOption::startWriteOptionSetting(KisPropertiesConfiguration* setting) const
{
    m_d->isWritingSettings = true;
    writeOptionSetting(setting);
    m_d->isWritingSettings = false;
}

QString KisPaintOpOption::label() const
{
    return m_d->label;
}

QString KisPaintOpOption::category() const
{
    return m_d->category;
}

bool KisPaintOpOption::isChecked() const
{
    return m_d->checked;
}

void KisPaintOpOption::setChecked(bool checked)
{
    m_d->checked = checked;
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


#include "kis_paintop_option.moc"

