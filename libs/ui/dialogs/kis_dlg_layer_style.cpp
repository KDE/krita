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
#include <QUuid>
#include <QInputDialog>

#include <KoColorPopupButton.h>
#include <KoColorSpaceRegistry.h>
#include <KoResourceServerProvider.h>

#include "kis_config.h"
#include "kis_cmb_contour.h"
#include "kis_cmb_gradient.h"
#include "KisResourceServerProvider.h"
#include "kis_psd_layer_style_resource.h"
#include "kis_psd_layer_style.h"

#include "kis_signals_blocker.h"
#include "kis_signal_compressor.h"
#include "kis_canvas_resource_provider.h"

#include <KoFileDialog.h>


KoAbstractGradient* fetchGradientLazy(KoAbstractGradient *gradient,
                                      KisCanvasResourceProvider *resourceProvider)
{
    if (!gradient) {
        gradient = resourceProvider->currentGradient();
    }
    return gradient;
}

KisDlgLayerStyle::KisDlgLayerStyle(KisPSDLayerStyleSP layerStyle, KisCanvasResourceProvider *resourceProvider, QWidget *parent)
    : KoDialog(parent)
    , m_layerStyle(layerStyle)
    , m_initialLayerStyle(layerStyle->clone())
    , m_isSwitchingPredefinedStyle(false)
    , m_sanityLayerStyleDirty(false)
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
    wdgLayerStyles.chkPreview->setVisible(false);

    connect(wdgLayerStyles.lstStyleSelector, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(notifyGuiConfigChanged()));

    m_stylesSelector = new StylesSelector(this);
    connect(m_stylesSelector, SIGNAL(styleSelected(KisPSDLayerStyleSP)), SLOT(notifyPredefinedStyleSelected(KisPSDLayerStyleSP)));
    wdgLayerStyles.stylesStack->addWidget(m_stylesSelector);

    m_blendingOptions = new BlendingOptions(this);
    wdgLayerStyles.stylesStack->addWidget(m_blendingOptions);

    m_dropShadow = new DropShadow(DropShadow::DropShadowMode, this);
    wdgLayerStyles.stylesStack->addWidget(m_dropShadow);
    connect(m_dropShadow, SIGNAL(configChanged()), SLOT(notifyGuiConfigChanged()));

    m_innerShadow = new DropShadow(DropShadow::InnerShadowMode, this);
    wdgLayerStyles.stylesStack->addWidget(m_innerShadow);
    connect(m_innerShadow, SIGNAL(configChanged()), SLOT(notifyGuiConfigChanged()));

    m_outerGlow = new InnerGlow(InnerGlow::OuterGlowMode, resourceProvider, this);
    wdgLayerStyles.stylesStack->addWidget(m_outerGlow);
    connect(m_outerGlow, SIGNAL(configChanged()), SLOT(notifyGuiConfigChanged()));

    m_innerGlow = new InnerGlow(InnerGlow::InnerGlowMode, resourceProvider, this);
    wdgLayerStyles.stylesStack->addWidget(m_innerGlow);
    connect(m_innerGlow, SIGNAL(configChanged()), SLOT(notifyGuiConfigChanged()));

    // Contour and Texture are sub-styles of Bevel and Emboss
    // They are only applied to canvas when Bevel and Emboss is active.
    m_contour = new Contour(this);
    m_texture = new Texture(this);
    m_bevelAndEmboss = new BevelAndEmboss(m_contour, m_texture, this);

    wdgLayerStyles.stylesStack->addWidget(m_bevelAndEmboss);
    wdgLayerStyles.stylesStack->addWidget(m_contour);
    wdgLayerStyles.stylesStack->addWidget(m_texture);

    // slotBevelAndEmbossChanged(QListWidgetItem*) enables/disables Contour and Texture on "Bevel and Emboss" toggle.
    connect(wdgLayerStyles.lstStyleSelector, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(slotBevelAndEmbossChanged(QListWidgetItem*)));
    connect(m_bevelAndEmboss, SIGNAL(configChanged()), SLOT(notifyGuiConfigChanged()));

    m_satin = new Satin(this);
    wdgLayerStyles.stylesStack->addWidget(m_satin);
    connect(m_satin, SIGNAL(configChanged()), SLOT(notifyGuiConfigChanged()));

    m_colorOverlay = new ColorOverlay(this);
    wdgLayerStyles.stylesStack->addWidget(m_colorOverlay);
    connect(m_colorOverlay, SIGNAL(configChanged()), SLOT(notifyGuiConfigChanged()));

    m_gradientOverlay = new GradientOverlay(resourceProvider, this);
    wdgLayerStyles.stylesStack->addWidget(m_gradientOverlay);
    connect(m_gradientOverlay, SIGNAL(configChanged()), SLOT(notifyGuiConfigChanged()));

    m_patternOverlay = new PatternOverlay(this);
    wdgLayerStyles.stylesStack->addWidget(m_patternOverlay);
    connect(m_patternOverlay, SIGNAL(configChanged()), SLOT(notifyGuiConfigChanged()));

    m_stroke = new Stroke(resourceProvider, this);
    wdgLayerStyles.stylesStack->addWidget(m_stroke);
    connect(m_stroke, SIGNAL(configChanged()), SLOT(notifyGuiConfigChanged()));

    KisConfig cfg(true);
    wdgLayerStyles.stylesStack->setCurrentIndex(cfg.readEntry("KisDlgLayerStyle::current", 1));
    wdgLayerStyles.lstStyleSelector->setCurrentRow(cfg.readEntry("KisDlgLayerStyle::current", 1));

    connect(wdgLayerStyles.lstStyleSelector,
            SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
             this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));

    // improve the checkbox visibility by altering the style sheet list a bit
    // the dark themes make them hard to see
    QPalette newPalette = palette();
    newPalette.setColor(QPalette::Active, QPalette::Background, palette().text().color() );
    wdgLayerStyles.lstStyleSelector->setPalette(newPalette);


    notifyPredefinedStyleSelected(layerStyle);

    connect(m_dropShadow, SIGNAL(globalAngleChanged(int)), SLOT(syncGlobalAngle(int)));
    connect(m_innerShadow, SIGNAL(globalAngleChanged(int)), SLOT(syncGlobalAngle(int)));
    connect(m_bevelAndEmboss, SIGNAL(globalAngleChanged(int)), SLOT(syncGlobalAngle(int)));


    connect(wdgLayerStyles.btnNewStyle, SIGNAL(clicked()), SLOT(slotNewStyle()));
    connect(wdgLayerStyles.btnLoadStyle, SIGNAL(clicked()), SLOT(slotLoadStyle()));
    connect(wdgLayerStyles.btnSaveStyle, SIGNAL(clicked()), SLOT(slotSaveStyle()));

    connect(wdgLayerStyles.chkMasterFxSwitch, SIGNAL(toggled(bool)), SLOT(slotMasterFxSwitchChanged(bool)));

    connect(this, SIGNAL(accepted()), SLOT(slotNotifyOnAccept()));
    connect(this, SIGNAL(rejected()), SLOT(slotNotifyOnReject()));
}

