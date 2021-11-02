/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_linked_pattern_manager.h"

#include <QBuffer>
#include <QByteArray>
#include <QApplication>
#include <QThread>

#include <KoResourceServerProvider.h>
#include <resources/KoPattern.h>

#include <kis_properties_configuration.h>
#include <KisResourcesInterface.h>
#include <KoResourceLoadResult.h>

struct KisLinkedPatternManager::Private {

    /// For legacy presets: we now load and save all embedded/linked resources in the kpp file.
    static KoResourceLoadResult tryLoadEmbeddedPattern(const KisPropertiesConfigurationSP setting) {

        QByteArray md5 = QByteArray::fromBase64(setting->getString("Texture/Pattern/PatternMD5").toLatin1());
        QString md5sum = setting->getString("Texture/Pattern/PatternMD5Sum");
        QString fileName = setting->getString("Texture/Pattern/PatternFileName");
        QString name = setting->getString("Texture/Pattern/Name");

        if (md5sum.isEmpty()) {
            md5sum = md5.toHex();
        }

        if (name.isEmpty() || name != QFileInfo(name).fileName()) {
            QFileInfo info(fileName);
            name = info.completeBaseName();
        }

        const QByteArray ba = QByteArray::fromBase64(setting->getString("Texture/Pattern/Pattern").toLatin1());
        return KoEmbeddedResource(KoResourceSignature(ResourceType::Patterns, md5sum, fileName, name), ba);
    }
};

void KisLinkedPatternManager::saveLinkedPattern(KisPropertiesConfigurationSP setting, const KoPatternSP pattern)
{
    setting->setProperty("Texture/Pattern/PatternMD5", QByteArray::fromHex(pattern->md5Sum().toLatin1()));
    setting->setProperty("Texture/Pattern/PatternMD5Sum", pattern->md5Sum());
    setting->setProperty("Texture/Pattern/PatternFileName", pattern->filename());
    setting->setProperty("Texture/Pattern/Name", pattern->name());
}

KoResourceLoadResult KisLinkedPatternManager::tryFetchPattern(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface)
{
    auto resourceSourceAdapter = resourcesInterface->source<KoPattern>(ResourceType::Patterns);

    QByteArray md5 = QByteArray::fromBase64(setting->getString("Texture/Pattern/PatternMD5").toLatin1());
    QString md5sum = setting->getString("Texture/Pattern/PatternMD5Sum");
    QString fileName = setting->getString("Texture/Pattern/PatternFileName");
    QString name = setting->getString("Texture/Pattern/Name");

    if (md5sum.isEmpty()) {
        md5sum = md5.toHex();
    }

    return resourceSourceAdapter.bestMatchLoadResult(md5sum, QFileInfo(fileName).fileName(), name);
}

KoResourceLoadResult KisLinkedPatternManager::loadLinkedPattern(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface)
{
    KoResourceLoadResult result = tryFetchPattern(setting, resourcesInterface);

    if (result.type() == KoResourceLoadResult::FailedLink) {
        result = Private::tryLoadEmbeddedPattern(setting);
    }

    return result;
}
