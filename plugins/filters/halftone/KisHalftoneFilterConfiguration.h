/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_HALFTONE_FILTER_CONFIGURATION_H
#define KIS_HALFTONE_FILTER_CONFIGURATION_H

#include <QHash>
#include <QString>
#include <QStringList>

#include <kis_filter_configuration.h>
#include <KoColor.h>
#include <KoColorSpaceRegistry.h>
#include <generator/kis_generator_registry.h>

class KisHalftoneFilterConfiguration;
typedef KisPinnedSharedPtr<KisHalftoneFilterConfiguration> KisHalftoneFilterConfigurationSP;

class KisHalftoneFilterConfiguration : public KisFilterConfiguration
{
public:
    KisHalftoneFilterConfiguration(const QString & name, qint32 version);
    // KisHalftoneFilterConfiguration(const KisHalftoneFilterConfiguration &rhs);

    ~KisHalftoneFilterConfiguration() override;

    // KisFilterConfigurationSP clone() const override;

    // QList<KoResourceSP> linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const override;
    // QList<KoResourceSP> embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const override;

    static constexpr const char *HalftoneMode_Intensity = "intensity";
    static constexpr const char *HalftoneMode_IndependentChannels = "independent_channels";
    static constexpr const char *HalftoneMode_Alpha = "alpha";

    // default properties
    inline static QString defaultMode() { return HalftoneMode_Intensity; }

    inline static QString defaultGeneratorId()
    {
        static QString defaultGeneratorId;
        if (defaultGeneratorId.isNull()) {
            QStringList generatorIds = KisGeneratorRegistry::instance()->keys();
            if (generatorIds.size() == 0) {
                defaultGeneratorId = "";
            } else {
                generatorIds.sort();
                if (generatorIds.indexOf("screentone") == -1) {
                    defaultGeneratorId = generatorIds.at(0);
                } else {
                    defaultGeneratorId = "screentone";
                }
            }
        }
        return defaultGeneratorId;
    }

    static constexpr qreal defaultHardness() { return 80.0; }

    static constexpr bool defaultInvert() { return false; }

    inline static const KoColor& defaultForegroundColor()
    {
        static const KoColor c(Qt::black, KoColorSpaceRegistry::instance()->rgb8());
        return c;
    }

    inline static const KoColor& defaultBackgroundColor()
    {
        static const KoColor c(Qt::white, KoColorSpaceRegistry::instance()->rgb8());
        return c;
    }

    static constexpr int defaultForegroundOpacity() { return 100; }

    static constexpr int defaultBackgroundOpacity() { return 100; }

    QString colorModelId() const;
    QString mode() const;
    QString generatorId(const QString &prefix) const;
    KisFilterConfigurationSP generatorConfiguration(const QString &prefix) const;
    qreal hardness(const QString &prefix) const;
    bool invert(const QString &prefix) const;
    KoColor foregroundColor(const QString &prefix) const;
    int foregroundOpacity(const QString &prefix) const;
    KoColor backgroundColor(const QString &prefix) const;
    int backgroundOpacity(const QString &prefix) const;

    void setColorModelId(const QString &newColorModelId);
    void setMode(const QString &newMode);
    void setGeneratorId(const QString &prefix, const QString &id);
    void setGeneratorConfiguration(const QString &prefix, KisFilterConfigurationSP config);
    void setHardness(const QString &prefix, qreal newHardness);
    void setInvert(const QString &prefix, bool newInvert);
    void setForegroundColor(const QString &prefix, const KoColor &newForegroundColor);
    void setForegroundOpacity(const QString &prefix, int newOpacity);
    void setBackgroundColor(const QString &prefix, const KoColor &newBackgroundColor);
    void setBackgroundOpacity(const QString &prefix, int newBackgroundOpacity);

// private:
//     QHash<QString, KisFilterConfigurationSP> m_generatorConfigurations;
};

#endif