KisDlgLayerStyle::~KisDlgLayerStyle()
{
}

void KisDlgLayerStyle::slotMasterFxSwitchChanged(bool value)
{
    wdgLayerStyles.lstStyleSelector->setEnabled(value);
    wdgLayerStyles.stylesStack->setEnabled(value);
    wdgLayerStyles.btnNewStyle->setEnabled(value);
    wdgLayerStyles.btnLoadStyle->setEnabled(value);
    wdgLayerStyles.btnSaveStyle->setEnabled(value);
    notifyGuiConfigChanged();
}

void KisDlgLayerStyle::notifyGuiConfigChanged()
{
    if (m_isSwitchingPredefinedStyle) return;

    m_configChangedCompressor->start();
    m_layerStyle->setUuid(QUuid::createUuid());
    m_sanityLayerStyleDirty = true;

    m_stylesSelector->notifyExternalStyleChanged(m_layerStyle->name(), m_layerStyle->uuid());
}

void KisDlgLayerStyle::notifyPredefinedStyleSelected(KisPSDLayerStyleSP style)
{
    m_isSwitchingPredefinedStyle = true;
    setStyle(style);
    m_isSwitchingPredefinedStyle = false;
    m_configChangedCompressor->start();
}

void KisDlgLayerStyle::slotBevelAndEmbossChanged(QListWidgetItem*) {
    QListWidgetItem *item;

    if (wdgLayerStyles.lstStyleSelector->item(6)->checkState() == Qt::Checked) {
        // Enable "Contour" (list item 7)
        item = wdgLayerStyles.lstStyleSelector->item(7);
        Qt::ItemFlags currentFlags7 = item->flags();
        item->setFlags(currentFlags7 | Qt::ItemIsEnabled);

        // Enable "Texture" (list item 8)
        item = wdgLayerStyles.lstStyleSelector->item(8);
        Qt::ItemFlags currentFlags8 = item->flags();
        item->setFlags(currentFlags8 | Qt::ItemIsEnabled);
    }
    else {
        // Disable "Contour"
        item = wdgLayerStyles.lstStyleSelector->item(7);
        Qt::ItemFlags currentFlags7 = item->flags();
        item->setFlags(currentFlags7 & (~Qt::ItemIsEnabled));

        // Disable "Texture"
        item = wdgLayerStyles.lstStyleSelector->item(8);
        Qt::ItemFlags currentFlags8 = item->flags();
        item->setFlags(currentFlags8 & (~Qt::ItemIsEnabled));
    }
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
    notifyPredefinedStyleSelected(m_initialLayerStyle);

    m_configChangedCompressor->stop();
    emit configChanged();
}

bool checkCustomNameAvailable(const QString &name)
{
    const QString customName = "CustomStyles.asl";

    KoResourceServer<KisPSDLayerStyleCollectionResource> *server = KisResourceServerProvider::instance()->layerStyleCollectionServer();

    KoResource *resource = server->resourceByName(customName);
    if (!resource) return true;

    KisPSDLayerStyleCollectionResource *collection = dynamic_cast<KisPSDLayerStyleCollectionResource*>(resource);

    Q_FOREACH (KisPSDLayerStyleSP style, collection->layerStyles()) {
        if (style->name() == name) {
            return false;
        }
    }

    return true;
}

QString selectAvailableStyleName(const QString &name)
{
    QString finalName = name;
    if (checkCustomNameAvailable(finalName)) {
        return finalName;
    }

    int i = 0;

    do {
        finalName = QString("%1%2").arg(name).arg(i++);
    } while (!checkCustomNameAvailable(finalName));

    return finalName;
}

void KisDlgLayerStyle::slotNewStyle()
{
    QString styleName =
        QInputDialog::getText(this,
                              i18nc("@title:window", "Enter new style name"),
                              i18nc("@label:textbox", "Name:"),
                              QLineEdit::Normal, i18nc("Default name for a new style", "New Style"));

    KisPSDLayerStyleSP style = this->style();
    style->setName(selectAvailableStyleName(styleName));

    m_stylesSelector->addNewStyle(style->clone());
}

void KisDlgLayerStyle::slotLoadStyle()
{
    QString filename; // default value?

    KoFileDialog dialog(this, KoFileDialog::OpenFile, "layerstyle");
    dialog.setCaption(i18n("Select ASL file"));
    dialog.setMimeTypeFilters(QStringList() << "application/x-photoshop-style-library", "application/x-photoshop-style-library");
    filename = dialog.filename();

    m_stylesSelector->loadCollection(filename);
    wdgLayerStyles.lstStyleSelector->setCurrentRow(0);
}

