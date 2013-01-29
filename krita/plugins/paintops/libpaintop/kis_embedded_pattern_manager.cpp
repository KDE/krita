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

#include <kis_pattern.h>
#include <kis_properties_configuration.h>
#include <kis_resource_server_provider.h>


struct KisEmbeddedPatternManager::Private {
    static KisPattern* tryFetchPatternByMd5(const QByteArray &md5) {
        KisPattern *pattern = 0;

        if (!md5.isEmpty()) {
            foreach(KoResource *res, KisResourceServerProvider::instance()->patternServer()->resources()) {
                KisPattern *pat = static_cast<KisPattern *>(res);
                if (pat->md5() == md5) {
                    pattern = pat;
                    break;
                }
            }
        }

        return pattern;
    }

    static KisPattern* tryLoadEmbeddedPattern(const KisPropertiesConfiguration* setting) {
        KisPattern *pattern = 0;

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
            pattern = new KisPattern(img, name, KisResourceServerProvider::instance()->patternServer()->saveLocation());
        }

        return pattern;
    }
};


void KisEmbeddedPatternManager::saveEmbeddedPattern(KisPropertiesConfiguration* setting, const KisPattern *pattern)
{
    QByteArray patternMD5 = pattern->md5();
    setting->setProperty("Texture/Pattern/PatternMD5", patternMD5.toBase64());

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    pattern->image().save(&buffer, "PNG");
    setting->setProperty("Texture/Pattern/Pattern", ba.toBase64());

    setting->setProperty("Texture/Pattern/PatternFileName", pattern->filename());
    setting->setProperty("Texture/Pattern/Name", pattern->name());
}

KisPattern* KisEmbeddedPatternManager::loadEmbeddedPattern(const KisPropertiesConfiguration* setting)
{
    KisPattern *pattern = 0;

    QByteArray md5 = QByteArray::fromBase64(setting->getString("Texture/Pattern/PatternMD5").toLatin1());
    pattern = Private::tryFetchPatternByMd5(md5);

    if (!pattern) {
        pattern = Private::tryLoadEmbeddedPattern(setting);
        if (pattern) {
            KisPattern *existingPattern = Private::tryFetchPatternByMd5(pattern->md5());
            if (existingPattern) {
                delete pattern;
                pattern = existingPattern;
            } else {
                KisResourceServerProvider::instance()->patternServer()->addResource(pattern, true);
            }
        }
    }

    return pattern;
}
