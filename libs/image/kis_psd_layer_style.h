/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_PSD_LAYER_STYLE_H
#define KIS_PSD_LAYER_STYLE_H

class QIODevice;
class QUuid;

#include <QVector>

#include <psd.h>

#include "kis_types.h"
#include "kritapsd_export.h"

class KisPSDLayerStyle;
typedef QSharedPointer<KisPSDLayerStyle> KisPSDLayerStyleSP;


/**
 * @brief The KisPSDLayerStyle class implements loading, saving and applying
 * the PSD layer effects.
 *
 * See http://www.tonton-pixel.com/Photoshop%20Additional%20File%20Formats/styles-file-format.html
 *
 */
class KRITAPSD_EXPORT KisPSDLayerStyle : public KoResource
{

public:
    explicit KisPSDLayerStyle();
    virtual ~KisPSDLayerStyle();
    KisPSDLayerStyle(const KisPSDLayerStyle& rhs);
    KisPSDLayerStyle operator=(const KisPSDLayerStyle& rhs);

    KoResourceSP clone() const;

    void clear();

    QString name() const;
    void setName(const QString &value);



    QUuid uuid() const;
    void setUuid(const QUuid &value) const;

    QString psdUuid() const;
    void setPsdUuid(const QString &value) const;

    /*
     * KoResource functions
     * they do nothing, just return true
     */
    bool load() override;
    bool loadFromDevice(QIODevice *dev) override;
    bool save() override;
    bool saveToDevice(QIODevice* dev) const override;


    QString resourceType() const override
    {
        return ResourceType::LayerStyles;
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

private:
    struct Private;
    Private * const d;
};

#endif // KIS_PSD_LAYER_STYLE_H