void KisDlgLayerStyle::slotSaveStyle()
{
    QString filename; // default value?

    KoFileDialog dialog(this, KoFileDialog::SaveFile, "layerstyle");
    dialog.setCaption(i18n("Select ASL file"));
    dialog.setMimeTypeFilters(QStringList() << "application/x-photoshop-style-library", "application/x-photoshop-style-library");
    filename = dialog.filename();

    QScopedPointer<KisPSDLayerStyleCollectionResource> collection(
        new KisPSDLayerStyleCollectionResource(filename));

    KisPSDLayerStyleSP newStyle = style()->clone();
    newStyle->setName(QFileInfo(filename).completeBaseName());

    KisPSDLayerStyleCollectionResource::StylesVector vector = collection->layerStyles();
    vector << newStyle;
    collection->setLayerStyles(vector);
    collection->save();
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
    // we may self-assign style is some cases
    if (style != m_layerStyle) {
        *m_layerStyle = *style;
    }
    m_sanityLayerStyleDirty = false;

    {
        KisSignalsBlocker b(m_stylesSelector);
        m_stylesSelector->notifyExternalStyleChanged(m_layerStyle->name(), m_layerStyle->uuid());
    }

    QListWidgetItem *item;
    item = wdgLayerStyles.lstStyleSelector->item(2);
    item->setCheckState(m_layerStyle->dropShadow()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(3);
    item->setCheckState(m_layerStyle->innerShadow()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(4);
    item->setCheckState(m_layerStyle->outerGlow()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(5);
    item->setCheckState(m_layerStyle->innerGlow()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(6);
    item->setCheckState(m_layerStyle->bevelAndEmboss()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(7);
    item->setCheckState(m_layerStyle->bevelAndEmboss()->contourEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(8);
    item->setCheckState(m_layerStyle->bevelAndEmboss()->textureEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(9);
    item->setCheckState(m_layerStyle->satin()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(10);
    item->setCheckState(m_layerStyle->colorOverlay()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(11);
    item->setCheckState(m_layerStyle->gradientOverlay()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(12);
    item->setCheckState(m_layerStyle->patternOverlay()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    item = wdgLayerStyles.lstStyleSelector->item(13);
    item->setCheckState(m_layerStyle->stroke()->effectEnabled() ? Qt::Checked : Qt::Unchecked);

    m_dropShadow->setShadow(m_layerStyle->dropShadow());
    m_innerShadow->setShadow(m_layerStyle->innerShadow());
    m_outerGlow->setConfig(m_layerStyle->outerGlow());
    m_innerGlow->setConfig(m_layerStyle->innerGlow());
    m_bevelAndEmboss->setBevelAndEmboss(m_layerStyle->bevelAndEmboss());
    m_satin->setSatin(m_layerStyle->satin());
    m_colorOverlay->setColorOverlay(m_layerStyle->colorOverlay());
    m_gradientOverlay->setGradientOverlay(m_layerStyle->gradientOverlay());
    m_patternOverlay->setPatternOverlay(m_layerStyle->patternOverlay());
    m_stroke->setStroke(m_layerStyle->stroke());

    wdgLayerStyles.chkMasterFxSwitch->setChecked(m_layerStyle->isEnabled());
    slotMasterFxSwitchChanged(m_layerStyle->isEnabled());
}

KisPSDLayerStyleSP KisDlgLayerStyle::style() const
{
    m_layerStyle->setEnabled(wdgLayerStyles.chkMasterFxSwitch->isChecked());

    m_layerStyle->dropShadow()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(2)->checkState() == Qt::Checked);
    m_layerStyle->innerShadow()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(3)->checkState() == Qt::Checked);
    m_layerStyle->outerGlow()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(4)->checkState() == Qt::Checked);
    m_layerStyle->innerGlow()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(5)->checkState() == Qt::Checked);
    m_layerStyle->bevelAndEmboss()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(6)->checkState() == Qt::Checked);
    m_layerStyle->bevelAndEmboss()->setContourEnabled(wdgLayerStyles.lstStyleSelector->item(7)->checkState() == Qt::Checked);
    m_layerStyle->bevelAndEmboss()->setTextureEnabled(wdgLayerStyles.lstStyleSelector->item(8)->checkState() == Qt::Checked);
    m_layerStyle->satin()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(9)->checkState() == Qt::Checked);
    m_layerStyle->colorOverlay()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(10)->checkState() == Qt::Checked);
    m_layerStyle->gradientOverlay()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(11)->checkState() == Qt::Checked);
    m_layerStyle->patternOverlay()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(12)->checkState() == Qt::Checked);
    m_layerStyle->stroke()->setEffectEnabled(wdgLayerStyles.lstStyleSelector->item(13)->checkState() == Qt::Checked);


    m_dropShadow->fetchShadow(m_layerStyle->dropShadow());
    m_innerShadow->fetchShadow(m_layerStyle->innerShadow());
    m_outerGlow->fetchConfig(m_layerStyle->outerGlow());
    m_innerGlow->fetchConfig(m_layerStyle->innerGlow());
    m_bevelAndEmboss->fetchBevelAndEmboss(m_layerStyle->bevelAndEmboss());
    m_satin->fetchSatin(m_layerStyle->satin());
    m_colorOverlay->fetchColorOverlay(m_layerStyle->colorOverlay());
    m_gradientOverlay->fetchGradientOverlay(m_layerStyle->gradientOverlay());
    m_patternOverlay->fetchPatternOverlay(m_layerStyle->patternOverlay());
    m_stroke->fetchStroke(m_layerStyle->stroke());

    m_sanityLayerStyleDirty = false;
    m_stylesSelector->notifyExternalStyleChanged(m_layerStyle->name(), m_layerStyle->uuid());

    return m_layerStyle;
}

void KisDlgLayerStyle::syncGlobalAngle(int angle)
{
    KisPSDLayerStyleSP style = this->style();

    if (style->dropShadow()->useGlobalLight()) {
        style->dropShadow()->setAngle(angle);
    }
    if (style->innerShadow()->useGlobalLight()) {
        style->innerShadow()->setAngle(angle);
    }
    if (style->bevelAndEmboss()->useGlobalLight()) {
        style->bevelAndEmboss()->setAngle(angle);
    }

    setStyle(style);
}

/********************************************************************/
/***** Styles Selector **********************************************/
/********************************************************************/

class StyleItem : public QListWidgetItem {
public:
    StyleItem(KisPSDLayerStyleSP style)
        : QListWidgetItem(style->name())
        , m_style(style)
    {
    }

public:
    KisPSDLayerStyleSP m_style;
};


StylesSelector::StylesSelector(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    connect(ui.cmbStyleCollections, SIGNAL(activated(QString)), this, SLOT(loadStyles(QString)));
    connect(ui.listStyles, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(selectStyle(QListWidgetItem*,QListWidgetItem*)));

    refillCollections();

    if (ui.cmbStyleCollections->count()) {
        ui.cmbStyleCollections->setCurrentIndex(0);
        loadStyles(ui.cmbStyleCollections->currentText());
    }
}

void StylesSelector::refillCollections()
{
    QString previousCollection = ui.cmbStyleCollections->currentText();

    ui.cmbStyleCollections->clear();
    Q_FOREACH (KoResource *res, KisResourceServerProvider::instance()->layerStyleCollectionServer()->resources()) {
        ui.cmbStyleCollections->addItem(res->name());
    }

    if (!previousCollection.isEmpty()) {
        KisSignalsBlocker blocker(this);

        int index = ui.cmbStyleCollections->findText(previousCollection);
        ui.cmbStyleCollections->setCurrentIndex(index);
    }
}

void StylesSelector::notifyExternalStyleChanged(const QString &name, const QUuid &uuid)
{
    int currentIndex = -1;

    for (int i = 0; i < ui.listStyles->count(); i++ ) {
        StyleItem *item = dynamic_cast<StyleItem*>(ui.listStyles->item(i));

        QString itemName = item->m_style->name();

        if (itemName == name) {
            bool isDirty = item->m_style->uuid() != uuid;

            if (isDirty) {
                itemName += "*";
            }

            currentIndex = i;
        }

        item->setText(itemName);
    }

    ui.listStyles->setCurrentRow(currentIndex);
}

void StylesSelector::loadStyles(const QString &name)
{
    ui.listStyles->clear();
    KoResource *res = KisResourceServerProvider::instance()->layerStyleCollectionServer()->resourceByName(name);
    KisPSDLayerStyleCollectionResource *collection = dynamic_cast<KisPSDLayerStyleCollectionResource*>(res);
    if (collection) {
        Q_FOREACH (KisPSDLayerStyleSP style, collection->layerStyles()) {
            // XXX: also use the preview image, when we have one
            ui.listStyles->addItem(new StyleItem(style));
        }
    }
}

void StylesSelector::selectStyle(QListWidgetItem *current, QListWidgetItem* /*previous*/)
{
    StyleItem *item = dynamic_cast<StyleItem*>(current);
    if (item) {
        emit styleSelected(item->m_style);
    }
}

void StylesSelector::loadCollection(const QString &fileName)
{
    if (!QFileInfo(fileName).exists()) {
        warnKrita << "Loaded style collection doesn't exist!";
        return;
    }

    KisPSDLayerStyleCollectionResource *collection =
        new KisPSDLayerStyleCollectionResource(fileName);

    collection->load();

    KoResourceServer<KisPSDLayerStyleCollectionResource> *server = KisResourceServerProvider::instance()->layerStyleCollectionServer();
    collection->setFilename(server->saveLocation() + '/' + collection->name());
    server->addResource(collection);

    refillCollections();

    int index = ui.cmbStyleCollections->findText(collection->name());
    ui.cmbStyleCollections->setCurrentIndex(index);
    loadStyles(collection->name());
}

void StylesSelector::addNewStyle(KisPSDLayerStyleSP style)
{
    KoResourceServer<KisPSDLayerStyleCollectionResource> *server = KisResourceServerProvider::instance()->layerStyleCollectionServer();

    // NOTE: not translatable, since it is a key!
    const QString customName = "CustomStyles.asl";
    const QString saveLocation = server->saveLocation();
    const QString fullFilename = saveLocation + customName;

    KoResource *resource = server->resourceByName(customName);
    KisPSDLayerStyleCollectionResource *collection = 0;

    if (!resource) {
        collection = new KisPSDLayerStyleCollectionResource("");
        collection->setName(customName);
        collection->setFilename(fullFilename);

        KisPSDLayerStyleCollectionResource::StylesVector vector;
        vector << style;
        collection->setLayerStyles(vector);

        server->addResource(collection);
    } else {
        collection = dynamic_cast<KisPSDLayerStyleCollectionResource*>(resource);

        KisPSDLayerStyleCollectionResource::StylesVector vector;
        vector = collection->layerStyles();
        vector << style;
        collection->setLayerStyles(vector);
        collection->save();
    }

    refillCollections();

    // select in gui

    int index = ui.cmbStyleCollections->findText(customName);
    KIS_ASSERT_RECOVER_RETURN(index >= 0);
    ui.cmbStyleCollections->setCurrentIndex(index);

    loadStyles(customName);

    notifyExternalStyleChanged(style->name(), style->uuid());
}

/********************************************************************/
/***** Bevel and Emboss *********************************************/
/********************************************************************/

BevelAndEmboss::BevelAndEmboss(Contour *contour, Texture *texture, QWidget *parent)
    : QWidget(parent)
    , m_contour(contour)
    , m_texture(texture)
{
    ui.setupUi(this);

    // Structure
    ui.intDepth->setRange(0, 100);
    ui.intDepth->setSuffix(i18n(" %"));

    ui.intSize->setRange(0, 250);
    ui.intSize->setSuffix(i18n(" px"));
    ui.intSize->setExponentRatio(2.0);

    ui.intSoften->setRange(0, 18);
    ui.intSoften->setSuffix(i18n(" px"));

    connect(ui.cmbStyle, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.cmbTechnique, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.intDepth, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.cmbDirection, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.intSize, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intSoften, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    // Shading
    ui.intOpacity->setRange(0, 100);
    ui.intOpacity->setSuffix(i18n(" %"));

    ui.intOpacity2->setRange(0, 100);
    ui.intOpacity2->setSuffix(i18n(" %"));

    ui.angleSelector->enableGlobalLight(true);
    connect(ui.angleSelector, SIGNAL(globalAngleChanged(int)), SIGNAL(globalAngleChanged(int)));
    connect(ui.angleSelector, SIGNAL(configChanged()), SIGNAL(configChanged()));

    connect(ui.intAltitude, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.cmbContour, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.chkAntiAliased, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.cmbHighlightMode, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.bnHighlightColor, SIGNAL(changed(KoColor)), SIGNAL(configChanged()));
    connect(ui.intOpacity, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.cmbShadowMode, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.bnShadowColor, SIGNAL(changed(KoColor)), SIGNAL(configChanged()));
    connect(ui.intOpacity2, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    // Contour
    m_contour->ui.intRange->setRange(1, 100);
    m_contour->ui.intRange->setSuffix(i18n(" %"));

    connect(m_contour->ui.cmbContour, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(m_contour->ui.chkAntiAliased, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(m_contour->ui.intRange, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    // Texture
    m_texture->ui.intScale->setRange(0, 100);
    m_texture->ui.intScale->setSuffix(i18n(" %"));

    m_texture->ui.intDepth->setRange(-1000, 1000);
    m_texture->ui.intDepth->setSuffix(i18n(" %"));

    connect(m_texture->ui.patternChooser, SIGNAL(resourceSelected(KoResource*)), SIGNAL(configChanged()));
    connect(m_texture->ui.intScale, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(m_texture->ui.intDepth, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(m_texture->ui.chkInvert, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(m_texture->ui.chkLinkWithLayer, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
}

void BevelAndEmboss::setBevelAndEmboss(const psd_layer_effects_bevel_emboss *bevelAndEmboss)
{
    ui.cmbStyle->setCurrentIndex((int)bevelAndEmboss->style());
    ui.cmbTechnique->setCurrentIndex((int)bevelAndEmboss->technique());
    ui.intDepth->setValue(bevelAndEmboss->depth());
    ui.cmbDirection->setCurrentIndex((int)bevelAndEmboss->direction());
    ui.intSize->setValue(bevelAndEmboss->size());
    ui.intSoften->setValue(bevelAndEmboss->soften());

    ui.angleSelector->setValue(bevelAndEmboss->angle());
    ui.angleSelector->setUseGlobalLight(bevelAndEmboss->useGlobalLight());

    ui.intAltitude->setValue(bevelAndEmboss->altitude());
    // FIXME: curve editing
    // ui.cmbContour;
    ui.chkAntiAliased->setChecked(bevelAndEmboss->glossAntiAliased());
    ui.cmbHighlightMode->selectCompositeOp(KoID(bevelAndEmboss->highlightBlendMode()));
    KoColor highlightshadow(KoColorSpaceRegistry::instance()->rgb8());
    highlightshadow.fromQColor(bevelAndEmboss->highlightColor());
    ui.bnHighlightColor->setColor(highlightshadow);
    ui.intOpacity->setValue(bevelAndEmboss->highlightOpacity());
    ui.cmbShadowMode->selectCompositeOp(KoID(bevelAndEmboss->shadowBlendMode()));
    highlightshadow.fromQColor(bevelAndEmboss->shadowColor());
    ui.bnShadowColor->setColor(highlightshadow);
    ui.intOpacity2->setValue(bevelAndEmboss->shadowOpacity());

    // FIXME: curve editing
    // m_contour->ui.cmbContour;
    m_contour->ui.chkAntiAliased->setChecked(bevelAndEmboss->antiAliased());
    m_contour->ui.intRange->setValue(bevelAndEmboss->contourRange());

    m_texture->ui.patternChooser->setCurrentPattern(bevelAndEmboss->texturePattern());
    m_texture->ui.intScale->setValue(bevelAndEmboss->textureScale());
    m_texture->ui.intDepth->setValue(bevelAndEmboss->textureDepth());
    m_texture->ui.chkInvert->setChecked(bevelAndEmboss->textureInvert());
    m_texture->ui.chkLinkWithLayer->setChecked(bevelAndEmboss->textureAlignWithLayer());
}

void BevelAndEmboss::fetchBevelAndEmboss(psd_layer_effects_bevel_emboss *bevelAndEmboss) const
{
    bevelAndEmboss->setStyle((psd_bevel_style)ui.cmbStyle->currentIndex());
    bevelAndEmboss->setTechnique((psd_technique_type)ui.cmbTechnique->currentIndex());
    bevelAndEmboss->setDepth(ui.intDepth->value());
    bevelAndEmboss->setDirection((psd_direction)ui.cmbDirection->currentIndex());
    bevelAndEmboss->setSize(ui.intSize->value());
    bevelAndEmboss->setSoften(ui.intSoften->value());

    bevelAndEmboss->setAngle(ui.angleSelector->value());
    bevelAndEmboss->setUseGlobalLight(ui.angleSelector->useGlobalLight());
    bevelAndEmboss->setAltitude(ui.intAltitude->value());
    bevelAndEmboss->setGlossAntiAliased(ui.chkAntiAliased->isChecked());
    bevelAndEmboss->setHighlightBlendMode(ui.cmbHighlightMode->selectedCompositeOp().id());
    bevelAndEmboss->setHighlightColor(ui.bnHighlightColor->color().toQColor());
    bevelAndEmboss->setHighlightOpacity(ui.intOpacity->value());
    bevelAndEmboss->setShadowBlendMode(ui.cmbShadowMode->selectedCompositeOp().id());
    bevelAndEmboss->setShadowColor(ui.bnShadowColor->color().toQColor());
    bevelAndEmboss->setShadowOpacity(ui.intOpacity2->value());

    // FIXME: curve editing
    bevelAndEmboss->setAntiAliased(m_contour->ui.chkAntiAliased->isChecked());
    bevelAndEmboss->setContourRange(m_contour->ui.intRange->value());

    bevelAndEmboss->setTexturePattern(static_cast<KoPattern*>(m_texture->ui.patternChooser->currentResource()));
    bevelAndEmboss->setTextureScale(m_texture->ui.intScale->value());
    bevelAndEmboss->setTextureDepth(m_texture->ui.intDepth->value());
    bevelAndEmboss->setTextureInvert(m_texture->ui.chkInvert->isChecked());
    bevelAndEmboss->setTextureAlignWithLayer(m_texture->ui.chkLinkWithLayer->isChecked());
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
/***** Contour          *********************************************/
/********************************************************************/

Contour::Contour(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}


/********************************************************************/
/***** Blending Options *********************************************/
/********************************************************************/

BlendingOptions::BlendingOptions(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    // FIXME: Blend options are not implemented yet
    ui.grpBlendingOptions->setTitle(QString("%1 (%2)").arg(ui.grpBlendingOptions->title()).arg(i18n("Not Implemented Yet")));
    ui.grpBlendingOptions->setEnabled(false);

}


/********************************************************************/
/***** Color Overlay    *********************************************/
/********************************************************************/


ColorOverlay::ColorOverlay(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.intOpacity->setRange(0, 100);
    ui.intOpacity->setSuffix(i18n(" %"));

    connect(ui.cmbCompositeOp, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.intOpacity, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.bnColor, SIGNAL(changed(KoColor)), SIGNAL(configChanged()));
}

void ColorOverlay::setColorOverlay(const psd_layer_effects_color_overlay *colorOverlay)
{
    ui.cmbCompositeOp->selectCompositeOp(KoID(colorOverlay->blendMode()));
    ui.intOpacity->setValue(colorOverlay->opacity());
    KoColor color(KoColorSpaceRegistry::instance()->rgb8());
    color.fromQColor(colorOverlay->color());
    ui.bnColor->setColor(color);
}

void ColorOverlay::fetchColorOverlay(psd_layer_effects_color_overlay *colorOverlay) const
{
    colorOverlay->setBlendMode(ui.cmbCompositeOp->selectedCompositeOp().id());
    colorOverlay->setOpacity(ui.intOpacity->value());
    colorOverlay->setColor(ui.bnColor->color().toQColor());
}


/********************************************************************/
/***** Drop Shadow **************************************************/
/********************************************************************/

DropShadow::DropShadow(Mode mode, QWidget *parent)
    : QWidget(parent),
      m_mode(mode)
{
    ui.setupUi(this);

    ui.intOpacity->setRange(0, 100);
    ui.intOpacity->setSuffix(i18n(" %"));

    ui.intDistance->setRange(0, 500);
    ui.intDistance->setSuffix(i18n(" px"));
    ui.intDistance->setExponentRatio(3.0);

    ui.intSpread->setRange(0, 100);
    ui.intSpread->setSuffix(i18n(" %"));

    ui.intSize->setRange(0, 250);
    ui.intSize->setSuffix(i18n(" px"));
    ui.intSize->setExponentRatio(2.0);

    ui.intNoise->setRange(0, 100);
    ui.intNoise->setSuffix(i18n(" %"));

    ui.angleSelector->enableGlobalLight(true);
    connect(ui.angleSelector, SIGNAL(globalAngleChanged(int)), SIGNAL(globalAngleChanged(int)));
    connect(ui.angleSelector, SIGNAL(configChanged()), SIGNAL(configChanged()));

    // connect everything to configChanged() signal
    connect(ui.cmbCompositeOp, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.intOpacity, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.bnColor, SIGNAL(changed(KoColor)), SIGNAL(configChanged()));

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
        ui.lblSpread->setText(i18n("Choke:"));
    }
}

void DropShadow::setShadow(const psd_layer_effects_shadow_common *shadow)
{
    ui.cmbCompositeOp->selectCompositeOp(KoID(shadow->blendMode()));
    ui.intOpacity->setValue(shadow->opacity());
    KoColor color(KoColorSpaceRegistry::instance()->rgb8());
    color.fromQColor(shadow->color());
    ui.bnColor->setColor(color);

    ui.angleSelector->setValue(shadow->angle());
    ui.angleSelector->setUseGlobalLight(shadow->useGlobalLight());

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
    shadow->setOpacity(ui.intOpacity->value());
    shadow->setColor(ui.bnColor->color().toQColor());

    shadow->setAngle(ui.angleSelector->value());
    shadow->setUseGlobalLight(ui.angleSelector->useGlobalLight());

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

class GradientPointerConverter
{
public:
    static KoAbstractGradientSP resourceToStyle(KoAbstractGradient *gradient) {
        return gradient ? KoAbstractGradientSP(gradient->clone()) : KoAbstractGradientSP();
    }

    static KoAbstractGradient* styleToResource(KoAbstractGradientSP gradient) {
        if (!gradient) return 0;

        KoResourceServer<KoAbstractGradient> *server = KoResourceServerProvider::instance()->gradientServer();
        KoAbstractGradient *resource = server->resourceByMD5(gradient->md5());

        if (!resource) {
            KoAbstractGradient *clone = gradient->clone();
            clone->setName(findAvailableName(gradient->name()));
            server->addResource(clone, false);
            resource = clone;
        }

        return resource;
    }

private:
    static QString findAvailableName(const QString &name) {
        KoResourceServer<KoAbstractGradient> *server = KoResourceServerProvider::instance()->gradientServer();
        QString newName = name;
        int i = 0;

        while (server->resourceByName(newName)) {
            newName = QString("%1%2").arg(name).arg(i++);
        }

        return newName;
    }
};

/********************************************************************/
/***** Gradient Overlay *********************************************/
/********************************************************************/

GradientOverlay::GradientOverlay(KisCanvasResourceProvider *resourceProvider, QWidget *parent)
    : QWidget(parent),
      m_resourceProvider(resourceProvider)
{
    ui.setupUi(this);

    ui.intOpacity->setRange(0, 100);
    ui.intOpacity->setSuffix(i18n(" %"));

    ui.intScale->setRange(0, 100);
    ui.intScale->setSuffix(i18n(" %"));

    ui.angleSelector->angleSelector()->setResetAngle(90.0);

    connect(ui.angleSelector, SIGNAL(configChanged()), SIGNAL(configChanged()));

    connect(ui.cmbCompositeOp, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.intOpacity, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.cmbGradient, SIGNAL(gradientChanged(KoAbstractGradient*)), SIGNAL(configChanged()));
    connect(ui.chkReverse, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.cmbStyle, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.chkAlignWithLayer, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.intScale, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
}

void GradientOverlay::setGradientOverlay(const psd_layer_effects_gradient_overlay *config)
{
    ui.cmbCompositeOp->selectCompositeOp(KoID(config->blendMode()));
    ui.intOpacity->setValue(config->opacity());

    KoAbstractGradient *gradient = fetchGradientLazy(
        GradientPointerConverter::styleToResource(config->gradient()), m_resourceProvider);

    if (gradient) {
        ui.cmbGradient->setGradient(gradient);
    }

    ui.chkReverse->setChecked(config->reverse());
    ui.cmbStyle->setCurrentIndex((int)config->style());
    ui.chkAlignWithLayer->setCheckable(config->alignWithLayer());
    ui.angleSelector->setValue(config->angle());
    ui.intScale->setValue(config->scale());
}

void GradientOverlay::fetchGradientOverlay(psd_layer_effects_gradient_overlay *config) const
{
    config->setBlendMode(ui.cmbCompositeOp->selectedCompositeOp().id());
    config->setOpacity(ui.intOpacity->value());
    config->setGradient(GradientPointerConverter::resourceToStyle(ui.cmbGradient->gradient()));
    config->setReverse(ui.chkReverse->isChecked());
    config->setStyle((psd_gradient_style)ui.cmbStyle->currentIndex());
    config->setAlignWithLayer(ui.chkAlignWithLayer->isChecked());
    config->setAngle(ui.angleSelector->value());
    config->setScale(ui.intScale->value());
}


/********************************************************************/
/***** Innner Glow      *********************************************/
/********************************************************************/

InnerGlow::InnerGlow(Mode mode, KisCanvasResourceProvider *resourceProvider, QWidget *parent)
    : QWidget(parent),
      m_mode(mode),
      m_resourceProvider(resourceProvider)
{
    ui.setupUi(this);

    ui.intOpacity->setRange(0, 100);
    ui.intOpacity->setSuffix(i18n(" %"));

    ui.intNoise->setRange(0, 100);
    ui.intNoise->setSuffix(i18n(" %"));

    ui.intChoke->setRange(0, 100);
    ui.intChoke->setSuffix(i18n(" %"));

    ui.intSize->setRange(0, 250);
    ui.intSize->setSuffix(i18n(" px"));
    ui.intSize->setExponentRatio(2.0);

    ui.intRange->setRange(1, 100);
    ui.intRange->setSuffix(i18n(" %"));

    ui.intJitter->setRange(0, 100);
    ui.intJitter->setSuffix(i18n(" %"));

    connect(ui.cmbCompositeOp, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.intOpacity, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intNoise, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    connect(ui.radioColor, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.bnColor, SIGNAL(changed(KoColor)), SIGNAL(configChanged()));
    connect(ui.radioGradient, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.cmbGradient, SIGNAL(gradientChanged(KoAbstractGradient*)), SIGNAL(configChanged()));

    connect(ui.cmbTechnique, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.cmbSource, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.intChoke, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intSize, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    connect(ui.cmbContour, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.chkAntiAliased, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.intRange, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intJitter, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    if (m_mode == OuterGlowMode) {
        ui.cmbSource->hide();
        ui.lblSource->hide();
        ui.lblChoke->setText(i18nc("layer styles parameter", "Spread:"));
    }

}

void InnerGlow::setConfig(const psd_layer_effects_glow_common *config)
{
    ui.cmbCompositeOp->selectCompositeOp(KoID(config->blendMode()));
    ui.intOpacity->setValue(config->opacity());
    ui.intNoise->setValue(config->noise());

    ui.radioColor->setChecked(config->fillType() == psd_fill_solid_color);
    KoColor color(KoColorSpaceRegistry::instance()->rgb8());
    color.fromQColor(config->color());
    ui.bnColor->setColor(color);
    ui.radioGradient->setChecked(config->fillType() == psd_fill_gradient);

    KoAbstractGradient *gradient = fetchGradientLazy(
        GradientPointerConverter::styleToResource(config->gradient()), m_resourceProvider);

    if (gradient) {
        ui.cmbGradient->setGradient(gradient);
    }

    ui.cmbTechnique->setCurrentIndex((int)config->technique());
    ui.intChoke->setValue(config->spread());
    ui.intSize->setValue(config->size());

    if (m_mode == InnerGlowMode) {
        const psd_layer_effects_inner_glow *iglow =
            dynamic_cast<const psd_layer_effects_inner_glow *>(config);
        KIS_ASSERT_RECOVER_RETURN(iglow);

        ui.cmbSource->setCurrentIndex(iglow->source() == psd_glow_edge);
    }

    // FIXME: Curve editing
    //ui.cmbContour;

    ui.chkAntiAliased->setChecked(config->antiAliased());
    ui.intRange->setValue(config->range());
    ui.intJitter->setValue(config->jitter());
}

void InnerGlow::fetchConfig(psd_layer_effects_glow_common *config) const
{
    config->setBlendMode(ui.cmbCompositeOp->selectedCompositeOp().id());
    config->setOpacity(ui.intOpacity->value());
    config->setNoise(ui.intNoise->value());

    if (ui.radioColor->isChecked()) {
        config->setFillType(psd_fill_solid_color);
    }
    else {
        config->setFillType(psd_fill_gradient);
    }

    config->setColor(ui.bnColor->color().toQColor());
    config->setGradient(GradientPointerConverter::resourceToStyle(ui.cmbGradient->gradient()));
    config->setTechnique((psd_technique_type)ui.cmbTechnique->currentIndex());
    config->setSpread(ui.intChoke->value());
    config->setSize(ui.intSize->value());

    if (m_mode == InnerGlowMode) {
        psd_layer_effects_inner_glow *iglow =
            dynamic_cast<psd_layer_effects_inner_glow *>(config);
        KIS_ASSERT_RECOVER_RETURN(iglow);

        iglow->setSource((psd_glow_source)ui.cmbSource->currentIndex());
    }

    // FIXME: Curve editing
    //ui.cmbContour;

    config->setAntiAliased(ui.chkAntiAliased->isChecked());
    config->setRange(ui.intRange->value());
    config->setJitter(ui.intJitter->value());
}


/********************************************************************/
/***** Pattern Overlay  *********************************************/
/********************************************************************/


PatternOverlay::PatternOverlay(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.intOpacity->setRange(0, 100);
    ui.intOpacity->setSuffix(i18n(" %"));

    ui.intScale->setRange(0, 100);
    ui.intScale->setSuffix(i18n(" %"));

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
    ui.intScale->setValue(pattern->scale());
}

void PatternOverlay::fetchPatternOverlay(psd_layer_effects_pattern_overlay *pattern) const
{
    pattern->setBlendMode(ui.cmbCompositeOp->selectedCompositeOp().id());
    pattern->setOpacity(ui.intOpacity->value());
    pattern->setPattern(static_cast<KoPattern*>(ui.patternChooser->currentResource()));
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

    ui.intOpacity->setRange(0, 100);
    ui.intOpacity->setSuffix(i18n(" %"));

    ui.intDistance->setRange(0, 250);
    ui.intDistance->setSuffix(i18n(" px"));

    ui.intSize->setRange(0, 250);
    ui.intSize->setSuffix(i18n(" px"));
    ui.intSize->setExponentRatio(2.0);

    connect(ui.cmbCompositeOp, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.bnColor, SIGNAL(changed(KoColor)), SIGNAL(configChanged()));
    connect(ui.intOpacity, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    connect(ui.angleSelector, SIGNAL(configChanged()), SIGNAL(configChanged()));
    connect(ui.intDistance, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.intSize, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    connect(ui.cmbContour, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.chkAntiAliased, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.chkInvert, SIGNAL(toggled(bool)), SIGNAL(configChanged()));

}

void Satin::setSatin(const psd_layer_effects_satin *satin)
{
    ui.cmbCompositeOp->selectCompositeOp(KoID(satin->blendMode()));
    KoColor color(KoColorSpaceRegistry::instance()->rgb8());
    color.fromQColor(satin->color());
    ui.bnColor->setColor(color);
    ui.intOpacity->setValue(satin->opacity());

    ui.angleSelector->setValue(satin->angle());

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
    satin->setOpacity(ui.intOpacity->value());
    satin->setColor(ui.bnColor->color().toQColor());

    satin->setAngle(ui.angleSelector->value());

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

Stroke::Stroke(KisCanvasResourceProvider *resourceProvider, QWidget *parent)
    : QWidget(parent),
      m_resourceProvider(resourceProvider)
{
    ui.setupUi(this);

    ui.intSize->setRange(0, 250);
    ui.intSize->setSuffix(i18n(" px"));
    ui.intSize->setExponentRatio(2.0);

    ui.intOpacity->setRange(0, 100);
    ui.intOpacity->setSuffix(i18n(" %"));

    ui.intScale->setRange(0, 100);
    ui.intScale->setSuffix(i18n(" %"));

    ui.intScale_2->setRange(0, 100);
    ui.intScale_2->setSuffix(i18n(" %"));

    connect(ui.cmbFillType, SIGNAL(currentIndexChanged(int)), ui.fillStack, SLOT(setCurrentIndex(int)));

    connect(ui.intSize, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));
    connect(ui.cmbPosition, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.cmbCompositeOp, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.intOpacity, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    connect(ui.cmbFillType, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));

    connect(ui.bnColor, SIGNAL(changed(KoColor)), SIGNAL(configChanged()));

    connect(ui.cmbGradient, SIGNAL(gradientChanged(KoAbstractGradient*)), SIGNAL(configChanged()));
    connect(ui.chkReverse, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.cmbStyle, SIGNAL(currentIndexChanged(int)), SIGNAL(configChanged()));
    connect(ui.chkAlignWithLayer, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.intScale, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    connect(ui.angleSelector, SIGNAL(configChanged()), SIGNAL(configChanged()));

    connect(ui.patternChooser, SIGNAL(resourceSelected(KoResource*)), SIGNAL(configChanged()));
    connect(ui.chkLinkWithLayer, SIGNAL(toggled(bool)), SIGNAL(configChanged()));
    connect(ui.intScale_2, SIGNAL(valueChanged(int)), SIGNAL(configChanged()));

    // cold initialization
    ui.fillStack->setCurrentIndex(ui.cmbFillType->currentIndex());
}

void Stroke::setStroke(const psd_layer_effects_stroke *stroke)
{

    ui.intSize->setValue(stroke->size());
    ui.cmbPosition->setCurrentIndex((int)stroke->position());
    ui.cmbCompositeOp->selectCompositeOp(KoID(stroke->blendMode()));
    ui.intOpacity->setValue(stroke->opacity());

    ui.cmbFillType->setCurrentIndex((int)stroke->fillType());
    KoColor color(KoColorSpaceRegistry::instance()->rgb8());
    color.fromQColor(stroke->color());
    ui.bnColor->setColor(color);

    KoAbstractGradient *gradient =
        fetchGradientLazy(GradientPointerConverter::styleToResource(stroke->gradient()), m_resourceProvider);

    if (gradient) {
        ui.cmbGradient->setGradient(gradient);
    }

    ui.chkReverse->setChecked(stroke->antiAliased());
    ui.cmbStyle->setCurrentIndex((int)stroke->style());
    ui.chkAlignWithLayer->setCheckable(stroke->alignWithLayer());
    ui.angleSelector->setValue(stroke->angle());
    ui.intScale->setValue(stroke->scale());

    ui.patternChooser->setCurrentPattern(stroke->pattern());
    ui.chkLinkWithLayer->setChecked(stroke->alignWithLayer());
    ui.intScale_2->setValue(stroke->scale());

}

void Stroke::fetchStroke(psd_layer_effects_stroke *stroke) const
{
    stroke->setSize(ui.intSize->value());
    stroke->setPosition((psd_stroke_position)ui.cmbPosition->currentIndex());
    stroke->setBlendMode(ui.cmbCompositeOp->selectedCompositeOp().id());
    stroke->setOpacity(ui.intOpacity->value());

    stroke->setFillType((psd_fill_type)ui.cmbFillType->currentIndex());

    stroke->setColor(ui.bnColor->color().toQColor());

    stroke->setGradient(GradientPointerConverter::resourceToStyle(ui.cmbGradient->gradient()));
    stroke->setReverse(ui.chkReverse->isChecked());
    stroke->setStyle((psd_gradient_style)ui.cmbStyle->currentIndex());
    stroke->setAlignWithLayer(ui.chkAlignWithLayer->isChecked());
    stroke->setAngle(ui.angleSelector->value());
    stroke->setScale(ui.intScale->value());

    stroke->setPattern(static_cast<KoPattern*>(ui.patternChooser->currentResource()));
    stroke->setAlignWithLayer(ui.chkLinkWithLayer->isChecked());
    stroke->setScale(ui.intScale->value());
}
