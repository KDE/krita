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

QString KisPaintOpOption::brushCategory()
{
    return i18n("Brush");
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
};

KisPaintOpOption::KisPaintOpOption(const QString & label, const QString& category, bool checked)
        : m_checkable(true)
        , m_d(new Private())
{
    m_d->checked = checked;
    m_d->label = label;
    m_d->category = category;
    m_d->configurationPage = 0;
}

KisPaintOpOption::~KisPaintOpOption()
{
    delete m_d;
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
    emit sigSettingChanged();
}

void KisPaintOpOption::setImage(KisImageWSP image)
{
    Q_UNUSED(image);
}

void KisPaintOpOption::setConfigurationPage(QWidget * page)
{
    m_d->configurationPage = page;
}

QWidget * KisPaintOpOption::configurationPage() const
{
    return m_d->configurationPage;
}

#include "kis_paintop_option.moc"

