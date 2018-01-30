/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISPREFIXEDPAINTOPOPTIONWRAPPER_H
#define KISPREFIXEDPAINTOPOPTIONWRAPPER_H

#include "kis_properties_configuration.h"

template <class Base>
class KisPrefixedPaintOpOptionWrapper : public Base
{
public:
    KisPrefixedPaintOpOptionWrapper(const QString &prefix)
        : m_prefix(prefix) {}

    template<typename... Args>
    KisPrefixedPaintOpOptionWrapper(const QString &prefix, Args... args)
        : Base(args...),
          m_prefix(prefix) {}

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const {
        // TODO: check if preinitialization for embeddedSettings is needed (for locked settings)
        KisPropertiesConfigurationSP embeddedConfig = new KisPropertiesConfiguration();
        Base::writeOptionSetting(embeddedConfig);
        setting->setPrefixedProperties(m_prefix, embeddedConfig);
    }

    void readOptionSetting(const KisPropertiesConfigurationSP setting) {
        KisPropertiesConfigurationSP embeddedConfig = new KisPropertiesConfiguration();
        setting->getPrefixedProperties(m_prefix, embeddedConfig);
        Base::readOptionSetting(embeddedConfig);
    }

private:
    const QString m_prefix;
};

#endif // KISPREFIXEDPAINTOPOPTIONWRAPPER_H
