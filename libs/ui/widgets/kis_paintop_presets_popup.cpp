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
#include <QWidgetAction>

#include <kconfig.h>
#include <klocalizedstring.h>

#include <KoDockRegistry.h>

#include <kis_icon.h>
#include <kis_icon.h>
#include <brushengine/kis_paintop_preset.h>
#include <brushengine/kis_paintop_config_widget.h>
#include <kis_canvas_resource_provider.h>
#include <widgets/kis_preset_chooser.h>
#include <widgets/kis_preset_selector_strip.h>

#include <ui_wdgpaintopsettings.h>
#include <kis_node.h>
#include "kis_config.h"

#include "kis_resource_server_provider.h"
#include "kis_lod_availability_widget.h"

#include "kis_signal_auto_connection.h"


// ones from brush engine selector
#include <brushengine/kis_paintop_factory.h>
#include "../kis_paint_ops_model.h"



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

    KisSignalAutoConnectionsStore widgetConnections;
};

KisPaintOpPresetsPopup::KisPaintOpPresetsPopup(KisCanvasResourceProvider * resourceProvider, QWidget * parent)
    : QWidget(parent)
    , m_d(new Private())
{
    setObjectName("KisPaintOpPresetsPopup");
    setFont(KoDockRegistry::dockFont());

    current_paintOpId = "";

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


    // DETAIL and THUMBNAIL view changer
    QMenu* menu = new QMenu(this);

    menu->setStyleSheet("margin: 6px");
    menu->addSection(i18n("Display"));

    QActionGroup *actionGroup = new QActionGroup(this);

    KisPresetChooser::ViewMode mode = (KisPresetChooser::ViewMode)KisConfig().presetChooserViewMode();

    QAction* action = menu->addAction(KisIconUtils::loadIcon("view-preview"), i18n("Thumbnails"), m_d->uiWdgPaintOpPresetSettings.presetWidget, SLOT(slotThumbnailMode()));
    action->setCheckable(true);
    action->setChecked(mode == KisPresetChooser::THUMBNAIL);
    action->setActionGroup(actionGroup);

    action = menu->addAction(KisIconUtils::loadIcon("view-list-details"), i18n("Details"), m_d->uiWdgPaintOpPresetSettings.presetWidget, SLOT(slotDetailMode()));
    action->setCheckable(true);
    action->setChecked(mode == KisPresetChooser::DETAIL);
    action->setActionGroup(actionGroup);


    // add horizontal slider for the icon size
    QSlider* iconSizeSlider = new QSlider(this);
    iconSizeSlider->setOrientation(Qt::Horizontal);
    iconSizeSlider->setRange(30, 80);
    iconSizeSlider->setValue(m_d->uiWdgPaintOpPresetSettings.presetWidget->iconSize());
    iconSizeSlider->setMinimumHeight(20);
    iconSizeSlider->setMinimumWidth(40);
    iconSizeSlider->setTickInterval(10);


    QWidgetAction *sliderAction= new QWidgetAction(this);
    sliderAction->setDefaultWidget(iconSizeSlider);

    menu->addSection(i18n("Icon Size"));
    menu->addAction(sliderAction);




    // configure the button and assign menu
    m_d->uiWdgPaintOpPresetSettings.presetChangeViewToolButton->setMenu(menu);
    m_d->uiWdgPaintOpPresetSettings.presetChangeViewToolButton->setIcon(KisIconUtils::loadIcon("view-choose"));
    m_d->uiWdgPaintOpPresetSettings.presetChangeViewToolButton->setPopupMode(QToolButton::InstantPopup);


    // show/hide buttons

    KisConfig cfg;

    m_d->uiWdgPaintOpPresetSettings.showScratchpadButton->setCheckable(true);
    m_d->uiWdgPaintOpPresetSettings.showScratchpadButton->setChecked(cfg.scratchpadVisible());
    m_d->uiWdgPaintOpPresetSettings.showEditorButton->setCheckable(true);
    m_d->uiWdgPaintOpPresetSettings.showEditorButton->setChecked(true);
    m_d->uiWdgPaintOpPresetSettings.detachWindowButton->setCheckable(true);
    m_d->uiWdgPaintOpPresetSettings.detachWindowButton->setChecked(cfg.paintopPopupDetached());


    m_d->uiWdgPaintOpPresetSettings.pinWindowButton->setCheckable(true);
    m_d->uiWdgPaintOpPresetSettings.pinWindowButton->setChecked(false);
    m_d->uiWdgPaintOpPresetSettings.pinWindowButton->setVisible(false);

    m_d->uiWdgPaintOpPresetSettings.pinWindowButton->setIcon(KisIconUtils::loadIcon("layer-unlocked"));
    m_d->uiWdgPaintOpPresetSettings.detachWindowButton->setVisible(true);



    // Connections


    connect(iconSizeSlider, SIGNAL(sliderMoved(int)),
            m_d->uiWdgPaintOpPresetSettings.presetWidget, SLOT(slotSetIconSize(int)));

    connect(iconSizeSlider, SIGNAL(sliderReleased()),
            m_d->uiWdgPaintOpPresetSettings.presetWidget, SLOT(slotSaveIconSize()));


    connect(m_d->uiWdgPaintOpPresetSettings.showScratchpadButton, SIGNAL(clicked(bool)),
            this, SLOT(slotSwitchScratchpad(bool)));

    connect(m_d->uiWdgPaintOpPresetSettings.showEditorButton, SIGNAL(clicked(bool)),
            this, SLOT(slotSwitchShowEditor(bool)));

    connect(m_d->uiWdgPaintOpPresetSettings.detachWindowButton, SIGNAL(clicked(bool)),
            this, SLOT(switchDetached(bool)));

    connect (m_d->uiWdgPaintOpPresetSettings.pinWindowButton, SIGNAL(clicked(bool)),
             this, SLOT(slotPinWindow(bool)));

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

    connect(m_d->uiWdgPaintOpPresetSettings.eraserBrushOpacityCheckBox, SIGNAL(toggled(bool)),
            this, SIGNAL(eraserBrushOpacityToggled(bool)));    
    
    connect(m_d->uiWdgPaintOpPresetSettings.bnDefaultPreset, SIGNAL(clicked()),
            m_d->uiWdgPaintOpPresetSettings.txtPreset, SLOT(clear()));

    connect(m_d->uiWdgPaintOpPresetSettings.txtPreset, SIGNAL(textChanged(QString)),
            SLOT(slotWatchPresetNameLineEdit()));

    connect(m_d->uiWdgPaintOpPresetSettings.brushEgineComboBox, SIGNAL(activated(QString)),
            this, SIGNAL(paintopActivated(QString)));


    // preset widget connections
    connect(m_d->uiWdgPaintOpPresetSettings.presetWidget->smallPresetChooser, SIGNAL(resourceSelected(KoResource*)),
            this, SIGNAL(signalResourceSelected(KoResource*)));

    connect(m_d->uiWdgPaintOpPresetSettings.bnSave, SIGNAL(clicked()),
            m_d->uiWdgPaintOpPresetSettings.presetWidget->smallPresetChooser, SLOT(updateViewSettings()));

    connect(m_d->uiWdgPaintOpPresetSettings.reload, SIGNAL(clicked()),
            m_d->uiWdgPaintOpPresetSettings.presetWidget->smallPresetChooser, SLOT(updateViewSettings()));



    m_d->detached = !cfg.paintopPopupDetached();
    m_d->ignoreHideEvents = false;
    m_d->minimumSettingsWidgetSize = QSize(0, 0);
    m_d->uiWdgPaintOpPresetSettings.presetWidget->setVisible(cfg.presetStripVisible());
    m_d->uiWdgPaintOpPresetSettings.scratchpadControls->setVisible(cfg.scratchpadVisible());
    m_d->detachedGeometry = QRect(100, 100, 0, 0);
    m_d->uiWdgPaintOpPresetSettings.dirtyPresetCheckBox->setChecked(cfg.useDirtyPresets());
    m_d->uiWdgPaintOpPresetSettings.eraserBrushSizeCheckBox->setChecked(cfg.useEraserBrushSize());
    m_d->uiWdgPaintOpPresetSettings.eraserBrushOpacityCheckBox->setChecked(cfg.useEraserBrushOpacity());

    m_d->uiWdgPaintOpPresetSettings.wdgLodAvailability->setCanvasResourceManager(resourceProvider->resourceManager());

    // brush engine is changed
   connect(m_d->uiWdgPaintOpPresetSettings.brushEgineComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPaintOpChanged(int)));

   connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(slotFocusChanged(QWidget*,QWidget*)));





}

