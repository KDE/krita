/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_embedded_pattern_manager.h"

#include <QBuffer>
#include <QByteArray>

#include <KoResourceServerProvider.h>
#include <resources/KoPattern.h>

#include <kis_properties_configuration.h>


struct KisEmbeddedPatternManager::Private {
    static KoPatternSP tryLoadEmbeddedPattern(const KisPropertiesConfigurationSP setting) {
        KoPatternSP pattern;

        QByteArray ba = QByteArray::fromBase64(setting->getString("Texture/Pattern/Pattern").toLatin1());
        QImage img;
        img.loadFromData(ba, "PNG");

        QString name = setting->getString("Texture/Pattern/Name");
        QString filename = setting->getString("Texture/Pattern/PatternFileName");

        if (name.isEmpty() || name != QFileInfo(name).fileName()) {
            QFileInfo info(filename);
            name = info.baseName();
        }

        if (!img.isNull()) {
            pattern = KoPatternSP(new KoPattern(img, name, KoResourceServerProvider::instance()->patternServer()->saveLocation()));
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

KoPatternSP KisEmbeddedPatternManager::loadEmbeddedPattern(const KisPropertiesConfigurationSP setting)
{
    KoPatternSP pattern;
    KoResourceServer<KoPattern> *server = KoResourceServerProvider::instance()->patternServer();

    QByteArray md5 = QByteArray::fromBase64(setting->getString("Texture/Pattern/PatternMD5").toLatin1());
    pattern = server->resourceByMD5(md5);
    if (pattern) return pattern;

    QString name = setting->getString("Texture/Pattern/Name");
    pattern = server->resourceByName(name);
    if (pattern) return pattern;

    QString fileName = setting->getString("Texture/Pattern/PatternFileName");
    pattern = server->resourceByFilename(fileName);
    if (pattern) return pattern;

    pattern = Private::tryLoadEmbeddedPattern(setting);
    if (pattern) {
        KoResourceServerProvider::instance()->patternServer()->addResource(pattern, false);
    }

    return pattern;
}
