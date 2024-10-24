/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisEmbeddedTextureData.h"

#include <QFileInfo>
#include <kis_properties_configuration.h>
#include <KoResourceLoadResult.h>
#include <KisResourcesInterface.h>


bool KisEmbeddedTextureData::isNull() const
{
    return md5Base64.isEmpty() && md5sum.isEmpty() && fileName.isEmpty() && name.isEmpty();
}

KisEmbeddedTextureData KisEmbeddedTextureData::fromPattern(KoPatternSP pattern)
{
    KisEmbeddedTextureData data;

    data.md5Base64 = QByteArray::fromHex(pattern->md5Sum().toLatin1());
    data.md5sum = pattern->md5Sum();
    data.fileName = pattern->filename();
    data.name = pattern->name();

    return data;
}


KoResourceLoadResult KisEmbeddedTextureData::tryFetchPattern(KisResourcesInterfaceSP resourcesInterface) const
{
    auto resourceSourceAdapter = resourcesInterface->source<KoPattern>(ResourceType::Patterns);

    QString effectiveMd5Sum = md5sum;

    if (effectiveMd5Sum.isEmpty()) {
        const QByteArray md5 = QByteArray::fromBase64(md5Base64.toLatin1());
        effectiveMd5Sum = md5.toHex();
    }

    return resourceSourceAdapter.bestMatchLoadResult(effectiveMd5Sum, fileName, name);
}

KoResourceLoadResult KisEmbeddedTextureData::tryLoadEmbeddedPattern() const
{
    QString effectiveMd5Sum = md5sum;

    if (effectiveMd5Sum.isEmpty()) {
        const QByteArray md5 = QByteArray::fromBase64(md5Base64.toLatin1());
        effectiveMd5Sum = md5.toHex();
    }

    QString effectiveName = name;

    if (effectiveName.isEmpty() || effectiveName != QFileInfo(effectiveName).fileName()) {
        QFileInfo info(effectiveName);
        effectiveName = info.completeBaseName();
    }

    KIS_SAFE_ASSERT_RECOVER(!patternBase64.isEmpty()) {
        // return a fail-link pattern
        return KoResourceSignature(ResourceType::Patterns, effectiveMd5Sum, fileName, effectiveName);
    }

    const QByteArray ba = QByteArray::fromBase64(patternBase64.toLatin1());
    return KoEmbeddedResource(KoResourceSignature(ResourceType::Patterns, effectiveMd5Sum, fileName, effectiveName), ba);
}


KoResourceLoadResult KisEmbeddedTextureData::loadLinkedPattern(KisResourcesInterfaceSP resourcesInterface) const
{
    KoResourceLoadResult result = tryFetchPattern(resourcesInterface);

    if (result.type() == KoResourceLoadResult::FailedLink && !patternBase64.isEmpty()) {
        result = tryLoadEmbeddedPattern();
    }

    return result;
}

bool KisEmbeddedTextureData::read(const KisPropertiesConfiguration *setting)
{
    md5Base64 = setting->getString("Texture/Pattern/PatternMD5");
    md5sum = setting->getString("Texture/Pattern/PatternMD5Sum");
    fileName = QFileInfo(setting->getString("Texture/Pattern/PatternFileName")).fileName();
    name = setting->getString("Texture/Pattern/Name");
    patternBase64 = setting->getString("Texture/Pattern/Pattern");

    return true;
}

void KisEmbeddedTextureData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty("Texture/Pattern/PatternMD5", md5Base64);
    setting->setProperty("Texture/Pattern/PatternMD5Sum", md5sum);
    setting->setProperty("Texture/Pattern/PatternFileName", fileName);
    setting->setProperty("Texture/Pattern/Name", name);
}
