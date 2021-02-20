/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_paintop_option.h"

#include <klocalizedstring.h>

#include <KisResourcesInterface.h>
#include <KoCanvasResourcesInterface.h>

struct KisPaintOpOption::Private
{
public:
    bool checked;
    QString label;
    KisPaintOpOption::PaintopCategory category;
    QWidget *configurationPage;

    bool updatesBlocked;
    bool isWritingSettings;

    KisResourcesInterfaceSP resourcesInterface;
    KoCanvasResourcesInterfaceSP canvasResourcesInterface;
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

void KisPaintOpOption::setResourcesInterface(KisResourcesInterfaceSP resourcesInterface)
{
    m_d->resourcesInterface = resourcesInterface;
}

void KisPaintOpOption::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    m_d->canvasResourcesInterface = canvasResourcesInterface;
}

KisResourcesInterfaceSP KisPaintOpOption::resourcesInterface() const
{
    return m_d->resourcesInterface;
}

KoCanvasResourcesInterfaceSP KisPaintOpOption::canvasResourcesInterface() const
{
    return m_d->canvasResourcesInterface;
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



