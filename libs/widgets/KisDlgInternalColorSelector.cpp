/*
 * Copyright (C) Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>, (C) 2016
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

#include <QList>
#include <QAbstractSpinBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPointer>
#include <QCompleter>

#include <functional>

#include <KConfigGroup>

#include "KoColorSpaceRegistry.h"
#include <KoColorSet.h>
#include <KisPaletteModel.h>
#include <KisPaletteListWidget.h>
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

std::function<KisScreenColorPickerBase *(QWidget *)> KisDlgInternalColorSelector::s_screenColorPickerFactory = 0;

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
    KisPaletteListWidget *paletteChooser = 0;
    KisScreenColorPickerBase *screenColorPicker = 0;
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

    m_ui->visualSelector->slotSetColor(color);
    m_ui->visualSelector->setDisplayRenderer(displayRenderer);
    m_ui->visualSelector->setConfig(false, config.modal);
    if (config.visualColorSelector) {
        connect(m_ui->visualSelector, SIGNAL(sigNewColor(KoColor)), this, SLOT(slotColorUpdated(KoColor)));
        connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), m_ui->visualSelector, SLOT(configurationChanged()));
    } else {
        m_ui->visualSelector->hide();
    }

    m_d->paletteChooser = new KisPaletteListWidget(this);
    m_d->paletteModel = new KisPaletteModel(this);
    m_ui->bnPaletteChooser->setIcon(KisIconUtils::loadIcon("hi16-palette_library"));
    m_ui->paletteBox->setPaletteModel(m_d->paletteModel);
    m_ui->paletteBox->setDisplayRenderer(displayRenderer);
    m_ui->cmbNameList->setCompanionView(m_ui->paletteBox);
    connect(m_d->paletteChooser, SIGNAL(sigPaletteSelected(KoColorSet*)), this, SLOT(slotChangePalette(KoColorSet*)));
    connect(m_ui->cmbNameList, SIGNAL(sigColorSelected(KoColor)), SLOT(slotColorUpdated(KoColor)));

    // For some bizare reason, the modal dialog doesn't like having the colorset set, so let's not.
    if (config.paletteBox) {
        //TODO: Add disable signal as well. Might be not necessary...?
        KConfigGroup cfg(KSharedConfig::openConfig()->group(""));
        QString paletteName = cfg.readEntry("internal_selector_active_color_set", QString());
        KoResourceServer<KoColorSet>* rServer = KoResourceServerProvider::instance()->paletteServer();
        KoColorSet *savedPal = rServer->resourceByName(paletteName);
        if (savedPal) {
            this->slotChangePalette(savedPal);
        } else {
            if (rServer->resources().count()) {
                savedPal = rServer->resources().first();
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
        m_ui->previousColor->setColor(m_d->currentColor);
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

    // KisScreenColorPicker is in the kritaui module, so dependency inversion is used to access it.
    m_ui->screenColorPickerWidget->setLayout(new QHBoxLayout(m_ui->screenColorPickerWidget));
    if (s_screenColorPickerFactory) {
        m_d->screenColorPicker = s_screenColorPickerFactory(m_ui->screenColorPickerWidget);
        m_ui->screenColorPickerWidget->layout()->addWidget(m_d->screenColorPicker);
        if (config.screenColorPicker) {
            connect(m_d->screenColorPicker, SIGNAL(sigNewColorPicked(KoColor)),this, SLOT(slotColorUpdated(KoColor)));
        } else {
            m_d->screenColorPicker->hide();
        }
    }

    connect(this, SIGNAL(signalForegroundColorChosen(KoColor)), this, SLOT(slotLockSelector()));
    m_d->compressColorChanges = new KisSignalCompressor(100 /* ms */, KisSignalCompressor::POSTPONE, this);
    connect(m_d->compressColorChanges, SIGNAL(timeout()), this, SLOT(endUpdateWithNewColor()));

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
    //if the update did not come from this selector...
    if (m_d->allowUpdates || QObject::sender() == this->parent()) {
        if (m_d->lockUsedCS){
            newColor.convertTo(m_d->currentColorSpace);
            m_d->currentColor = newColor;
        } else {
            m_d->currentColor = newColor;
        }
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
    m_ui->visualSelector->slotsetColorSpace(m_d->currentColorSpace);

}

void KisDlgInternalColorSelector::lockUsedColorSpace(const KoColorSpace *cs)
{
    colorSpaceChanged(cs);
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

void KisDlgInternalColorSelector::slotConfigurationChanged()
{
    //m_d->canvas->displayColorConverter()->
    //slotColorSpaceChanged(m_d->canvas->image()->colorSpace());
}

void KisDlgInternalColorSelector::slotLockSelector()
{
    m_d->allowUpdates = false;
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

    KConfigGroup group(KSharedConfig::openConfig(), "");
    if (source != m_ui->paletteBox && group.readEntry("colorsettings/forcepalettecolors", false)) {
        m_ui->paletteBox->selectClosestColor(m_d->currentColor);
    }

    m_ui->previousColor->setColor(m_d->previousColor);

    m_ui->currentColor->setColor(m_d->currentColor);

    if (source != this->parent()) {
        emit(signalForegroundColorChosen(m_d->currentColor));
        m_d->compressColorChanges->start();
    }

    if (m_d->screenColorPicker) {
        m_d->screenColorPicker->updateIcons();
    }
}


void KisDlgInternalColorSelector::endUpdateWithNewColor()
{
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
            cfg.writeEntry("internal_selector_active_color_set", m_d->paletteModel->colorSet()->name());
        }
    }
}

void KisDlgInternalColorSelector::slotSetColorFromHex()
{
    slotColorUpdated(m_d->sRGB);
}

void KisDlgInternalColorSelector::slotChangePalette(KoColorSet *set)
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

