/* This file is part of the KDE project
 * Copyright (C) 2008 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 * Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "widgets/kis_paintop_presets_popup.h"

#include <QList>
#include <QComboBox>
#include <QHBoxLayout>
#include <QToolButton>
#include <QGridLayout>
#include <QFont>
#include <QMenu>
#include <QAction>
#include <QShowEvent>
#include <QFontDatabase>

#include <kconfig.h>
#include <klocalizedstring.h>

#include <kis_icon_utils.h>
#include <kis_icon_utils.h>
#include <kis_paintop_preset.h>
#include <kis_paintop_config_widget.h>
#include <kis_canvas_resource_provider.h>
#include <widgets/kis_preset_chooser.h>
#include <widgets/kis_preset_selector_strip.h>

#include <ui_wdgpaintopsettings.h>
#include <kis_node.h>
#include "kis_config.h"

#include "kis_resource_server_provider.h"


struct KisPaintOpPresetsPopup::Private
{

public:

    Ui_WdgPaintOpSettings uiWdgPaintOpPresetSettings;
    QGridLayout *layout;
    KisPaintOpConfigWidget *settingsWidget;
    QFont smallFont;
    KisCanvasResourceProvider *resourceProvider;
    bool detached;
    bool ignoreHideEvents;
    QSize minimumSettingsWidgetSize;
    QRect detachedGeometry;
};

KisPaintOpPresetsPopup::KisPaintOpPresetsPopup(KisCanvasResourceProvider * resourceProvider, QWidget * parent)
    : QWidget(parent)
    , m_d(new Private())
{
    setObjectName("KisPaintOpPresetsPopup");
    KConfigGroup group( KSharedConfig::openConfig(), "GUI");
    m_d->smallFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    qreal pointSize = group.readEntry("palettefontsize", m_d->smallFont.pointSize() * 0.75);
    pointSize = qMax(pointSize, QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont).pointSizeF());
    m_d->smallFont.setPointSizeF(pointSize);
    setFont(m_d->smallFont);

    m_d->resourceProvider = resourceProvider;

    m_d->uiWdgPaintOpPresetSettings.setupUi(this);

    m_d->layout = new QGridLayout(m_d->uiWdgPaintOpPresetSettings.frmOptionWidgetContainer);
    m_d->layout->setSizeConstraint(QLayout::SetFixedSize);

    m_d->uiWdgPaintOpPresetSettings.scratchPad->setupScratchPad(resourceProvider, Qt::white);
    m_d->uiWdgPaintOpPresetSettings.scratchPad->setCutoutOverlayRect(QRect(25, 25, 200, 200));
    m_d->uiWdgPaintOpPresetSettings.fillLayer->setIcon(KisIconUtils::loadIcon("document-new"));
    m_d->uiWdgPaintOpPresetSettings.fillLayer->hide();
    m_d->uiWdgPaintOpPresetSettings.fillGradient->setIcon(KisIconUtils::loadIcon("krita_tool_gradient"));
    m_d->uiWdgPaintOpPresetSettings.fillSolid->setIcon(KisIconUtils::loadIcon("krita_tool_color_fill"));
    m_d->uiWdgPaintOpPresetSettings.eraseScratchPad->setIcon(KisIconUtils::loadIcon("edit-delete"));
    m_d->uiWdgPaintOpPresetSettings.paintPresetIcon->setIcon(KisIconUtils::loadIcon("krita_tool_freehand"));

    connect(m_d->uiWdgPaintOpPresetSettings.eraseScratchPad, SIGNAL(clicked()),
            m_d->uiWdgPaintOpPresetSettings.scratchPad, SLOT(fillDefault()));

    connect(m_d->uiWdgPaintOpPresetSettings.fillLayer, SIGNAL(clicked()),
            m_d->uiWdgPaintOpPresetSettings.scratchPad, SLOT(fillLayer()));

    connect(m_d->uiWdgPaintOpPresetSettings.fillGradient, SIGNAL(clicked()),
            m_d->uiWdgPaintOpPresetSettings.scratchPad, SLOT(fillGradient()));

    connect(m_d->uiWdgPaintOpPresetSettings.fillSolid, SIGNAL(clicked()),
            m_d->uiWdgPaintOpPresetSettings.scratchPad, SLOT(fillBackground()));

    connect(m_d->uiWdgPaintOpPresetSettings.paintPresetIcon, SIGNAL(clicked()),
            m_d->uiWdgPaintOpPresetSettings.scratchPad, SLOT(paintPresetImage()));

    m_d->settingsWidget = 0;
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    connect(m_d->uiWdgPaintOpPresetSettings.bnSave, SIGNAL(clicked()),
            this, SIGNAL(savePresetClicked()));

    connect(m_d->uiWdgPaintOpPresetSettings.reload, SIGNAL(clicked()),
            this, SIGNAL(reloadPresetClicked()));

    connect(m_d->uiWdgPaintOpPresetSettings.bnDefaultPreset, SIGNAL(clicked()),
            this, SIGNAL(defaultPresetClicked()));

    connect(m_d->uiWdgPaintOpPresetSettings.dirtyPresetCheckBox, SIGNAL(toggled(bool)),
            this, SIGNAL(dirtyPresetToggled(bool)));

    connect(m_d->uiWdgPaintOpPresetSettings.eraserBrushSizeCheckBox, SIGNAL(toggled(bool)),
            this, SIGNAL(eraserBrushSizeToggled(bool)));

    connect(m_d->uiWdgPaintOpPresetSettings.bnDefaultPreset, SIGNAL(clicked()),
            m_d->uiWdgPaintOpPresetSettings.txtPreset, SLOT(clear()));

    connect(m_d->uiWdgPaintOpPresetSettings.txtPreset, SIGNAL(textChanged(QString)),
            SLOT(slotWatchPresetNameLineEdit()));

    connect(m_d->uiWdgPaintOpPresetSettings.paintopList, SIGNAL(activated(QString)),
            this, SIGNAL(paintopActivated(QString)));

    connect(m_d->uiWdgPaintOpPresetSettings.presetWidget->smallPresetChooser, SIGNAL(resourceSelected(KoResource*)),
            this, SIGNAL(signalResourceSelected(KoResource*)));

    connect(m_d->uiWdgPaintOpPresetSettings.bnSave, SIGNAL(clicked()),
            m_d->uiWdgPaintOpPresetSettings.presetWidget->smallPresetChooser, SLOT(updateViewSettings()));

    connect(m_d->uiWdgPaintOpPresetSettings.reload, SIGNAL(clicked()),
            m_d->uiWdgPaintOpPresetSettings.presetWidget->smallPresetChooser, SLOT(updateViewSettings()));

    KisConfig cfg;
    m_d->detached = !cfg.paintopPopupDetached();
    m_d->ignoreHideEvents = false;
    m_d->minimumSettingsWidgetSize = QSize(0, 0);
    m_d->uiWdgPaintOpPresetSettings.presetWidget->setVisible(cfg.presetStripVisible());
    m_d->uiWdgPaintOpPresetSettings.scratchpadControls->setVisible(cfg.scratchpadVisible());
    m_d->detachedGeometry = QRect(100, 100, 0, 0);
    m_d->uiWdgPaintOpPresetSettings.dirtyPresetCheckBox->setChecked(cfg.useDirtyPresets());
    m_d->uiWdgPaintOpPresetSettings.eraserBrushSizeCheckBox->setChecked(cfg.useEraserBrushSize());
}


KisPaintOpPresetsPopup::~KisPaintOpPresetsPopup()
{
    if (m_d->settingsWidget) {
        m_d->layout->removeWidget(m_d->settingsWidget);
        m_d->settingsWidget->hide();
        m_d->settingsWidget->setParent(0);
        m_d->settingsWidget = 0;
    }
    delete m_d;
}

void KisPaintOpPresetsPopup::setPaintOpSettingsWidget(QWidget * widget)
{
    if (m_d->settingsWidget) {
        m_d->layout->removeWidget(m_d->settingsWidget);
        m_d->uiWdgPaintOpPresetSettings.frmOptionWidgetContainer->updateGeometry();
    }
    m_d->layout->update();
    updateGeometry();

    m_d->settingsWidget = static_cast<KisPaintOpConfigWidget*>(widget);
    if (m_d->settingsWidget){
        if (m_d->settingsWidget->supportScratchBox()) {
            showScratchPad();
        }
        else {
            hideScratchPad();
        }
    }

    if (widget) {
        widget->setFont(m_d->smallFont);

        QSize hint = widget->sizeHint();
        m_d->minimumSettingsWidgetSize = QSize(qMax(hint.width(), m_d->minimumSettingsWidgetSize.width()),
                                               qMax(hint.height(), m_d->minimumSettingsWidgetSize.height()));
        widget->setMinimumSize(m_d->minimumSettingsWidgetSize);
        m_d->layout->addWidget(widget);

        m_d->layout->update();
        widget->show();
    }
}

void KisPaintOpPresetsPopup::slotWatchPresetNameLineEdit()
{
    QString text = m_d->uiWdgPaintOpPresetSettings.txtPreset->text();

    KisPaintOpPresetResourceServer * rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    bool overwrite = rServer->resourceByName(text) != 0;

    KisPaintOpPresetSP preset = m_d->resourceProvider->currentPreset();

    bool btnSaveAvailable = preset->valid() &&
        (preset->isPresetDirty() | !overwrite);

    QString btnText = overwrite ? i18n("Overwrite Preset") : i18n("Save to Presets");

    m_d->uiWdgPaintOpPresetSettings.bnSave->setText(btnText);
    m_d->uiWdgPaintOpPresetSettings.bnSave->setEnabled(btnSaveAvailable);

    m_d->uiWdgPaintOpPresetSettings.reload->setVisible(true);
    m_d->uiWdgPaintOpPresetSettings.reload->setEnabled(btnSaveAvailable && overwrite);

    QFont font = m_d->uiWdgPaintOpPresetSettings.txtPreset->font();
    font.setItalic(btnSaveAvailable);
    m_d->uiWdgPaintOpPresetSettings.txtPreset->setFont(font);
}

QString KisPaintOpPresetsPopup::getPresetName() const
{
    return m_d->uiWdgPaintOpPresetSettings.txtPreset->text();
}

QImage KisPaintOpPresetsPopup::cutOutOverlay()
{
    return m_d->uiWdgPaintOpPresetSettings.scratchPad->cutoutOverlay();
}

void KisPaintOpPresetsPopup::contextMenuEvent(QContextMenuEvent *e) {

    QMenu menu(this);
    QAction* action = menu.addAction(m_d->detached ? i18n("Attach to Toolbar") : i18n("Detach from Toolbar"));
    connect(action, SIGNAL(triggered()), this, SLOT(switchDetached()));
    QAction* showPresetStrip = menu.addAction(i18n("Show Preset Strip"));
    showPresetStrip->setCheckable(true);
    showPresetStrip->setChecked(m_d->uiWdgPaintOpPresetSettings.presetWidget->isVisible());
    connect(showPresetStrip, SIGNAL(triggered(bool)), this, SLOT(slotSwitchPresetStrip(bool)));
    QAction* showScratchPad = menu.addAction(i18n("Show Scratchpad"));
    showScratchPad->setCheckable(true);
    showScratchPad->setChecked(m_d->uiWdgPaintOpPresetSettings.scratchPad->isVisible());
    connect(showScratchPad, SIGNAL(triggered(bool)), this, SLOT(slotSwitchScratchpad(bool)));
    menu.exec(e->globalPos());
}

void KisPaintOpPresetsPopup::switchDetached(bool show)
{
    if (parentWidget()) {

        m_d->detached = !m_d->detached;

        if (m_d->detached) {
            m_d->ignoreHideEvents = true;
            parentWidget()->setWindowFlags(Qt::Tool);
            m_d->uiWdgPaintOpPresetSettings.scratchpadControls->setVisible(false);
            if (show) {
                parentWidget()->show();
            }
            m_d->ignoreHideEvents = false;
        }
        else {
            parentWidget()->setWindowFlags(Qt::Popup);
            KisConfig cfg;
            m_d->uiWdgPaintOpPresetSettings.scratchpadControls->setVisible(cfg.scratchpadVisible());
            parentWidget()->hide();
        }

        KisConfig cfg;
        cfg.setPaintopPopupDetached(m_d->detached);
    }
}

void KisPaintOpPresetsPopup::hideScratchPad()
{
    m_d->uiWdgPaintOpPresetSettings.scratchPad->setEnabled(false);
    m_d->uiWdgPaintOpPresetSettings.fillGradient->setEnabled(false);
    m_d->uiWdgPaintOpPresetSettings.fillSolid->setEnabled(false);
    m_d->uiWdgPaintOpPresetSettings.eraseScratchPad->setEnabled(false);
}

void KisPaintOpPresetsPopup::showScratchPad()
{
    m_d->uiWdgPaintOpPresetSettings.scratchPad->setEnabled(true);
    m_d->uiWdgPaintOpPresetSettings.fillGradient->setEnabled(true);
    m_d->uiWdgPaintOpPresetSettings.fillSolid->setEnabled(true);
    m_d->uiWdgPaintOpPresetSettings.eraseScratchPad->setEnabled(true);
}

void KisPaintOpPresetsPopup::resourceSelected(KoResource* resource)
{
    m_d->uiWdgPaintOpPresetSettings.txtPreset->setText(resource->name());
    slotWatchPresetNameLineEdit();
}

void KisPaintOpPresetsPopup::setPaintOpList(const QList< KisPaintOpFactory* >& list)
{
   m_d->uiWdgPaintOpPresetSettings.paintopList->setPaintOpList(list);
}

void KisPaintOpPresetsPopup::setCurrentPaintOp(const QString& paintOpId)
{
    m_d->uiWdgPaintOpPresetSettings.paintopList->setCurrent(paintOpId);
    m_d->uiWdgPaintOpPresetSettings.presetWidget->setPresetFilter(paintOpId);
}

QString KisPaintOpPresetsPopup::currentPaintOp()
{
    return m_d->uiWdgPaintOpPresetSettings.paintopList->currentItem();
}

void KisPaintOpPresetsPopup::setPresetImage(const QImage& image)
{
    m_d->uiWdgPaintOpPresetSettings.scratchPad->setPresetImage(image);
}

void KisPaintOpPresetsPopup::hideEvent(QHideEvent *event)
{
    if (m_d->ignoreHideEvents) {
        return;
    }
    if (m_d->detached) {
        m_d->detachedGeometry = window()->geometry();
    }
    QWidget::hideEvent(event);
}

void KisPaintOpPresetsPopup::showEvent(QShowEvent *)
{
    if (m_d->detached) {
        window()->setGeometry(m_d->detachedGeometry);
    }
}

void KisPaintOpPresetsPopup::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    emit sizeChanged();
}

bool KisPaintOpPresetsPopup::detached() const
{
    return m_d->detached;
}

void KisPaintOpPresetsPopup::slotSwitchPresetStrip(bool visible)
{
    m_d->uiWdgPaintOpPresetSettings.presetWidget->setVisible(visible);
    KisConfig cfg;
    cfg.setPresetStripVisible(visible);
}

void KisPaintOpPresetsPopup::slotSwitchScratchpad(bool visible)
{
    m_d->uiWdgPaintOpPresetSettings.scratchpadControls->setVisible(visible);
    KisConfig cfg;
    cfg.setScratchpadVisible(visible);
}

void KisPaintOpPresetsPopup::updateViewSettings()
{
    m_d->uiWdgPaintOpPresetSettings.presetWidget->smallPresetChooser->updateViewSettings();
}

void KisPaintOpPresetsPopup::updateThemedIcons()
 {
    m_d->uiWdgPaintOpPresetSettings.fillLayer->setIcon(KisIconUtils::loadIcon("document-new"));
    m_d->uiWdgPaintOpPresetSettings.fillLayer->hide();
    m_d->uiWdgPaintOpPresetSettings.fillGradient->setIcon(KisIconUtils::loadIcon("krita_tool_gradient"));
    m_d->uiWdgPaintOpPresetSettings.fillSolid->setIcon(KisIconUtils::loadIcon("krita_tool_color_fill"));
    m_d->uiWdgPaintOpPresetSettings.eraseScratchPad->setIcon(KisIconUtils::loadIcon("edit-delete"));
    m_d->uiWdgPaintOpPresetSettings.paintPresetIcon->setIcon(KisIconUtils::loadIcon("krita_tool_freehand"));
}
