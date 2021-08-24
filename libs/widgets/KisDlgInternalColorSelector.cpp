/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QList>
#include <QAbstractSpinBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPointer>
#include <QCompleter>

#include <functional>

#include <KConfigGroup>
#include <kstandardguiitem.h>

#include "KoColorSpaceRegistry.h"
#include <KoColorSet.h>
#include <KisPaletteModel.h>
#include <KisPaletteChooser.h>
#include <kis_palette_view.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServer.h>

#include "kis_signal_compressor.h"
#include "KoColorDisplayRendererInterface.h"

#include "kis_spinbox_color_selector.h"

#include "KisDlgInternalColorSelector.h"
#include "ui_WdgDlgInternalColorSelector.h"
#include "kis_config_notifier.h"
#include "kis_color_input.h"
#include "kis_icon_utils.h"
#include "KisSqueezedComboBox.h"

std::function<KisScreenColorSamplerBase *(QWidget *)> KisDlgInternalColorSelector::s_screenColorSamplerFactory = 0;

struct KisDlgInternalColorSelector::Private
{
    bool allowUpdates = true;
    KoColor currentColor;
    KoColor previousColor;
    KoColor sRGB = KoColor(KoColorSpaceRegistry::instance()->rgb8());
    const KoColorSpace *currentColorSpace;
    bool lockUsedCS = false;
    bool chooseAlpha = false;
    KisSignalCompressor *compressColorChanges;
    const KoColorDisplayRendererInterface *displayRenderer;
    KisHexColorInput *hexColorInput = 0;
    KisPaletteModel *paletteModel = 0;
    KisPaletteChooser *paletteChooser = 0;
    KisScreenColorSamplerBase *screenColorSampler = 0;
};

KisDlgInternalColorSelector::KisDlgInternalColorSelector(QWidget *parent, KoColor color, Config config, const QString &caption, const KoColorDisplayRendererInterface *displayRenderer)
    : QDialog(parent)
    , m_d(new Private)
{
    setModal(config.modal);
    setFocusPolicy(Qt::ClickFocus);
    m_ui = new Ui_WdgDlgInternalColorSelector();
    m_ui->setupUi(this);

    setWindowTitle(caption);

    m_d->currentColor = color;
    m_d->currentColorSpace = m_d->currentColor.colorSpace();
    m_d->displayRenderer = displayRenderer;

    m_ui->spinboxselector->slotSetColor(color);
    connect(m_ui->spinboxselector, SIGNAL(sigNewColor(KoColor)), this, SLOT(slotColorUpdated(KoColor)));

    m_ui->spinboxHSXSelector->attachToSelector(m_ui->visualSelector);

    m_ui->visualSelector->setDisplayRenderer(displayRenderer);
    m_ui->visualSelector->setConfig(false, config.modal);
    if (config.visualColorSelector) {
        connect(m_ui->visualSelector, SIGNAL(sigNewColor(KoColor)), this, SLOT(slotColorUpdated(KoColor)));
        connect(m_ui->visualSelector, SIGNAL(sigColorModelChanged()), this, SLOT(slotSelectorModelChanged()));
        connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), m_ui->visualSelector, SLOT(configurationChanged()));
    } else {
        m_ui->visualSelector->hide();
    }
    m_ui->visualSelector->slotSetColor(color);

    m_d->paletteChooser = new KisPaletteChooser(this);
    m_d->paletteModel = new KisPaletteModel(this);
    m_ui->bnPaletteChooser->setIcon(KisIconUtils::loadIcon("palette-library"));
    m_ui->paletteBox->setPaletteModel(m_d->paletteModel);
    m_ui->paletteBox->setDisplayRenderer(displayRenderer);
    m_ui->cmbNameList->setCompanionView(m_ui->paletteBox);
    connect(m_d->paletteChooser, SIGNAL(sigPaletteSelected(KoColorSetSP)), this, SLOT(slotChangePalette(KoColorSetSP)));
    connect(m_ui->cmbNameList, SIGNAL(sigColorSelected(KoColor)), SLOT(slotColorUpdated(KoColor)));

    // For some bizarre reason, the modal dialog doesn't like having the colorset set, so let's not.
    if (config.paletteBox) {
        //TODO: Add disable signal as well. Might be not necessary...?
        KConfigGroup cfg(KSharedConfig::openConfig()->group(""));
        QString paletteMd5 = cfg.readEntry("internal_selector_active_color_set_md5", QString());
        QString paletteName = cfg.readEntry("internal_selector_active_color_set", QString());
        KoResourceServer<KoColorSet>* rServer = KoResourceServerProvider::instance()->paletteServer();
        KoColorSetSP savedPal = rServer->resource(paletteMd5, "", paletteName);
        if (savedPal) {
            this->slotChangePalette(savedPal);
        } else {
            if (rServer->resourceCount()) {
                savedPal = rServer->firstResource();
                if (savedPal) {
                    this->slotChangePalette(savedPal);
                }
            }
        }

        connect(m_ui->paletteBox, SIGNAL(sigColorSelected(KoColor)), this,
                SLOT(slotColorUpdated(KoColor)));
        m_ui->bnPaletteChooser->setPopupWidget(m_d->paletteChooser);
    } else {
        m_ui->paletteBox->setEnabled(false);
        m_ui->cmbNameList->setEnabled(false);
        m_ui->bnPaletteChooser->setEnabled(false);
    }

    if (config.prevNextButtons) {
        m_ui->currentColor->setColor(m_d->currentColor);
        m_ui->currentColor->setDisplayRenderer(displayRenderer);
        m_ui->previousColor->setColor(m_d->previousColor);
        m_ui->previousColor->setDisplayRenderer(displayRenderer);
        connect(m_ui->previousColor, SIGNAL(triggered(KoColorPatch*)), SLOT(slotSetColorFromPatch(KoColorPatch*)));
    } else {
        m_ui->currentColor->hide();
        m_ui->previousColor->hide();
    }

    if (config.hexInput) {
        m_d->sRGB.fromKoColor(m_d->currentColor);
        m_d->hexColorInput = new KisHexColorInput(this, &m_d->sRGB);
        m_d->hexColorInput->update();
        connect(m_d->hexColorInput, SIGNAL(updated()), SLOT(slotSetColorFromHex()));
        m_ui->rightPane->addWidget(m_d->hexColorInput);
        m_d->hexColorInput->setToolTip(i18n("This is a hexcode input, for webcolors. It can only get colors in the sRGB space."));
    }

    // KisScreenColorSampler is in the kritaui module, so dependency inversion is used to access it.
    m_ui->screenColorSamplerWidget->setLayout(new QHBoxLayout());
    if (s_screenColorSamplerFactory) {
        m_d->screenColorSampler = s_screenColorSamplerFactory(m_ui->screenColorSamplerWidget);
        m_ui->screenColorSamplerWidget->layout()->addWidget(m_d->screenColorSampler);
        if (config.screenColorSampler) {
            connect(m_d->screenColorSampler, SIGNAL(sigNewColorSampled(KoColor)),this, SLOT(slotColorUpdated(KoColor)));
        } else {
            m_d->screenColorSampler->hide();
        }
    }

    m_d->compressColorChanges = new KisSignalCompressor(100 /* ms */, KisSignalCompressor::POSTPONE, this);
    connect(m_d->compressColorChanges, SIGNAL(timeout()), this, SLOT(endUpdateWithNewColor()));

    KGuiItem::assign(m_ui->buttonBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(m_ui->buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
    connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()), Qt::UniqueConnection);
    connect(m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()), Qt::UniqueConnection);

    connect(this, SIGNAL(finished(int)), SLOT(slotFinishUp()));
}

