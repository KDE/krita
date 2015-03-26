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

#include <KoColorPopupButton.h>

#include "kis_config.h"
#include "kis_cmb_contour.h"
#include "kis_cmb_gradient.h"
#include "kis_resource_server_provider.h"
#include "kis_psd_layer_style_resource.h"
#include "kis_psd_layer_style.h"

#include "kis_signals_blocker.h"
#include "kis_signal_compressor.h"



KisDlgLayerStyle::KisDlgLayerStyle(KisPSDLayerStyleSP layerStyle, QWidget *parent)
    : KDialog(parent)
    , m_layerStyle(layerStyle)
    , m_initialLayerStyle(layerStyle->clone())
{
    setCaption(i18n("Layer Styles"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_configChangedCompressor =
        new KisSignalCompressor(1000, KisSignalCompressor::POSTPONE, this);
    connect(m_configChangedCompressor, SIGNAL(timeout()), SIGNAL(configChanged()));

    QWidget *page = new QWidget(this);
    wdgLayerStyles.setupUi(page);
    setMainWidget(page);

    connect(wdgLayerStyles.lstStyleSelector, SIGNAL(itemChanged(QListWidgetItem*)), m_configChangedCompressor, SLOT(start()));

    m_stylesSelector = new StylesSelector(this);
    connect(m_stylesSelector, SIGNAL(styleSelected(KisPSDLayerStyleSP)), SLOT(setStyle(KisPSDLayerStyleSP)));
    wdgLayerStyles.stylesStack->addWidget(m_stylesSelector);

    m_blendingOptions = new BlendingOptions(this);
    wdgLayerStyles.stylesStack->addWidget(m_blendingOptions);

    m_dropShadow = new DropShadow(DropShadow::DropShadowMode, this);
    wdgLayerStyles.stylesStack->addWidget(m_dropShadow);
    connect(m_dropShadow, SIGNAL(configChanged()), m_configChangedCompressor, SLOT(start()));

    m_innerShadow = new DropShadow(DropShadow::InnerShadowMode, this);
    wdgLayerStyles.stylesStack->addWidget(m_innerShadow);
    connect(m_innerShadow, SIGNAL(configChanged()), m_configChangedCompressor, SLOT(start()));

    m_outerGlow = new OuterGlow(this);
    wdgLayerStyles.stylesStack->addWidget(m_outerGlow);
    connect(m_outerGlow, SIGNAL(configChanged()), m_configChangedCompressor, SLOT(start()));

    m_innerGlow = new InnerGlow(this);
    wdgLayerStyles.stylesStack->addWidget(m_innerGlow);
    connect(m_innerGlow, SIGNAL(configChanged()), m_configChangedCompressor, SLOT(start()));

    m_bevelAndEmboss = new BevelAndEmboss(this);
    wdgLayerStyles.stylesStack->addWidget(m_bevelAndEmboss);
    connect(m_bevelAndEmboss, SIGNAL(configChanged()), m_configChangedCompressor, SLOT(start()));

    m_contour = new Contour(this);
    wdgLayerStyles.stylesStack->addWidget(m_contour);
    connect(m_contour, SIGNAL(configChanged()), m_configChangedCompressor, SLOT(start()));

    m_texture = new Texture(this);
    wdgLayerStyles.stylesStack->addWidget(m_texture);
    connect(m_texture, SIGNAL(configChanged()), m_configChangedCompressor, SLOT(start()));

    m_satin = new Satin(this);
    wdgLayerStyles.stylesStack->addWidget(m_satin);
    connect(m_satin, SIGNAL(configChanged()), m_configChangedCompressor, SLOT(start()));

    m_colorOverlay = new ColorOverlay(this);
    wdgLayerStyles.stylesStack->addWidget(m_colorOverlay);
    connect(m_colorOverlay, SIGNAL(configChanged()), m_configChangedCompressor, SLOT(start()));

    m_gradientOverlay = new GradientOverlay(this);
    wdgLayerStyles.stylesStack->addWidget(m_gradientOverlay);
    connect(m_gradientOverlay, SIGNAL(configChanged()), m_configChangedCompressor, SLOT(start()));

    m_patternOverlay = new PatternOverlay(this);
    wdgLayerStyles.stylesStack->addWidget(m_patternOverlay);
    connect(m_patternOverlay, SIGNAL(configChanged()), m_configChangedCompressor, SLOT(start()));

    m_stroke = new Stroke(this);
    wdgLayerStyles.stylesStack->addWidget(m_stroke);
    connect(m_stroke, SIGNAL(configChanged()), m_configChangedCompressor, SLOT(start()));

    KisConfig cfg;
    wdgLayerStyles.stylesStack->setCurrentIndex(cfg.readEntry("KisDlgLayerStyle::current", 1));
    wdgLayerStyles.lstStyleSelector->setCurrentRow(cfg.readEntry("KisDlgLayerStyle::current", 1));

    connect(wdgLayerStyles.lstStyleSelector,
            SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
             this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));

    setStyle(layerStyle);

    connect(this, SIGNAL(accepted()), SLOT(slotNotifyOnAccept()));
    connect(this, SIGNAL(rejected()), SLOT(slotNotifyOnReject()));
}

KisDlgLayerStyle::~KisDlgLayerStyle()
{
}

void KisDlgLayerStyle::slotNotifyOnAccept()
{
    if (m_configChangedCompressor->isActive()) {
        m_configChangedCompressor->stop();
        emit configChanged();
    }
}

void KisDlgLayerStyle::slotNotifyOnReject()
{
    setStyle(m_initialLayerStyle);

    m_configChangedCompressor->stop();
    emit configChanged();
}

void KisDlgLayerStyle::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current) {
        current = previous;
    }
    wdgLayerStyles.stylesStack->setCurrentIndex(wdgLayerStyles.lstStyleSelector->row(current));
}

