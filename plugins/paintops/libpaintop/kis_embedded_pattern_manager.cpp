/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_embedded_pattern_manager.h"

#include <QBuffer>
#include <QByteArray>

#include <KoResourceServerProvider.h>
#include <resources/KoPattern.h>

#include <kis_properties_configuration.h>
#include <KisResourcesInterface.h>


struct KisEmbeddedPatternManager::Private {
    static KoPatternSP tryLoadEmbeddedPattern(const KisPropertiesConfigurationSP setting) {
        KoPatternSP pattern;

        QByteArray ba = QByteArray::fromBase64(setting->getString("Texture/Pattern/Pattern").toLatin1());
        QImage img;
        img.loadFromData(ba, "PNG");

        QString name = setting->getString("Texture/Pattern/Name");
        QString filename = QFileInfo(setting->getString("Texture/Pattern/PatternFileName")).fileName(); // For broken embedded patters like in "i)_Wet_Paint"

        if (name.isEmpty() || name != QFileInfo(name).fileName()) {
            QFileInfo info(filename);
            name = info.completeBaseName();
        }

        if (!img.isNull()) {
            pattern = KoPatternSP(new KoPattern(img, name, filename));
        }

        return pattern;
    }
};

void KisEmbeddedPatternManager::saveEmbeddedPattern(KisPropertiesConfigurationSP setting, const KoPatternSP pattern)
{
    QByteArray patternMD5 = pattern->md5();

    /**
     * The process of saving a pattern may be quite expensive, so
     * we won't rewrite the pattern if has the same md5-sum and at
     * least some data is present
     */
    QByteArray existingMD5 = QByteArray::fromBase64(setting->getString("Texture/Pattern/PatternMD5").toLatin1());
    QString existingPatternBase64 = setting->getString("Texture/Pattern/PatternMD5").toLatin1();

    if (patternMD5 == existingMD5 && !existingPatternBase64.isEmpty()) {
        return;
    }
    setting->setProperty("Texture/Pattern/PatternMD5", patternMD5.toBase64());

    setting->setProperty("Texture/Pattern/PatternFileName", pattern->filename());
    setting->setProperty("Texture/Pattern/Name", pattern->name());

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    pattern->pattern().save(&buffer, "PNG");
    setting->setProperty("Texture/Pattern/Pattern", ba.toBase64());
}

KoPatternSP KisEmbeddedPatternManager::tryFetchPattern(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface)
{
    KoPatternSP pattern;
    auto resourceSourceAdapter = resourcesInterface->source<KoPattern>(ResourceType::Patterns);

    QByteArray md5 = QByteArray::fromBase64(setting->getString("Texture/Pattern/PatternMD5").toLatin1());
    pattern = resourceSourceAdapter.resourceForMD5(md5);
    if (pattern) return pattern;

    QString name = setting->getString("Texture/Pattern/Name");
    if (!name.isEmpty()) {
        pattern = resourceSourceAdapter.resourceForName(name);
    }
    if (pattern) return pattern;

    QString fileName = setting->getString("Texture/Pattern/PatternFileName");
    pattern = resourceSourceAdapter.resourceForFilename(QFileInfo(fileName).fileName());

    return pattern;
}

KoPatternSP KisEmbeddedPatternManager::loadEmbeddedPattern(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface)
{
    KoPatternSP pattern = tryFetchPattern(setting, resourcesInterface);
    if (pattern) return pattern;

    pattern = Private::tryLoadEmbeddedPattern(setting);
    // this resource will be added to the memory storage by KisResourceLocator

    return pattern;
}
