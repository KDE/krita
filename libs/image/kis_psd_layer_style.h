/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_PSD_LAYER_STYLE_H
#define KIS_PSD_LAYER_STYLE_H

class QIODevice;
class QUuid;

#include <QVector>
#include <KoEphemeralResource.h>

#include <psd.h>

#include "kis_types.h"
#include "kritaimage_export.h"

class KisPSDLayerStyle;
typedef QSharedPointer<KisPSDLayerStyle> KisPSDLayerStyleSP;


/**
 * @brief The KisPSDLayerStyle class implements loading, saving and applying
 * the PSD layer effects.
 *
 * See https://www.tonton-pixel.com/Photoshop%20Additional%20File%20Formats/styles-file-format.html
 *
 */
class KRITAIMAGE_EXPORT KisPSDLayerStyle : public KoEphemeralResource<KoResource>
{

public:
    KisPSDLayerStyle(const QString &filename = "", KisResourcesInterfaceSP resourcesInterface = KisResourcesInterfaceSP());
    virtual ~KisPSDLayerStyle();
    KisPSDLayerStyle(const KisPSDLayerStyle& rhs);
    KisPSDLayerStyle operator=(const KisPSDLayerStyle& rhs) = delete;

    KoResourceSP clone() const override;

    void clear();

    QString name() const;
    void setName(const QString &value);

    QUuid uuid() const;
    void setUuid(const QUuid &value) const;

    QString psdUuid() const;
    void setPsdUuid(const QString &value) const;

    QPair<QString, QString> resourceType() const override
    {
        return QPair<QString, QString>(ResourceType::LayerStyles, "");
    }

    /**
     * \return true if all the styles are disabled
     */
    bool isEmpty() const;

    bool isEnabled() const;
    void setEnabled(bool value);

    const psd_layer_effects_context* context() const;
    const psd_layer_effects_drop_shadow* dropShadow() const;
    const psd_layer_effects_inner_shadow* innerShadow() const;
    const psd_layer_effects_outer_glow* outerGlow() const;
    const psd_layer_effects_inner_glow* innerGlow() const;
    const psd_layer_effects_satin* satin() const;
    const psd_layer_effects_color_overlay* colorOverlay() const;
    const psd_layer_effects_gradient_overlay* gradientOverlay() const;
    const psd_layer_effects_pattern_overlay* patternOverlay() const;
    const psd_layer_effects_stroke* stroke() const;
    const psd_layer_effects_bevel_emboss* bevelAndEmboss() const;

    psd_layer_effects_context* context();
    psd_layer_effects_drop_shadow* dropShadow();
    psd_layer_effects_inner_shadow* innerShadow();
    psd_layer_effects_outer_glow* outerGlow();
    psd_layer_effects_inner_glow* innerGlow();
    psd_layer_effects_satin* satin();
    psd_layer_effects_color_overlay* colorOverlay();
    psd_layer_effects_gradient_overlay* gradientOverlay();
    psd_layer_effects_pattern_overlay* patternOverlay();
    psd_layer_effects_stroke* stroke();
    psd_layer_effects_bevel_emboss* bevelAndEmboss();


    /**
     * @return resource interface that is used by KisPSDLayerStyle object for
     * loading linked resources
     */
    KisResourcesInterfaceSP resourcesInterface() const;

    /**
     * Set resource interface that will be used by KisPSDLayerStyle object for
     * loading linked resources
     */
    void setResourcesInterface(KisResourcesInterfaceSP resourcesInterface);

    /**
     * \see KisRequiredResourcesOperators::createLocalResourcesSnapshot
     */
    void createLocalResourcesSnapshot(KisResourcesInterfaceSP globalResourcesInterface = nullptr);

    /**
     * \see KisRequiredResourcesOperators::hasLocalResourcesSnapshot
     */
    bool hasLocalResourcesSnapshot() const;

    /**
     * \see KisRequiredResourcesOperators::cloneWithResourcesSnapshot
     */
    KisPSDLayerStyleSP cloneWithResourcesSnapshot(KisResourcesInterfaceSP globalResourcesInterface = nullptr) const;

    QList<KoResourceSP> embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const override;

private:
    struct Private;
    Private * const d;
};

#endif // KIS_PSD_LAYER_STYLE_H