KisDlgInternalColorSelector::~KisDlgInternalColorSelector()
{
    delete m_ui;
}

void KisDlgInternalColorSelector::slotColorUpdated(KoColor newColor)
{
    // not-so-nice solution: if someone calls this slot directly and that code was
    // triggered by our compressor signal, our compressor is technically the sender()!
    if (sender() == m_d->compressColorChanges) {
        return;
    }
    // Do not accept external updates while a color update emit is pending;
    // Note: Assumes external updates only come from parent(), a separate slot might be better
    if (m_d->allowUpdates || (QObject::sender() && QObject::sender() != this->parent())) {
        // Enforce palette colors
        KConfigGroup group(KSharedConfig::openConfig(), "");
        if (group.readEntry("colorsettings/forcepalettecolors", false)) {
            newColor = m_ui->paletteBox->closestColor(newColor);
        }

        if (m_d->lockUsedCS){
            newColor.convertTo(m_d->currentColorSpace);
        } else {
            colorSpaceChanged(newColor.colorSpace());
        }
        m_d->currentColor = newColor;
        updateAllElements(QObject::sender());
    }
}

void KisDlgInternalColorSelector::slotSetColorFromPatch(KoColorPatch *patch)
{
    slotColorUpdated(patch->color());
}

void KisDlgInternalColorSelector::colorSpaceChanged(const KoColorSpace *cs)
{
    if (cs == m_d->currentColorSpace) {
        return;
    }

    m_d->currentColorSpace = KoColorSpaceRegistry::instance()->colorSpace(cs->colorModelId().id(), cs->colorDepthId().id(), cs->profile());
    m_ui->spinboxselector->slotSetColorSpace(m_d->currentColorSpace);
    m_ui->visualSelector->slotSetColorSpace(m_d->currentColorSpace);

}

