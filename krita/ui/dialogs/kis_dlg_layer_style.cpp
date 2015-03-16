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

    m_dropShadow = new DropShadow(this);
    wdgLayerStyles.stylesStack->addWidget(m_dropShadow);
    connect(m_dropShadow, SIGNAL(configChanged()), m_configChangedCompressor, SLOT(start()));

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
    QListWidgetItem *item = wdgLayerStyles.lstStyleSelector->item(2);
    item->setCheckState(style->drop_shadow()->effectEnabled() ? Qt::Checked : Qt::Unchecked);
    m_dropShadow->setDropShadow(*style->drop_shadow());
}

KisPSDLayerStyleSP KisDlgLayerStyle::style() const
{
    psd_layer_effects_drop_shadow ds = m_dropShadow->dropShadow();
    ds.setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(2)->checkState() == Qt::Checked);
    *m_layerStyle->drop_shadow() = ds;

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

/********************************************************************/
/***** Drop Shadow **************************************************/
/********************************************************************/

DropShadow::DropShadow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.doubleOpacity->setRange(0, 100, 0);
    ui.doubleOpacity->setSuffix(" %");

    ui.intDistance->setRange(0, 300);
    ui.intDistance->setSuffix(" px");

    ui.intSpread->setRange(0, 100);
    ui.intSpread->setSuffix(" %");

    ui.intSize->setRange(0, 100);
    ui.intSize->setSuffix(" px");

    ui.intNoise->setRange(0, 100);
    ui.intNoise->setSuffix(" px");

    connect(ui.dialAngle, SIGNAL(valueChanged(int)), SLOT(slotDialAngleChanged(int)));
    connect(ui.intAngle, SIGNAL(valueChanged(int)), SLOT(slotIntAngleChanged(int)));
    connect(ui.chkUseGlobalLight, SIGNAL(toggled(bool)), ui.dialAngle, SLOT(setDisabled(bool)));
    connect(ui.chkUseGlobalLight, SIGNAL(toggled(bool)), ui.intAngle, SLOT(setDisabled(bool)));

    // FIXME: predefined curves
    ui.cmbContour->addItem("NOT IMPLEMENTED");


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

void DropShadow::setDropShadow(const psd_layer_effects_drop_shadow &dropShadow)
{
    ui.cmbCompositeOp->selectCompositeOp(KoID(dropShadow.blendMode()));
    ui.doubleOpacity->setValue(dropShadow.opacity());

    ui.dialAngle->setValue(dropShadow.angle());
    ui.intAngle->setValue(dropShadow.angle());
    ui.chkUseGlobalLight->setChecked(dropShadow.useGlobalLight());

    ui.intDistance->setValue(dropShadow.distance());
    ui.intSpread->setValue(dropShadow.spread());
    ui.intSize->setValue(dropShadow.size());

    // FIXME: curve editing
    // ui.cmbContour;
    ui.chkAntiAliased->setChecked(dropShadow.antiAliased());

    ui.intNoise->setValue(dropShadow.noise());
    ui.chkLayerKnocksOutDropShadow->setChecked(dropShadow.knocksOut());
}

psd_layer_effects_drop_shadow DropShadow::dropShadow() const
{
    psd_layer_effects_drop_shadow ds;


    ds.setBlendMode(ui.cmbCompositeOp->selectedCompositeOp().id());
    ds.setOpacity(ui.doubleOpacity->value());

    ds.setAngle(ui.dialAngle->value());
    ds.setUseGlobalLight(ui.chkUseGlobalLight->isChecked());

    ds.setDistance(ui.intDistance->value());
    ds.setSpread(ui.intSpread->value());
    ds.setSize(ui.intSize->value());

    // FIXME: curve editing
    // ui.cmbContour;
    ds.setAntiAliased(ui.chkAntiAliased->isChecked());
    ds.setNoise(ui.intNoise->value());

    ds.setKnocksOut(ui.chkLayerKnocksOutDropShadow->isChecked());

    return ds;
}

/********************************************************************/
/***** Gradient Overlay *********************************************/
/********************************************************************/

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
