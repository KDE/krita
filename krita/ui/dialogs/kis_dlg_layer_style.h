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

#include <kdialog.h>

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
#include "ui_wdgOuterGlow.h"
#include "ui_WdgPatternOverlay.h"
#include "ui_WdgSatin.h"
#include "ui_WdgStroke.h"
#include "ui_wdgstylesselector.h"
#include "ui_wdgTexture.h"

class QListWidgetItem;
class KisPSDLayerStyle;
class KisSignalCompressor;

class BevelAndEmboss : public QWidget {
    Q_OBJECT
public:
    BevelAndEmboss(QWidget *parent);
    void setBevelAndEmboss(const psd_layer_effects_bevel_emboss *bevelEmboss);
    void fetchBevelAndEmboss(psd_layer_effects_bevel_emboss *bevelEmboss) const;

Q_SIGNALS:
    void configChanged();

private:
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

class Contour : public QWidget {
    Q_OBJECT
public:
    Contour(QWidget *parent);

Q_SIGNALS:
    void configChanged();

private:
    Ui::WdgContour ui;
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

private Q_SLOTS:
    void slotDialAngleChanged(int value);
    void slotIntAngleChanged(int value);

Q_SIGNALS:
    void configChanged();

private:
    Ui::WdgDropShadow ui;
    Mode m_mode;
};

class GradientOverlay : public QWidget {
    Q_OBJECT
public:
    GradientOverlay(QWidget *parent);
    void setGradientOverlay(const psd_layer_effects_gradient_overlay *gradient);
    void fetchGradientOverlay(psd_layer_effects_gradient_overlay *gradient) const;

private Q_SLOTS:
    void slotDialAngleChanged(int value);
    void slotIntAngleChanged(int value);

Q_SIGNALS:
    void configChanged();

private:
    Ui::WdgGradientOverlay ui;
};

class InnerGlow : public QWidget {
    Q_OBJECT
public:
    InnerGlow(QWidget *parent);
    void setInnerGlow(const psd_layer_effects_inner_glow *innerGlow);
    void fetchInnerGlow(psd_layer_effects_inner_glow *innerGlow) const;

Q_SIGNALS:
    void configChanged();

private:
    Ui::WdgInnerGlow ui;
};

class OuterGlow : public QWidget {
    Q_OBJECT
public:
    OuterGlow(QWidget *parent);
    void setOuterGlow(const psd_layer_effects_outer_glow *outerGlow);
    void fetchOuterGlow(psd_layer_effects_outer_glow *outerGlow) const;

Q_SIGNALS:
    void configChanged();

private:
    Ui::WdgOuterGlow ui;
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

private Q_SLOTS:
    void slotDialAngleChanged(int value);
    void slotIntAngleChanged(int value);

Q_SIGNALS:
    void configChanged();

private:
    Ui::WdgSatin ui;
};

class Stroke : public QWidget {
    Q_OBJECT
public:
    Stroke(QWidget *parent);
    void setStroke(const psd_layer_effects_stroke *stroke);
    void fetchStroke(psd_layer_effects_stroke *stroke) const;

private Q_SLOTS:
    void slotDialAngleChanged(int value);
    void slotIntAngleChanged(int value);

Q_SIGNALS:
    void configChanged();

private:
    Ui::WdgStroke ui;
};

class StylesSelector : public QWidget {
    Q_OBJECT
public:
    StylesSelector(QWidget *parent);
private slots:
    void loadStyles(const QString &name);
    void selectStyle(QListWidgetItem *previous, QListWidgetItem* current);
signals:
    void styleSelected(KisPSDLayerStyleSP style);
private:
    Ui::WdgStylesSelector ui;
};

class Texture : public QWidget {
    Q_OBJECT
public:
    Texture(QWidget *parent);

Q_SIGNALS:
    void configChanged();

private:
    Ui::WdgTexture ui;
};

class KisDlgLayerStyle : public KDialog
{
    Q_OBJECT
public:
    explicit KisDlgLayerStyle(KisPSDLayerStyleSP layerStyle, QWidget *parent = 0);
    ~KisDlgLayerStyle();

signals:
    void configChanged();

public slots:
    void changePage(QListWidgetItem *, QListWidgetItem*);

    void slotNotifyOnAccept();
    void slotNotifyOnReject();

    // Sets all the widgets to the contents of the given style
    void setStyle(KisPSDLayerStyleSP style);
    KisPSDLayerStyleSP style() const;

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
    OuterGlow *m_outerGlow;
    PatternOverlay * m_patternOverlay;
    Satin *m_satin;
    Stroke *m_stroke;
    StylesSelector *m_stylesSelector;
    Texture *m_texture;

    KisSignalCompressor *m_configChangedCompressor;

};

#endif // KIS_DLG_LAYER_STYLE_H
