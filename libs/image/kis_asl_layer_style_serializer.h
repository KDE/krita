/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASL_LAYER_STYLE_SERIALIZER_H
#define __KIS_ASL_LAYER_STYLE_SERIALIZER_H

#include "kritaimage_export.h"

class QIODevice;
class KoPattern;

#include "kis_psd_layer_style.h"
#include "asl/kis_asl_callback_object_catcher.h"

class KRITAIMAGE_EXPORT KisAslLayerStyleSerializer
{
public:
    KisAslLayerStyleSerializer();
    ~KisAslLayerStyleSerializer();

    void saveToDevice(QIODevice *device);
    bool saveToFile(const QString& filename);
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

    bool isValid() {
        return isInitialized() && m_isValid;
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
    bool m_initialized {false};
    bool m_isValid {true};
};

#endif /* __KIS_ASL_LAYER_STYLE_SERIALIZER_H */
