/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_ASL_LAYER_STYLE_SERIALIZER_H
#define __KIS_ASL_LAYER_STYLE_SERIALIZER_H

#include "kritapsd_export.h"

class QIODevice;
class KoPattern;

#include "kis_psd_layer_style.h"
#include "asl/kis_asl_callback_object_catcher.h"


class KRITAPSD_EXPORT KisAslLayerStyleSerializer
{
public:
    KisAslLayerStyleSerializer();
    ~KisAslLayerStyleSerializer();

    void saveToDevice(QIODevice *device);
    void readFromDevice(QIODevice *device);
    bool readFromFile(const QString& filename);

    QVector<KisPSDLayerStyleSP> styles() const;
    void setStyles(const QVector<KisPSDLayerStyleSP> &styles);

    QHash<QString, KoPatternSP> patterns() const;
    QHash<QString, KisPSDLayerStyleSP> stylesHash();


    void registerPSDPattern(const QDomDocument &doc);
    void readFromPSDXML(const QDomDocument &doc);

    QDomDocument formXmlDocument() const;
    QDomDocument formPsdXmlDocument() const;

    bool isInitialized() {
        return m_initialized;
    }


private:
    void registerPatternObject(const KoPatternSP pattern, const  QString& patternUuid);

    void assignPatternObject(const QString &patternUuid,
                             const QString &patternName,
                             boost::function<void (KoPatternSP )> setPattern);

    QVector<KoPatternSP> fetchAllPatterns(KisPSDLayerStyle *style) const;

    void newStyleStarted(bool isPsdStructure);
    void connectCatcherToStyle(KisPSDLayerStyle *style, const QString &prefix);

private:
    QHash<QString, KoPatternSP> m_patternsStore;

    KisAslCallbackObjectCatcher m_catcher;
    QVector<KisPSDLayerStyleSP> m_stylesVector;
    QHash<QString, KisPSDLayerStyleSP> m_stylesHash;
    bool m_initialized;
};

#endif /* __KIS_ASL_LAYER_STYLE_SERIALIZER_H */
