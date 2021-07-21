/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_KRA_LOAD_VISITOR_H_
#define KIS_KRA_LOAD_VISITOR_H_

#include <QRect>
#include <QStringList>

// kritaimage
#include "kis_types.h"
#include "kis_node_visitor.h"

#include "kritalibkra_export.h"

class KisFilterConfiguration;
class KoStore;
class KoShapeControllerBase;
class KoColorProfile;
class KisNodeFilterInterface;

class KRITALIBKRA_EXPORT KisKraLoadVisitor : public KisNodeVisitor
{
public:


    KisKraLoadVisitor(KisImageSP image,
                      KoStore *store,
                      KoShapeControllerBase *shapeController,
                      QMap<KisNode *, QString> &layerFilenames,
                      QMap<KisNode *, QString> &keyframeFilenames,
                      const QString & name,
                      int syntaxVersion);

public:
    void setExternalUri(const QString &uri);

    bool visit(KisNode*) override {
        return true;
    }
    bool visit(KisExternalLayer *) override;
    bool visit(KisPaintLayer *layer) override;
    bool visit(KisGroupLayer *layer) override;
    bool visit(KisAdjustmentLayer* layer) override;
    bool visit(KisGeneratorLayer* layer) override;
    bool visit(KisCloneLayer *layer) override;
    bool visit(KisFilterMask *mask) override;
    bool visit(KisTransformMask *mask) override;
    bool visit(KisTransparencyMask *mask) override;
    bool visit(KisSelectionMask *mask) override;
    bool visit(KisColorizeMask *mask) override;

    QStringList errorMessages() const;
    QStringList warningMessages() const;

private:

    bool loadPaintDevice(KisPaintDeviceSP device, const QString& location);

    template<class DevicePolicy>
    bool loadPaintDeviceFrame(KisPaintDeviceSP device, const QString &location, DevicePolicy policy);

    bool loadProfile(KisPaintDeviceSP device,  const QString& location);
    bool loadFilterConfiguration(KisFilterConfigurationSP kfc, const QString& location);
    const KoColorProfile* loadProfile(const QString& location, const QString &colorModelId, const QString &colorDepthId);
    void fixOldFilterConfigurations(KisFilterConfigurationSP kfc);
    bool loadMetaData(KisNode* node);
    void initSelectionForMask(KisMask *mask);
    bool loadSelection(const QString& location, KisSelectionSP dstSelection);
    QString getLocation(KisNode* node, const QString& suffix = QString());
    QString getLocation(const QString &filename, const QString &suffix = QString());
    void loadNodeKeyframes(KisNode *node);

    /**
     * Load deprecated filters.
     * Most deprecated filters can be handled by this, but the brightnesscontact to perchannels
     * conversion needs to be handled in the perchannel class because those filters
     * have their own xml loading functionality.
     */
    void loadDeprecatedFilter(KisFilterConfigurationSP cfg);

private:
    KisImageSP m_image;
    KoStore *m_store;
    bool m_external;
    QString m_uri;
    QMap<KisNode *, QString> m_layerFilenames;
    QMap<KisNode *, QString> m_keyframeFilenames;
    QString m_name;
    int m_syntaxVersion;
    QStringList m_errorMessages;
    QStringList m_warningMessages;
    KoShapeControllerBase *m_shapeController;
    QMap<QString, const KoColorProfile *> m_profileCache;
};

#endif // KIS_KRA_LOAD_VISITOR_H_