void KisDlgLayerStyle::setStyle(KisPSDLayerStyleSP style)
{
    QListWidgetItem *item;
    item = wdgLayerStyles.lstStyleSelector->item(2);
    item->setCheckState(style->dropShadow()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(3);
    item->setCheckState(style->innerShadow()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(4);
    item->setCheckState(style->outerGlow()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(5);
    item->setCheckState(style->innerGlow()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

//    item = wdgLayerStyles.lstStyleSelector->item(6);
//    item->setCheckState(style->bevelAndEmboss()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

//    item = wdgLayerStyles.lstStyleSelector->item(7);
//    item->setCheckState(style->contour()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

//    item = wdgLayerStyles.lstStyleSelector->item(8);
//    item->setCheckState(style->texture()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(9);
    item->setCheckState(style->satin()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(10);
    item->setCheckState(style->colorOverlay()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(11);
    item->setCheckState(style->gradientOverlay()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(12);
    item->setCheckState(style->patternOverlay()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(13);
    item->setCheckState(style->stroke()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    m_dropShadow->setShadow(style->dropShadow());
    m_innerShadow->setShadow(style->innerShadow());
    m_outerGlow->setOuterGlow(style->outerGlow());
    m_innerGlow->setInnerGlow(style->innerGlow());
//    m_bevelAndEmboss->setBevelAndEmboss(style->bevelAndEmboss());
//    m_contour->setContour(style->contour());
//    m_texture->setTexture(style->texture());
    m_satin->setSatin(style->satin());
    m_colorOverlay->setColorOverlay(style->colorOverlay());
    m_gradientOverlay->setGradientOverlay(style->gradientOverlay());
    m_patternOverlay->setPatternOverlay(style->patternOverlay());
    m_stroke->setStroke(style->stroke());

}

KisPSDLayerStyleSP KisDlgLayerStyle::style() const
{
    m_layerStyle->dropShadow()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(2)->checkState() == Qt::Checked);
    m_layerStyle->innerShadow()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(3)->checkState() == Qt::Checked);
    m_layerStyle->outerGlow()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(4)->checkState() == Qt::Checked);
    m_layerStyle->innerGlow()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(5)->checkState() == Qt::Checked);
//    m_layerStyle->bevelAndEmboss()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(6)->checkState() == Qt::Checked);
//    m_layerStyle->contour()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(7)->checkState() == Qt::Checked);
//    m_layerStyle->texture()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(8)->checkState() == Qt::Checked);
    m_layerStyle->satin()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(9)->checkState() == Qt::Checked);
    m_layerStyle->colorOverlay()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(10)->checkState() == Qt::Checked);
    m_layerStyle->gradientOverlay()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(11)->checkState() == Qt::Checked);
    m_layerStyle->patternOverlay()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(12)->checkState() == Qt::Checked);
    m_layerStyle->stroke()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(13)->checkState() == Qt::Checked);


    m_dropShadow->fetchShadow(m_layerStyle->dropShadow());
    m_innerShadow->fetchShadow(m_layerStyle->innerShadow());
    m_outerGlow->fetchOuterGlow(m_layerStyle->outerGlow());
    m_innerGlow->fetchInnerGlow(m_layerStyle->innerGlow());
//    m_bevelAndEmboss->fetchBevelAndEmboss(m_layerStyle->bevelAndEmboss());
//    m_contour->fetchContour(m_layerStyle->contour());
//    m_texture->fetchTexture(m_layerStyle->texture());
    m_satin->fetchSatin(m_layerStyle->satin());
    m_colorOverlay->fetchColorOverlay(m_layerStyle->colorOverlay());
    m_gradientOverlay->fetchGradientOverlay(m_layerStyle->gradientOverlay());
    m_patternOverlay->fetchPatternOverlay(m_layerStyle->patternOverlay());
    m_stroke->fetchStroke(m_layerStyle->stroke());

    return m_layerStyle;
}

