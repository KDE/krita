/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisPaintopPropertiesBase.h"
#include "kis_properties_configuration.h"
#include <KisResourcesInterface.h>


KisPaintopPropertiesResourcesBase::~KisPaintopPropertiesResourcesBase()
{

}

void KisPaintopPropertiesResourcesBase::readOptionSetting(KisPropertiesConfigurationSP settings, KisResourcesInterfaceSP resourcesInterface)
{
    readOptionSettingResourceImpl(settings.data(), resourcesInterface);
}

void KisPaintopPropertiesResourcesBase::writeOptionSetting(KisPropertiesConfigurationSP settings) const
{
    writeOptionSettingImpl(settings.data());
}

void KisPaintopPropertiesResourcesBase::readOptionSetting(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface)
{
    readOptionSettingResourceImpl(settings, resourcesInterface);
}

void KisPaintopPropertiesResourcesBase::writeOptionSetting(KisPropertiesConfiguration *settings) const
{
    writeOptionSettingImpl(settings);
}

QList<KoResourceSP> KisPaintopPropertiesResourcesBase::prepareLinkedResources(const KisPropertiesConfigurationSP settings, KisResourcesInterfaceSP resourcesInterface) const
{
    return prepareLinkedResourcesImpl(settings.data(), resourcesInterface);
}

QList<KoResourceSP> KisPaintopPropertiesResourcesBase::prepareLinkedResources(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface) const
{
    return prepareLinkedResourcesImpl(settings, resourcesInterface);
}

QList<KoResourceSP> KisPaintopPropertiesResourcesBase::prepareEmbeddedResources(const KisPropertiesConfigurationSP settings, KisResourcesInterfaceSP resourcesInterface) const
{
    return prepareEmbeddedResources(settings.data(), resourcesInterface);
}

QList<KoResourceSP> KisPaintopPropertiesResourcesBase::prepareEmbeddedResources(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface) const
{
    return prepareEmbeddedResources(settings, resourcesInterface);
}

void KisPaintopPropertiesBase::readOptionSetting(KisPropertiesConfigurationSP settings)
{
    readOptionSettingImpl(settings.data());
}

void KisPaintopPropertiesBase::readOptionSetting(const KisPropertiesConfiguration *settings)
{
    readOptionSettingImpl(settings);
}

void KisPaintopPropertiesBase::readOptionSettingResourceImpl(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(resourcesInterface);
    readOptionSettingImpl(settings);
}

QList<KoResourceSP> KisPaintopPropertiesBase::prepareLinkedResourcesImpl(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface) const
{
    Q_UNUSED(settings);
    Q_UNUSED(resourcesInterface);
    return {};
}

QList<KoResourceSP> KisPaintopPropertiesBase::prepareEmbeddedResourcesImpl(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface) const
{
    Q_UNUSED(settings);
    Q_UNUSED(resourcesInterface);
    return {};
}