void KisDlgInternalColorSelector::lockUsedColorSpace(const KoColorSpace *cs)
{
    colorSpaceChanged(cs);
    if (m_d->currentColor.colorSpace() != m_d->currentColorSpace) {
        m_d->currentColor.convertTo(m_d->currentColorSpace);
        m_ui->spinboxselector->slotSetColor(m_d->currentColor);
        m_ui->visualSelector->slotSetColor(m_d->currentColor);
    }
    m_d->lockUsedCS = true;
}

void KisDlgInternalColorSelector::setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer)
{
    if (displayRenderer) {
        m_d->displayRenderer = displayRenderer;
        m_ui->visualSelector->setDisplayRenderer(displayRenderer);
        m_ui->currentColor->setDisplayRenderer(displayRenderer);
        m_ui->previousColor->setDisplayRenderer(displayRenderer);
        m_ui->paletteBox->setDisplayRenderer(displayRenderer);
    } else {
        m_d->displayRenderer = KoDumbColorDisplayRenderer::instance();
    }
}

KoColor KisDlgInternalColorSelector::getModalColorDialog(const KoColor color, QWidget* parent, QString caption)
{
    Config config = Config();
    KisDlgInternalColorSelector dialog(parent, color, config, caption);
    dialog.setPreviousColor(color);
    dialog.exec();
    return dialog.getCurrentColor();
}

KoColor KisDlgInternalColorSelector::getCurrentColor()
{
    return m_d->currentColor;
}

void KisDlgInternalColorSelector::chooseAlpha(bool chooseAlpha)
{
    m_d->chooseAlpha = chooseAlpha;
}

void KisDlgInternalColorSelector::setPreviousColor(KoColor c)
{
    m_d->previousColor = c;
}

void KisDlgInternalColorSelector::reject()
{
    slotColorUpdated(m_d->previousColor);
    QDialog::reject();
}

void KisDlgInternalColorSelector::updateAllElements(QObject *source)
{
    //update everything!!!
    if (source != m_ui->spinboxselector) {
        m_ui->spinboxselector->slotSetColor(m_d->currentColor);
    }
    if (source != m_ui->visualSelector) {
        m_ui->visualSelector->slotSetColor(m_d->currentColor);
    }

    if (source != m_d->hexColorInput) {
        m_d->sRGB.fromKoColor(m_d->currentColor);
        m_d->hexColorInput->update();
    }

    if (source != m_ui->paletteBox) {
        m_ui->paletteBox->selectClosestColor(m_d->currentColor);
    }

    m_ui->previousColor->setColor(m_d->previousColor);

    m_ui->currentColor->setColor(m_d->currentColor);

    if (source && source != this->parent()) {
        m_d->allowUpdates = false;
        m_d->compressColorChanges->start();
    }

    if (m_d->screenColorSampler) {
        m_d->screenColorSampler->updateIcons();
    }
}

void KisDlgInternalColorSelector::slotSelectorModelChanged()
{
    if (m_ui->visualSelector->isHSXModel()) {
        QString label;
        switch (m_ui->visualSelector->getColorModel()) {
        case KisVisualColorSelector::HSV:
            label = i18n("HSV");
            break;
        case KisVisualColorSelector::HSL:
            label = i18n("HSL");
            break;
        case KisVisualColorSelector::HSI:
            label = i18n("HSI");
            break;
        case KisVisualColorSelector::HSY:
            label = i18n("HSY'");
            break;
        default:
            label =  i18n("Unknown");
        }
        if (m_ui->tabWidget->count() == 1) {
            m_ui->tabWidget->addTab(m_ui->tab_hsx, label);
        }
        else {
            m_ui->tabWidget->setTabText(1, label);
        }
    }
    else {
        if (m_ui->tabWidget->count() == 2) {
            m_ui->tabWidget->removeTab(1);
        }
    }
}

void KisDlgInternalColorSelector::endUpdateWithNewColor()
{
    emit signalForegroundColorChosen(m_d->currentColor);
    m_d->allowUpdates = true;
}

void KisDlgInternalColorSelector::focusInEvent(QFocusEvent *)
{
    //setPreviousColor();
}

void KisDlgInternalColorSelector::slotFinishUp()
{
    setPreviousColor(m_d->currentColor);
    KConfigGroup cfg(KSharedConfig::openConfig()->group(""));
    if (m_d->paletteModel) {
        if (m_d->paletteModel->colorSet()) {
            cfg.writeEntry("internal_selector_active_color_set_md5", m_d->paletteModel->colorSet()->md5Sum());
            cfg.writeEntry("internal_selector_active_color_set", m_d->paletteModel->colorSet()->name());
        }
    }
}

void KisDlgInternalColorSelector::slotSetColorFromHex()
{
    slotColorUpdated(m_d->sRGB);
}

void KisDlgInternalColorSelector::slotChangePalette(KoColorSetSP set)
{
    if (!set) {
        return;
    }
    m_d->paletteModel->setPalette(set);
}

void KisDlgInternalColorSelector::showEvent(QShowEvent *event)
{
    updateAllElements(0);
    QDialog::showEvent(event);
}

