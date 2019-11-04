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
#ifndef KIS_DLG_LAYER_STYLE_H
#define KIS_DLG_LAYER_STYLE_H

#include <QUuid>

#include <KoDialog.h>

#include "kis_types.h"

#include <psd.h>

#include "ui_wdglayerstyles.h"
#include "ui_wdgBevelAndEmboss.h"
#include "ui_wdgblendingoptions.h"
#include "ui_WdgColorOverlay.h"
#include "ui_wdgContour.h"
#include "ui_wdgdropshadow.h"
#include "ui_WdgGradientOverlay.h"
#include "ui_wdgInnerGlow.h"
#include "ui_WdgPatternOverlay.h"
#include "ui_WdgSatin.h"
#include "ui_WdgStroke.h"
#include "ui_wdgstylesselector.h"
#include "ui_wdgTexture.h"

#include <kis_psd_layer_style.h>

class QListWidgetItem;
class KisSignalCompressor;
class KisCanvasResourceProvider;


class Contour : public QWidget {
    Q_OBJECT
public:
    Contour(QWidget *parent);
    Ui::WdgContour ui;
};


class Texture : public QWidget {
    Q_OBJECT
public:
    Texture(QWidget *parent);
    Ui::WdgTexture ui;
};


class BevelAndEmboss : public QWidget {
    Q_OBJECT
public:
    BevelAndEmboss(Contour *contour, Texture *texture, QWidget *parent);
    void setBevelAndEmboss(const psd_layer_effects_bevel_emboss *bevelAndEmboss);
    void fetchBevelAndEmboss(psd_layer_effects_bevel_emboss *bevelAndEmboss) const;

Q_SIGNALS:
    void configChanged();
    void globalAngleChanged(int value);

private:
    Contour *m_contour;
    Texture *m_texture;
    Ui::WdgBevelAndEmboss ui;
};

class BlendingOptions : public QWidget {
    Q_OBJECT
public:
    BlendingOptions(QWidget *parent);
Q_SIGNALS:
    void configChanged();

private:
    Ui::WdgBlendingOptions ui;
};


class ColorOverlay : public QWidget {
    Q_OBJECT
public:
    ColorOverlay(QWidget *parent);
    void setColorOverlay(const psd_layer_effects_color_overlay *colorOverlay);
    void fetchColorOverlay(psd_layer_effects_color_overlay *colorOverlay) const;

Q_SIGNALS:
    void configChanged();

private:
    Ui::WdgColorOverlay ui;
};


class DropShadow : public QWidget {
    Q_OBJECT

public:
    enum Mode {
        DropShadowMode,
        InnerShadowMode
    };

public:
    DropShadow(Mode mode, QWidget *parent);
    void setShadow(const psd_layer_effects_shadow_common *shadow);
    void fetchShadow(psd_layer_effects_shadow_common *shadow) const;

Q_SIGNALS:
    void configChanged();
    void globalAngleChanged(int value);

private:
    Ui::WdgDropShadow ui;
    Mode m_mode;
};

class GradientOverlay : public QWidget {
    Q_OBJECT
public:
    GradientOverlay(KisCanvasResourceProvider *resourceProvider, QWidget *parent);
    void setGradientOverlay(const psd_layer_effects_gradient_overlay *gradient);
    void fetchGradientOverlay(psd_layer_effects_gradient_overlay *gradient) const;

Q_SIGNALS:
    void configChanged();

private:
    Ui::WdgGradientOverlay ui;
    KisCanvasResourceProvider *m_resourceProvider;
};

class InnerGlow : public QWidget {
    Q_OBJECT
public:
    enum Mode {
        InnerGlowMode = 0,
        OuterGlowMode
    };

public:
    InnerGlow(Mode mode, KisCanvasResourceProvider *resourceProvider, QWidget *parent);
    void setConfig(const psd_layer_effects_glow_common *innerGlow);
    void fetchConfig(psd_layer_effects_glow_common *innerGlow) const;

Q_SIGNALS:
    void configChanged();

private:
    Ui::WdgInnerGlow ui;
    Mode m_mode;
    KisCanvasResourceProvider *m_resourceProvider;
};

