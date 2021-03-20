/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
