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
#include "kis_dlg_layer_style.h"

#include <QWidget>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QComboBox>
#include <QDial>
#include <QCheckBox>
#include <QSpinBox>

#include "kis_config.h"

#include "kis_resource_server_provider.h"
#include "kis_psd_layer_style_resource.h"
#include "kis_psd_layer_style.h"

KisDlgLayerStyle::KisDlgLayerStyle(KisPSDLayerStyle *layerStyle, QWidget *parent)
    : KDialog(parent)
    , m_layerStyle(layerStyle)
{
    setCaption(i18n("Layer Styles"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    QWidget *page = new QWidget(this);
    wdgLayerStyles.setupUi(page);
    setMainWidget(page);

    m_stylesSelector = new StylesSelector(this);
    connect(m_stylesSelector, SIGNAL(styleSelected(KisPSDLayerStyle*)), SLOT(setStyle(KisPSDLayerStyle*)));
    wdgLayerStyles.stylesStack->addWidget(m_stylesSelector);

    m_blendingOptions = new BlendingOptions(this);
    wdgLayerStyles.stylesStack->addWidget(m_blendingOptions);
    m_dropShadow = new DropShadow(this);
    wdgLayerStyles.stylesStack->addWidget(m_dropShadow);
    m_innerShadow = new InnerShadow(this);
    wdgLayerStyles.stylesStack->addWidget(m_innerShadow);
    m_outerGlow = new OuterGlow(this);
    wdgLayerStyles.stylesStack->addWidget(m_outerGlow);
    m_innerGlow = new InnerGlow(this);
    wdgLayerStyles.stylesStack->addWidget(m_innerGlow);
    m_bevelAndEmboss = new BevelAndEmboss(this);
    wdgLayerStyles.stylesStack->addWidget(m_bevelAndEmboss);
    m_contour = new Contour(this);
    wdgLayerStyles.stylesStack->addWidget(m_contour);
    m_texture = new Texture(this);
    wdgLayerStyles.stylesStack->addWidget(m_texture);
    m_satin = new Satin(this);
    wdgLayerStyles.stylesStack->addWidget(m_satin);
    m_colorOverlay = new ColorOverlay(this);
    wdgLayerStyles.stylesStack->addWidget(m_colorOverlay);
    m_gradientOverlay = new GradientOverlay(this);
    wdgLayerStyles.stylesStack->addWidget(m_gradientOverlay);
    m_patternOverlay = new PatternOverlay(this);
    wdgLayerStyles.stylesStack->addWidget(m_patternOverlay);
    m_stroke = new Stroke(this);
    wdgLayerStyles.stylesStack->addWidget(m_stroke);

    KisConfig cfg;
    wdgLayerStyles.stylesStack->setCurrentIndex(cfg.readEntry("KisDlgLayerStyle::current", 1));
    wdgLayerStyles.lstStyleSelector->setCurrentRow(cfg.readEntry("KisDlgLayerStyle::current", 1));

    connect(wdgLayerStyles.lstStyleSelector,
            SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
             this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));

    setStyle(layerStyle);
}

void KisDlgLayerStyle::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current) {
        current = previous;
    }
    wdgLayerStyles.stylesStack->setCurrentIndex(wdgLayerStyles.lstStyleSelector->row(current));
}

void KisDlgLayerStyle::setStyle(KisPSDLayerStyle *style)
{
    QListWidgetItem *item = wdgLayerStyles.lstStyleSelector->item(2);
    item->setCheckState(style->dropShadow().effect_enable ? Qt::Checked : Qt::Unchecked);
    m_dropShadow->setDropShadow(style->dropShadow());
}

KisPSDLayerStyle *KisDlgLayerStyle::style() const
{
    m_layerStyle->setDropShadow(m_dropShadow->dropShadow());

    return m_layerStyle;
}

BevelAndEmboss::BevelAndEmboss(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


BlendingOptions::BlendingOptions(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


ColorOverlay::ColorOverlay(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


Contour::Contour(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


DropShadow::DropShadow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.doubleOpacity->setRange(0, 100, 0);
    ui.doubleOpacity->setSuffix("%");

}

void DropShadow::setDropShadow(const psd_layer_effects_drop_shadow &dropShadow)
{
    ui.chkLayerKnocksOutDropShadow->setChecked(dropShadow.knocks_out);
    //ui.cmbContour;
    ui.chkAntiAliased->setChecked(dropShadow.anti_aliased);
    ui.intNoise->setValue(dropShadow.noise);
    ui.cmbCompositeOp->selectCompositeOp(KoID(dropShadow.blend_mode));
    ui.doubleOpacity->setValue(dropShadow.opacity);
    ui.dialAngle->setValue(dropShadow.angle);
    ui.intAngle->setValue(dropShadow.angle);
    ui.chkUseGlobalLight->setChecked(dropShadow.use_global_light);
    ui.intDistance->setValue(dropShadow.distance);
    ui.intSpread->setValue(dropShadow.spread);
    ui.intSize->setValue(dropShadow.size);
}

psd_layer_effects_drop_shadow DropShadow::dropShadow() const
{
    psd_layer_effects_drop_shadow ds;
    ds.knocks_out = ui.chkLayerKnocksOutDropShadow->isChecked();
    // ui.cmbContour;
    ds.anti_aliased = ui.chkAntiAliased->isChecked();
    ds.noise = ui.intNoise->value();
    ds.blend_mode = ui.cmbCompositeOp->selectedCompositeOp().id();
    ds.opacity = ui.doubleOpacity->value();
    ds.angle = ui.dialAngle->value();
    ds.use_global_light = ui.chkUseGlobalLight->isChecked();
    ds.distance = ui.intDistance->value();
    ds.spread = ui.intSpread->value();
    ds.size = ui.intSize->value();

    return ds;
}


GradientOverlay::GradientOverlay(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


InnerGlow::InnerGlow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


InnerShadow::InnerShadow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


PatternOverlay::PatternOverlay(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


Satin::Satin(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


Stroke::Stroke(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

StylesSelector::StylesSelector(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    connect(ui.cmbStyleCollections, SIGNAL(activated(QString)), this, SLOT(loadStyles(QString)));
    connect(ui.listStyles, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(selectStyle(QListWidgetItem*,QListWidgetItem*)));
    foreach(KoResource *res, KisResourceServerProvider::instance()->layerStyleCollectionServer()->resources()) {
        ui.cmbStyleCollections->addItem(res->name());
    }
}

class StyleItem : public QListWidgetItem {
public:
    StyleItem(KisPSDLayerStyle *style, const QString &name)
        : QListWidgetItem(name)
        , m_style(style)
    {
    }
    KisPSDLayerStyle *m_style;
};

void StylesSelector::loadStyles(const QString &name)
{
    ui.listStyles->clear();
    KoResource *res = KisResourceServerProvider::instance()->layerStyleCollectionServer()->resourceByName(name);
    KisPSDLayerStyleCollectionResource *collection = dynamic_cast<KisPSDLayerStyleCollectionResource*>(res);
    if (collection) {
        foreach(KisPSDLayerStyle *style, collection->layerStyles()) {
            // XXX: also use the preview image, when we have one
            ui.listStyles->addItem(new StyleItem(style, style->name()));
        }
    }
}

void StylesSelector::selectStyle(QListWidgetItem */*previous*/, QListWidgetItem* current)
{
    StyleItem *item = dynamic_cast<StyleItem*>(current);
    if (item) {
        emit styleSelected(item->m_style);
    }
}

Texture::Texture(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


OuterGlow::OuterGlow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}