void KisPaintOpPresetsPopup::slotFocusChanged(QWidget* object1, QWidget* object2)
{
    // object1 = "old" focus window. object2 = "active" focus window
    // only potentially hide the brush editor if it is detached and NOT pinned
    if ( m_d->uiWdgPaintOpPresetSettings.pinWindowButton->isChecked() == false &&
         m_d->uiWdgPaintOpPresetSettings.detachWindowButton->isChecked() == true) {

        // if the active focus window is not the brush editor, we need to hide it
        // otherwise it will go behind the active window and require 2 clicks to bring it back
        // once to hide it, second time to show it again at the top
        if (object1 && object2 && object2->parentWidget() && parentWidget() ) {


            // the depth of the windows will change once the main window is clicked
            // Since it gets focus, it will be in front of the brush editor (which means greater depth)
            if ( object2->depth() > object1->depth()) {
                 parentWidget()->hide();
            }
        }
    }

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

    m_d->widgetConnections.clear();
    m_d->settingsWidget = 0;

    if (widget) {

        m_d->settingsWidget = dynamic_cast<KisPaintOpConfigWidget*>(widget);
        KIS_ASSERT_RECOVER_RETURN(m_d->settingsWidget);

        if (m_d->settingsWidget->supportScratchBox()) {
            showScratchPad();
        } else {
            hideScratchPad();
        }

        m_d->widgetConnections.addConnection(m_d->uiWdgPaintOpPresetSettings.wdgLodAvailability, SIGNAL(sigUserChangedLodAvailability(bool)),
                                             m_d->settingsWidget, SLOT(slotUserChangedLodAvailability(bool)));
        m_d->widgetConnections.addConnection(m_d->settingsWidget, SIGNAL(sigUserChangedLodAvailability(bool)),
                                             m_d->uiWdgPaintOpPresetSettings.wdgLodAvailability, SLOT(slotUserChangedLodAvailability(bool)));
        m_d->widgetConnections.addConnection(m_d->settingsWidget, SIGNAL(sigConfigurationItemChanged()),
                                             this, SLOT(slotUpdateLodAvailability()));

        m_d->settingsWidget->coldInitExternalLodAvailabilityWidget();


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

void KisPaintOpPresetsPopup::slotUpdateLodAvailability()
{
    if (!m_d->settingsWidget) return;

    KisPaintopLodLimitations l = m_d->settingsWidget->lodLimitations();
    m_d->uiWdgPaintOpPresetSettings.wdgLodAvailability->setLimitations(l);
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

void KisPaintOpPresetsPopup::contextMenuEvent(QContextMenuEvent *e)
{
    Q_UNUSED(e);

#if 0
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
#endif
}

void KisPaintOpPresetsPopup::switchDetached(bool show)
{
    if (parentWidget()) {

        m_d->detached = !m_d->detached;

        if (m_d->detached) {
            m_d->ignoreHideEvents = true;
            parentWidget()->setWindowFlags(Qt::Tool);

            if (show) {
                parentWidget()->show();
            }
            m_d->ignoreHideEvents = false;

            m_d->uiWdgPaintOpPresetSettings.pinWindowButton->setVisible(true); // allow to keep window on top

        }
        else {
            parentWidget()->setWindowFlags(Qt::Popup);
            KisConfig cfg;
            parentWidget()->hide();

            // turn off pinning since it doesn't apply
            m_d->uiWdgPaintOpPresetSettings.pinWindowButton->setVisible(false);
            m_d->uiWdgPaintOpPresetSettings.pinWindowButton->setChecked(false);
            m_d->uiWdgPaintOpPresetSettings.pinWindowButton->setIcon(KisIconUtils::loadIcon("layer-unlocked"));
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
    m_d->uiWdgPaintOpPresetSettings.presetWidget->smallPresetChooser->setCurrentResource(resource);
    m_d->uiWdgPaintOpPresetSettings.txtPreset->setText(resource->name());
    slotWatchPresetNameLineEdit();
}

void KisPaintOpPresetsPopup::setPaintOpList(const QList< KisPaintOpFactory* >& list)
{
   // fill in the available brush engines
   KisSortedPaintOpListModel* brushEnginesModel = new KisSortedPaintOpListModel(this);
   brushEnginesModel->fill(list);

   m_d->uiWdgPaintOpPresetSettings.brushEgineComboBox->setModel(brushEnginesModel);

}


void KisPaintOpPresetsPopup::setCurrentPaintOp(const QString& paintOpId)
{
    // iterate through the items and find the engine we need the combo box to switch to
    for (int i= 0; i < m_d->uiWdgPaintOpPresetSettings.brushEgineComboBox->count(); i++) {

        KisSortedPaintOpListModel* brushEnginesModel = static_cast<KisSortedPaintOpListModel*>( m_d->uiWdgPaintOpPresetSettings.brushEgineComboBox->model() );
        KisPaintOpInfo info;

        if(brushEnginesModel->entryAt(info, brushEnginesModel->index(i, 0))) {

           if (info.id == paintOpId ) {
               current_paintOpId = paintOpId;

               m_d->uiWdgPaintOpPresetSettings.brushEgineComboBox->setCurrentIndex(i); // found it!
           }

        }

    }

    m_d->uiWdgPaintOpPresetSettings.presetWidget->setPresetFilter(paintOpId);

}

QString KisPaintOpPresetsPopup::currentPaintOp()
{
    return m_d->uiWdgPaintOpPresetSettings.brushEgineComboBox->currentText();
}

QString KisPaintOpPresetsPopup::currentPaintOpId() {
    return current_paintOpId;
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
    emit brushEditorShown();
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

void KisPaintOpPresetsPopup::slotPinWindow(bool enabled)
{

    if (enabled) {
          parentWidget()->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
            m_d->uiWdgPaintOpPresetSettings.pinWindowButton->setIcon(KisIconUtils::loadIcon("layer-locked"));
      }
      else {
          parentWidget()->setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
            m_d->uiWdgPaintOpPresetSettings.pinWindowButton->setIcon(KisIconUtils::loadIcon("layer-unlocked"));
      }

      parentWidget()->show(); // switching window state wants to hide window. keep it shown
}


void KisPaintOpPresetsPopup::slotSwitchShowEditor(bool visible) {
   m_d->uiWdgPaintOpPresetSettings.brushEditorSettingsControls->setVisible(visible);
}

void KisPaintOpPresetsPopup::slotPaintOpChanged(int index) {

    // find the brush engine model ID from the combo box's index
    KisSortedPaintOpListModel* brushEnginesModel = static_cast<KisSortedPaintOpListModel*>( m_d->uiWdgPaintOpPresetSettings.brushEgineComboBox->model() );
    KisPaintOpInfo info;

    if(brushEnginesModel->entryAt(info, brushEnginesModel->index(index, 0)))
        current_paintOpId = info.id;

    // add filter for the presets in the brush editor
    m_d->uiWdgPaintOpPresetSettings.presetWidget->setPresetFilter(current_paintOpId);
    setCurrentPaintOp(current_paintOpId);

}


void KisPaintOpPresetsPopup::updateViewSettings()
{
    m_d->uiWdgPaintOpPresetSettings.presetWidget->smallPresetChooser->updateViewSettings();
}

void KisPaintOpPresetsPopup::currentPresetChanged(KisPaintOpPresetSP preset)
{
     m_d->uiWdgPaintOpPresetSettings.presetWidget->smallPresetChooser->setCurrentResource(preset.data());
}

void KisPaintOpPresetsPopup::updateThemedIcons()
 {
    m_d->uiWdgPaintOpPresetSettings.fillLayer->setIcon(KisIconUtils::loadIcon("document-new"));
    m_d->uiWdgPaintOpPresetSettings.fillLayer->hide();
    m_d->uiWdgPaintOpPresetSettings.fillGradient->setIcon(KisIconUtils::loadIcon("krita_tool_gradient"));
    m_d->uiWdgPaintOpPresetSettings.fillSolid->setIcon(KisIconUtils::loadIcon("krita_tool_color_fill"));
    m_d->uiWdgPaintOpPresetSettings.eraseScratchPad->setIcon(KisIconUtils::loadIcon("edit-delete"));
    m_d->uiWdgPaintOpPresetSettings.paintPresetIcon->setIcon(KisIconUtils::loadIcon("krita_tool_freehand"));
    m_d->uiWdgPaintOpPresetSettings.presetChangeViewToolButton->setIcon(KisIconUtils::loadIcon("view-choose"));

    if (m_d->uiWdgPaintOpPresetSettings.pinWindowButton->isChecked()) {
        m_d->uiWdgPaintOpPresetSettings.pinWindowButton->setIcon(KisIconUtils::loadIcon("layer-locked"));
    } else {
               m_d->uiWdgPaintOpPresetSettings.pinWindowButton->setIcon(KisIconUtils::loadIcon("layer-unlocked"));
    }


}