class PatternOverlay : public QWidget {
    Q_OBJECT
public:
    PatternOverlay(QWidget *parent);
    void setPatternOverlay(const psd_layer_effects_pattern_overlay *pattern);
    void fetchPatternOverlay(psd_layer_effects_pattern_overlay *pattern) const;

Q_SIGNALS:
    void configChanged();

private:
    Ui::WdgPatternOverlay ui;
};

class Satin : public QWidget {
    Q_OBJECT
public:
    Satin(QWidget *parent);
    void setSatin(const psd_layer_effects_satin *satin);
    void fetchSatin(psd_layer_effects_satin *satin) const;

Q_SIGNALS:
    void configChanged();

private:
    Ui::WdgSatin ui;
};

class Stroke : public QWidget {
    Q_OBJECT
public:
    Stroke(KisCanvasResourceProvider *resourceProvider, QWidget *parent);
    void setStroke(const psd_layer_effects_stroke *stroke);
    void fetchStroke(psd_layer_effects_stroke *stroke) const;

Q_SIGNALS:
    void configChanged();

private:
    Ui::WdgStroke ui;
    KisCanvasResourceProvider *m_resourceProvider;
};

class StylesSelector : public QWidget {
    Q_OBJECT
public:
    StylesSelector(QWidget *parent);

    void notifyExternalStyleChanged(const QString &name, const QUuid &uuid);

    void addNewStyle(KisPSDLayerStyleSP style);
    void loadCollection(const QString &fileName);

private Q_SLOTS:
    void loadStyles(const QString &name);
    void selectStyle(QListWidgetItem *previous, QListWidgetItem* current);
Q_SIGNALS:
    void styleSelected(KisPSDLayerStyleSP style);

private:
    void refillCollections();

private:
    Ui::WdgStylesSelector ui;
};

class KisDlgLayerStyle : public KoDialog
{
    Q_OBJECT
public:
    explicit KisDlgLayerStyle(KisPSDLayerStyleSP layerStyle, KisCanvasResourceProvider *resourceProvider, QWidget *parent = 0);
    ~KisDlgLayerStyle() override;

    KisPSDLayerStyleSP style() const;

Q_SIGNALS:
    void configChanged();

public Q_SLOTS:
    void slotMasterFxSwitchChanged(bool value);
    void syncGlobalAngle(int angle);

    void notifyGuiConfigChanged();
    void notifyPredefinedStyleSelected(KisPSDLayerStyleSP style);

    void slotBevelAndEmbossChanged(QListWidgetItem*);

    void changePage(QListWidgetItem *, QListWidgetItem*);

    void slotNotifyOnAccept();
    void slotNotifyOnReject();

    // Sets all the widgets to the contents of the given style
    void setStyle(KisPSDLayerStyleSP style);

    void slotLoadStyle();
    void slotSaveStyle();
    void slotNewStyle();

private:

    KisPSDLayerStyleSP m_layerStyle;
    KisPSDLayerStyleSP m_initialLayerStyle;

    Ui::WdgStylesDialog wdgLayerStyles;

    BevelAndEmboss *m_bevelAndEmboss;
    BlendingOptions *m_blendingOptions;
    ColorOverlay *m_colorOverlay;
    Contour *m_contour;
    DropShadow *m_dropShadow;
    GradientOverlay *m_gradientOverlay;
    InnerGlow *m_innerGlow;
    DropShadow *m_innerShadow;
    InnerGlow *m_outerGlow;
    PatternOverlay * m_patternOverlay;
    Satin *m_satin;
    Stroke *m_stroke;
    StylesSelector *m_stylesSelector;
    Texture *m_texture;

    KisSignalCompressor *m_configChangedCompressor;
    bool m_isSwitchingPredefinedStyle;

    /**
     * Used for debugging purposes only to track if m_layerStyle is in
     * sync with what is stored in the GUI
     */
    mutable bool m_sanityLayerStyleDirty;
};

#endif // KIS_DLG_LAYER_STYLE_H