/********************************************************************/
/***** Styles Selector **********************************************/
/********************************************************************/

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
    StyleItem(KisPSDLayerStyleSP style, const QString &name)
        : QListWidgetItem(name)
        , m_style(style)
    {
    }
    KisPSDLayerStyleSP m_style;
};

void StylesSelector::loadStyles(const QString &name)
{
    ui.listStyles->clear();
    KoResource *res = KisResourceServerProvider::instance()->layerStyleCollectionServer()->resourceByName(name);
    KisPSDLayerStyleCollectionResource *collection = dynamic_cast<KisPSDLayerStyleCollectionResource*>(res);
    if (collection) {
        foreach(KisPSDLayerStyleSP style, collection->layerStyles()) {
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

/********************************************************************/
/***** Bevel and Emboss *********************************************/
/********************************************************************/

BevelAndEmboss::BevelAndEmboss(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

void BevelAndEmboss::setBevelAndEmboss(const psd_layer_effects_bevel_emboss *bevelEmboss)
{

}

void BevelAndEmboss::fetchBevelAndEmboss(psd_layer_effects_bevel_emboss *bevelEmboss) const
{

}


BlendingOptions::BlendingOptions(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

}


/********************************************************************/
/***** Color Overlay    *********************************************/
/********************************************************************/


ColorOverlay::ColorOverlay(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.intOpacity->setRange(0, 100);
    ui.intOpacity->setSuffix(" %");

    connect(ui.cmbCompositeOp, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.intOpacity, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.bnColor, SIGNAL(changed(QColor)), SIGNAL(configChanged()));
}

void ColorOverlay::setColorOverlay(const psd_layer_effects_color_overlay *colorOverlay)
{
    ui.cmbCompositeOp->selectCompositeOp(KoID(colorOverlay->blendMode()));
    ui.intOpacity->setValue(colorOverlay->opacity());
    ui.bnColor->setColor(colorOverlay->color());
}

void ColorOverlay::fetchColorOverlay(psd_layer_effects_color_overlay *colorOverlay) const
{
    colorOverlay->setBlendMode(ui.cmbCompositeOp->selectedCompositeOp().id());
    colorOverlay->setOpacity(ui.intOpacity->value());
    colorOverlay->setColor(ui.bnColor->color());
}


Contour::Contour(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

/********************************************************************/
/***** Drop Shadow **************************************************/
/********************************************************************/

DropShadow::DropShadow(Mode mode, QWidget *parent)
    : QWidget(parent),
      m_mode(mode)
{
    ui.setupUi(this);

    ui.doubleOpacity->setRange(0, 100, 0);
    ui.doubleOpacity->setSuffix(" %");

    ui.intDistance->setRange(0, 30000);
    ui.intDistance->setSuffix(" px");

    ui.intSpread->setRange(0, 100);
    ui.intSpread->setSuffix(" %");

    ui.intSize->setRange(0, 250);
    ui.intSize->setSuffix(" px");

    ui.intNoise->setRange(0, 100);
    ui.intNoise->setSuffix(" %");

    connect(ui.dialAngle, SIGNAL(valueChanged(int)), SLOT(slotDialAngleChanged(int)));
    connect(ui.intAngle, SIGNAL(valueChanged(int)), SLOT(slotIntAngleChanged(int)));
    connect(ui.chkUseGlobalLight, SIGNAL(toggled(bool)), ui.dialAngle, SLOT(setDisabled(bool)));
    connect(ui.chkUseGlobalLight, SIGNAL(toggled(bool)), ui.intAngle, SLOT(setDisabled(bool)));

    // connect everything to configChanged() signal
    connect(ui.cmbCompositeOp, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.doubleOpacity, SIGNAL(valueChanged(qreal)), SIGNAL(configChanged()));

    connect(ui.dialAngle, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intAngle, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.chkUseGlobalLight, SIGNAL(toggled(bool)), SIGNAL(configChanged()));

    connect(ui.intDistance, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intSpread, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intSize, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    connect(ui.cmbContour, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.chkAntiAliased, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.intNoise, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    connect(ui.chkLayerKnocksOutDropShadow, SIGNAL(toggled(bool)), SIGNAL(configChanged()));

    if (m_mode == InnerShadowMode) {
        ui.chkLayerKnocksOutDropShadow->setVisible(false);
        ui.grpMain->setTitle(i18n("Inner Shadow"));
        ui.lblSpread->setText(i18n("Choke"));
    }
}

void DropShadow::slotDialAngleChanged(int value)
{
    KisSignalsBlocker b(ui.intAngle);
    ui.intAngle->setValue(value);
}

void DropShadow::slotIntAngleChanged(int value)
{
    KisSignalsBlocker b(ui.dialAngle);
    ui.dialAngle->setValue(value);
}

void DropShadow::setShadow(const psd_layer_effects_shadow_common *shadow)
{
    ui.cmbCompositeOp->selectCompositeOp(KoID(shadow->blendMode()));
    ui.doubleOpacity->setValue(shadow->opacity());

    ui.dialAngle->setValue(shadow->angle());
    ui.intAngle->setValue(shadow->angle());
    ui.chkUseGlobalLight->setChecked(shadow->useGlobalLight());

    ui.intDistance->setValue(shadow->distance());
    ui.intSpread->setValue(shadow->spread());
    ui.intSize->setValue(shadow->size());

    // FIXME: curve editing
    // ui.cmbContour;
    ui.chkAntiAliased->setChecked(shadow->antiAliased());

    ui.intNoise->setValue(shadow->noise());

    if (m_mode == DropShadowMode) {
        const psd_layer_effects_drop_shadow *realDropShadow = dynamic_cast<const psd_layer_effects_drop_shadow*>(shadow);
        KIS_ASSERT_RECOVER_NOOP(realDropShadow);

        ui.chkLayerKnocksOutDropShadow->setChecked(shadow->knocksOut());
    }
}

void DropShadow::fetchShadow(psd_layer_effects_shadow_common *shadow) const
{
    shadow->setBlendMode(ui.cmbCompositeOp->selectedCompositeOp().id());
    shadow->setOpacity(ui.doubleOpacity->value());

    shadow->setAngle(ui.dialAngle->value());
    shadow->setUseGlobalLight(ui.chkUseGlobalLight->isChecked());

    shadow->setDistance(ui.intDistance->value());
    shadow->setSpread(ui.intSpread->value());
    shadow->setSize(ui.intSize->value());

    // FIXME: curve editing
    // ui.cmbContour;
    shadow->setAntiAliased(ui.chkAntiAliased->isChecked());
    shadow->setNoise(ui.intNoise->value());

    if (m_mode == DropShadowMode) {
        psd_layer_effects_drop_shadow *realDropShadow = dynamic_cast<psd_layer_effects_drop_shadow*>(shadow);
        KIS_ASSERT_RECOVER_NOOP(realDropShadow);

        realDropShadow->setKnocksOut(ui.chkLayerKnocksOutDropShadow->isChecked());
    }
}

/********************************************************************/
/***** Gradient Overlay *********************************************/
/********************************************************************/

GradientOverlay::GradientOverlay(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.intOpacity->setRange(0, 100);
    ui.intOpacity->setSuffix(" %");

    ui.intScale->setRange(0, 100);
    ui.intScale->setSuffix(" %");

    connect(ui.dialAngle, SIGNAL(valueChanged(int)), SLOT(slotDialAngleChanged(int)));
    connect(ui.intAngle, SIGNAL(valueChanged(int)), SLOT(slotIntAngleChanged(int)));

    connect(ui.cmbCompositeOp, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.intOpacity, SIGNAL(valueChanged(qreal)), SIGNAL(configChanged()));
    connect(ui.cmbGradient, SIGNAL(gradientChanged(KoAbstractGradient*)), SIGNAL(configChanged()));
    connect(ui.chkReverse, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.cmbStyle, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.chkAlignWithLayer, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.dialAngle, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intAngle, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intScale, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
}

void GradientOverlay::setGradientOverlay(const psd_layer_effects_gradient_overlay *gradient)
{
    ui.cmbCompositeOp->selectCompositeOp(KoID(gradient->blendMode()));
    ui.intOpacity->setValue(gradient->opacity());
    ui.cmbGradient->setGradient(gradient->gradient());
    ui.chkReverse->setChecked(gradient->antiAliased());
    ui.cmbStyle->setCurrentIndex((int)gradient->style());
    ui.chkAlignWithLayer->setCheckable(gradient->alignWithLayer());
    ui.dialAngle->setValue(gradient->angle());
    ui.intAngle->setValue(gradient->angle());
    ui.intScale->setValue(gradient->scale());
}

void GradientOverlay::fetchGradientOverlay(psd_layer_effects_gradient_overlay *gradient) const
{
    gradient->setBlendMode(ui.cmbCompositeOp->selectedCompositeOp().id());
    gradient->setOpacity(ui.intOpacity->value());
    gradient->setGradient(ui.cmbGradient->gradient());
    gradient->setReverse(ui.chkReverse->isChecked());
    gradient->setStyle((psd_gradient_style)ui.cmbStyle->currentIndex());
    gradient->setAlignWithLayer(ui.chkAlignWithLayer->isChecked());
    gradient->setAngle(ui.dialAngle->value());
    gradient->setScale(ui.intScale->value());
}

void GradientOverlay::slotDialAngleChanged(int value)
{
    KisSignalsBlocker b(ui.intAngle);
    ui.intAngle->setValue(value);
}

void GradientOverlay::slotIntAngleChanged(int value)
{
    KisSignalsBlocker b(ui.dialAngle);
    ui.dialAngle->setValue(value);
}

/********************************************************************/
/***** Innner Glow      *********************************************/
/********************************************************************/

InnerGlow::InnerGlow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.intOpacity->setRange(0, 100);
    ui.intOpacity->setSuffix(" %");

    ui.intNoise->setRange(0, 100);
    ui.intNoise->setSuffix(" %");

    ui.intChoke->setRange(0, 100);
    ui.intChoke->setSuffix(" %");

    ui.intSize->setRange(0, 250);
    ui.intSize->setSuffix(" px");

    ui.intRange->setRange(0, 100);
    ui.intRange->setSuffix(" %");

    ui.intJitter->setRange(0, 100);
    ui.intJitter->setSuffix(" %");

    connect(ui.cmbCompositeOp, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.intOpacity, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intNoise, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    connect(ui.radioColor, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.bnColor, SIGNAL(changed(QColor)), SIGNAL(configChanged()));
    connect(ui.radioGradient, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.cmbGradient, SIGNAL(gradientChanged(KoAbstractGradient*)), SIGNAL(configChanged()));

    connect(ui.cmbTechnique, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.intChoke, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intSize, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    connect(ui.cmbContour, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.chkAntiAliased, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.intRange, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intJitter, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

}

void InnerGlow::setInnerGlow(const psd_layer_effects_inner_glow *innerGlow)
{
    ui.cmbCompositeOp->selectCompositeOp(KoID(innerGlow->blendMode()));
    ui.intOpacity->setValue(innerGlow->opacity());
    ui.intNoise->setValue(innerGlow->noise());

    ui.radioColor->setChecked(innerGlow->fillType() == psd_fill_solid_color);
    ui.bnColor->setColor(innerGlow->color());
    ui.radioGradient->setChecked(innerGlow->fillType() == psd_fill_gradient);
    ui.cmbGradient->setGradient(innerGlow->gradient());

    ui.cmbTechnique->setCurrentIndex((int)innerGlow->technique());
    ui.intChoke->setValue(innerGlow->spread());
    ui.intSize->setValue(innerGlow->size());

    // FIXME: Curve editing
    //ui.cmbContour;

    ui.chkAntiAliased->setChecked(innerGlow->antiAliased());
    ui.intRange->setValue(innerGlow->range());
    ui.intJitter->setValue(innerGlow->jitter());
}

void InnerGlow::fetchInnerGlow(psd_layer_effects_inner_glow *innerGlow) const
{
    innerGlow->setBlendMode(ui.cmbCompositeOp->selectedCompositeOp().id());
    innerGlow->setOpacity(ui.intOpacity->value());
    innerGlow->setNoise(ui.intNoise->value());

    if (ui.radioColor->isChecked()) {
        innerGlow->setFillType(psd_fill_solid_color);
    }
    else {
        innerGlow->setFillType(psd_fill_gradient);
    }

    innerGlow->setColor(ui.bnColor->color());
    innerGlow->setGradient(ui.cmbGradient->gradient());

    innerGlow->setTechnique((psd_technique_type)ui.cmbTechnique->currentIndex());
    innerGlow->setSpread(ui.intChoke->value());
    innerGlow->setSize(ui.intSize->value());

    // FIXME: Curve editing
    //ui.cmbContour;

    innerGlow->setAntiAliased(ui.chkAntiAliased->isChecked());
    innerGlow->setRange(ui.intRange->value());
    innerGlow->setJitter(ui.intJitter->value());
}

/********************************************************************/
/***** Pattern Overlay  *********************************************/
/********************************************************************/


PatternOverlay::PatternOverlay(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.intOpacity->setRange(0, 100);
    ui.intOpacity->setSuffix(" %");

    ui.intScale->setRange(0, 100);
    ui.intScale->setSuffix(" %");

    connect(ui.cmbCompositeOp, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.intOpacity, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.patternChooser, SIGNAL(resourceSelected(KoResource*)), SIGNAL(configChanged()));
    connect(ui.chkLinkWithLayer, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.intScale, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
}

void PatternOverlay::setPatternOverlay(const psd_layer_effects_pattern_overlay *pattern)
{
    ui.cmbCompositeOp->selectCompositeOp(KoID(pattern->blendMode()));
    ui.intOpacity->setValue(pattern->opacity());
    ui.patternChooser->setCurrentPattern(pattern->pattern());
    ui.chkLinkWithLayer->setChecked(pattern->alignWithLayer());
    ui.intScale->setValue(pattern->noise());
}

void PatternOverlay::fetchPatternOverlay(psd_layer_effects_pattern_overlay *pattern) const
{
    pattern->setBlendMode(ui.cmbCompositeOp->selectedCompositeOp().id());
    pattern->setOpacity(ui.intOpacity->value());
    pattern->setPattern((KoPattern*)ui.patternChooser->currentResource());
    pattern->setAlignWithLayer(ui.chkLinkWithLayer->isChecked());
    pattern->setScale(ui.intScale->value());
}

/********************************************************************/
/***** Satin            *********************************************/
/********************************************************************/


Satin::Satin(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.doubleOpacity->setRange(0, 100, 0);
    ui.doubleOpacity->setSuffix(" %");

    ui.intDistance->setRange(0, 250);
    ui.intDistance->setSuffix(" px");

    ui.intSize->setRange(0, 250);
    ui.intSize->setSuffix(" px");

    connect(ui.dialAngle, SIGNAL(valueChanged(int)), SLOT(slotDialAngleChanged(int)));
    connect(ui.intAngle, SIGNAL(valueChanged(int)), SLOT(slotIntAngleChanged(int)));

    connect(ui.cmbCompositeOp, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.doubleOpacity, SIGNAL(valueChanged(qreal)), SIGNAL(configChanged()));

    connect(ui.dialAngle, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intAngle, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intDistance, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intSize, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    connect(ui.cmbContour, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.chkAntiAliased, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.chkInvert, SIGNAL(toggled(bool)), SIGNAL(configChanged()));

}

void Satin::slotDialAngleChanged(int value)
{
    KisSignalsBlocker b(ui.intAngle);
    ui.intAngle->setValue(value);
}

void Satin::slotIntAngleChanged(int value)
{
    KisSignalsBlocker b(ui.dialAngle);
    ui.dialAngle->setValue(value);
}

void Satin::setSatin(const psd_layer_effects_satin *satin)
{
    ui.cmbCompositeOp->selectCompositeOp(KoID(satin->blendMode()));
    ui.bnColor->setColor(satin->color());
    ui.doubleOpacity->setValue(satin->opacity());

    ui.dialAngle->setValue(satin->angle());
    ui.intAngle->setValue(satin->angle());

    ui.intDistance->setValue(satin->distance());
    ui.intSize->setValue(satin->size());

    // FIXME: Curve editing
    //ui.cmbContour;

    ui.chkAntiAliased->setChecked(satin->antiAliased());
    ui.chkInvert->setChecked(satin->invert());

}

void Satin::fetchSatin(psd_layer_effects_satin *satin) const
{
    satin->setBlendMode(ui.cmbCompositeOp->selectedCompositeOp().id());
    satin->setOpacity(ui.doubleOpacity->value());
    satin->setColor(ui.bnColor->color());

    satin->setAngle(ui.dialAngle->value());

    satin->setDistance(ui.intDistance->value());
    satin->setSize(ui.intSize->value());

    // FIXME: curve editing
    // ui.cmbContour;
    satin->setAntiAliased(ui.chkAntiAliased->isChecked());
    satin->setInvert(ui.chkInvert->isChecked());
}

/********************************************************************/
/***** Stroke           *********************************************/
/********************************************************************/

Stroke::Stroke(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

void Stroke::setStroke(const psd_layer_effects_stroke *stroke)
{

}

void Stroke::fetchStroke(psd_layer_effects_stroke *stroke) const
{

}

/********************************************************************/
/***** Texture          *********************************************/
/********************************************************************/


Texture::Texture(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

/********************************************************************/
/***** Outer Glow       *********************************************/
/********************************************************************/

OuterGlow::OuterGlow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.intOpacity->setRange(0, 100);
    ui.intOpacity->setSuffix(" %");

    ui.intNoise->setRange(0, 100);
    ui.intNoise->setSuffix(" %");

    ui.intSpread->setRange(0, 100);
    ui.intSpread->setSuffix(" %");

    ui.intSize->setRange(0, 250);
    ui.intSize->setSuffix(" px");

    ui.intRange->setRange(0, 100);
    ui.intRange->setSuffix(" %");

    ui.intJitter->setRange(0, 100);
    ui.intJitter->setSuffix(" %");

    connect(ui.cmbCompositeOp, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.intOpacity, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intNoise, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    connect(ui.radioColor, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.bnColor, SIGNAL(changed(QColor)), SIGNAL(configChanged()));
    connect(ui.radioGradient, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.cmbGradient, SIGNAL(gradientChanged(KoAbstractGradient*)), SIGNAL(configChanged()));

    connect(ui.cmbTechnique, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.intSpread, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intSize, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    connect(ui.cmbContour, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.chkAntiAliased, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.intRange, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intJitter, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

}

void OuterGlow::setOuterGlow(const psd_layer_effects_outer_glow *outerGlow)
{
    ui.cmbCompositeOp->selectCompositeOp(KoID(outerGlow->blendMode()));
    ui.intOpacity->setValue(outerGlow->opacity());
    ui.intNoise->setValue(outerGlow->noise());

    ui.radioColor->setChecked(outerGlow->fillType() == psd_fill_solid_color);
    ui.bnColor->setColor(outerGlow->color());
    ui.radioGradient->setChecked(outerGlow->fillType() == psd_fill_gradient);
    ui.cmbGradient->setGradient(outerGlow->gradient());

    ui.cmbTechnique->setCurrentIndex((int)outerGlow->technique());
    ui.intSpread->setValue(outerGlow->spread());
    ui.intSize->setValue(outerGlow->size());

    // FIXME: Curve editing
    //ui.cmbContour;

    ui.chkAntiAliased->setChecked(outerGlow->antiAliased());
    ui.intRange->setValue(outerGlow->range());
    ui.intJitter->setValue(outerGlow->jitter());
}

void OuterGlow::fetchOuterGlow(psd_layer_effects_outer_glow *outerGlow) const
{
    outerGlow->setBlendMode(ui.cmbCompositeOp->selectedCompositeOp().id());
    outerGlow->setOpacity(ui.intOpacity->value());
    outerGlow->setNoise(ui.intNoise->value());

    if (ui.radioColor->isChecked()) {
        outerGlow->setFillType(psd_fill_solid_color);
    }
    else {
        outerGlow->setFillType(psd_fill_gradient);
    }

    outerGlow->setColor(ui.bnColor->color());
    outerGlow->setGradient(ui.cmbGradient->gradient());

    outerGlow->setTechnique((psd_technique_type)ui.cmbTechnique->currentIndex());
    outerGlow->setSpread(ui.intSpread->value());
    outerGlow->setSize(ui.intSize->value());

    // FIXME: Curve editing
    //ui.cmbContour;

    outerGlow->setAntiAliased(ui.chkAntiAliased->isChecked());
    outerGlow->setRange(ui.intRange->value());
    outerGlow->setJitter(ui.intJitter->value());
}
