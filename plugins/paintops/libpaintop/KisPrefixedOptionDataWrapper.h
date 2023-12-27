/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISPREFIXEDOPTIONDATAWRAPPER_H
#define KISPREFIXEDOPTIONDATAWRAPPER_H

#include <QString>
#include <kis_properties_configuration.h>

/**
 * KisPrefixedOptionDataWrapper wraps an option data type
 * so that it would support prefixed loading/saving. That
 * is mostly used for masked brush features.
 */

template <typename T>
struct KisPrefixedOptionDataWrapper : public T
{
    static constexpr bool supports_prefix = true;

    KisPrefixedOptionDataWrapper(const QString &_prefix)
        : prefix(_prefix)
    {
    }

    bool read(const KisPropertiesConfiguration *setting) {
        if (!setting) return false;

        if (prefix.isEmpty()) {
            return T::read(setting);
        } else {
            KisPropertiesConfiguration prefixedSetting;
            setting->getPrefixedProperties(prefix, &prefixedSetting);
            return T::read(&prefixedSetting);
        }
    }

    void write(KisPropertiesConfiguration *setting) const {
        if (prefix.isEmpty()) {
            T::write(setting);
        } else {
            KisPropertiesConfiguration prefixedSetting;
            T::write(&prefixedSetting);
            setting->setPrefixedProperties(prefix, &prefixedSetting);
        }
    }

    QString prefix;
};

#endif // KISPREFIXEDOPTIONDATAWRAPPER_H
