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
#include "ui_wdgInnerShadow.h"
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
public:
    BevelAndEmboss(QWidget *parent);
private:
    Ui::WdgBevelAndEmboss ui;
};

class BlendingOptions : public QWidget {
public:
    BlendingOptions(QWidget *parent);
private:
    Ui::WdgBlendingOptions ui;
};


class ColorOverlay : public QWidget {
public:
    ColorOverlay(QWidget *parent);
private:
    Ui::WdgColorOverlay ui;
};

class Contour : public QWidget {
public:
    Contour(QWidget *parent);
private:
    Ui::WdgContour ui;
};


class DropShadow : public QWidget {
    Q_OBJECT
public:
    DropShadow(QWidget *parent);
    void setDropShadow(const psd_layer_effects_drop_shadow &dropShadow);
    psd_layer_effects_drop_shadow dropShadow() const;

private Q_SLOTS:
    void slotDialAngleChanged(int value);
    void slotIntAngleChanged(int value);

Q_SIGNALS:
    void configChanged();

private:
    Ui::WdgDropShadow ui;

};

class GradientOverlay : public QWidget {
public:
    GradientOverlay(QWidget *parent);
private:
    Ui::WdgGradientOverlay ui;
};

class InnerGlow : public QWidget {
public:
    InnerGlow(QWidget *parent);
private:
    Ui::WdgInnerGlow ui;
};

class InnerShadow : public QWidget {
public:
    InnerShadow(QWidget *parent);
private:
    Ui::WdgInnerShadow ui;
};

class OuterGlow : public QWidget {
public:
    OuterGlow(QWidget *parent);
private:
    Ui::WdgOuterGlow ui;
};

class PatternOverlay : public QWidget {
public:
    PatternOverlay(QWidget *parent);
private:
    Ui::WdgPatternOverlay ui;
};

class Satin : public QWidget {
public:
    Satin(QWidget *parent);
private:
    Ui::WdgSatin ui;
};

class Stroke : public QWidget {
public:
    Stroke(QWidget *parent);
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
    void styleSelected(KisPSDLayerStyle *style);
private:
    Ui::WdgStylesSelector ui;
};

class Texture : public QWidget {
public:
    Texture(QWidget *parent);
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
    InnerShadow *m_innerShadow;
    OuterGlow *m_outerGlow;
    PatternOverlay *m_patternOverlay;
    Satin *m_satin;
    Stroke *m_stroke;
    StylesSelector *m_stylesSelector;
    Texture *m_texture;

    KisSignalCompressor *m_configChangedCompressor;

};

#endif // KIS_DLG_LAYER_STYLE_H
