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

#include <kconfig.h>
#include <kglobalsettings.h>
#include <klocale.h>

#include <KoColorSpaceRegistry.h>

#include <KoIcon.h>
#include <kis_paintop_preset.h>
#include <kis_paintop_settings_widget.h>
#include <kis_canvas_resource_provider.h>
#include <widgets/kis_preset_chooser.h>
#include <widgets/kis_preset_selector_strip.h>

#include <ui_wdgpaintopsettings.h>
#include <kis_node.h>
#include "kis_config.h"


struct KisPaintOpPresetsPopup::Private
{

public:

    Ui_WdgPaintOpSettings uiWdgPaintOpPresetSettings;
    QGridLayout *layout;
    KisPaintOpSettingsWidget *settingsWidget;
    QFont smallFont;
    KisCanvasResourceProvider *resourceProvider;
    bool detached;
    bool ignoreHideEvents;
    QSize minimumSettingsWidgetSize;
};

KisPaintOpPresetsPopup::KisPaintOpPresetsPopup(KisCanvasResourceProvider * resourceProvider, QWidget * parent)
    : QWidget(parent)
    , m_d(new Private())
{
    setObjectName("KisPaintOpPresetsPopup");
    KConfigGroup group(KGlobal::config(), "GUI");
    m_d->smallFont  = KGlobalSettings::generalFont();
    qreal pointSize = group.readEntry("palettefontsize", m_d->smallFont.pointSize() * 0.75);
    pointSize = qMax(pointSize, KGlobalSettings::smallestReadableFont().pointSizeF());
    m_d->smallFont.setPointSizeF(pointSize);
    setFont(m_d->smallFont);

    m_d->resourceProvider = resourceProvider;

    m_d->uiWdgPaintOpPresetSettings.setupUi(this);

    m_d->layout = new QGridLayout(m_d->uiWdgPaintOpPresetSettings.frmOptionWidgetContainer);
    m_d->layout->setSizeConstraint(QLayout::SetFixedSize);

    m_d->uiWdgPaintOpPresetSettings.scratchPad->setupScratchPad(resourceProvider, Qt::white);
    m_d->uiWdgPaintOpPresetSettings.scratchPad->setCutoutOverlayRect(QRect(25, 25, 200, 200));
    m_d->uiWdgPaintOpPresetSettings.fillLayer->setIcon(koIcon("newlayer"));
    m_d->uiWdgPaintOpPresetSettings.fillLayer->hide();
    m_d->uiWdgPaintOpPresetSettings.fillGradient->setIcon(koIcon("krita_tool_gradient"));
    m_d->uiWdgPaintOpPresetSettings.fillSolid->setIcon(koIcon("krita_tool_color_fill"));
    m_d->uiWdgPaintOpPresetSettings.eraseScratchPad->setIcon(koIcon("edit-clear"));
    m_d->uiWdgPaintOpPresetSettings.paintPresetIcon->setIcon(koIcon("krita_paintop_icon"));

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

    connect(m_d->uiWdgPaintOpPresetSettings.bnDefaultPreset, SIGNAL(clicked()),
            this, SIGNAL(defaultPresetClicked()));
                        
    connect(m_d->uiWdgPaintOpPresetSettings.bnDefaultPreset, SIGNAL(clicked()),
            m_d->uiWdgPaintOpPresetSettings.txtPreset, SLOT(clear()));

    connect(m_d->uiWdgPaintOpPresetSettings.txtPreset, SIGNAL(textChanged(QString)),
            this, SIGNAL(presetNameLineEditChanged(QString)));

    connect(m_d->uiWdgPaintOpPresetSettings.paintopList, SIGNAL(activated(const QString&)),
            this, SIGNAL(paintopActivated(QString)));

    connect(this, SIGNAL(paintopActivated(QString)),
            m_d->uiWdgPaintOpPresetSettings.presetWidget, SLOT(currentPaintopChanged(QString)));

    connect(m_d->uiWdgPaintOpPresetSettings.presetWidget->smallPresetChooser, SIGNAL(resourceSelected(KoResource*)),
            this, SIGNAL(signalResourceSelected(KoResource*)));
    
    connect(m_d->uiWdgPaintOpPresetSettings.bnSave, SIGNAL(clicked()),
            m_d->uiWdgPaintOpPresetSettings.presetWidget->smallPresetChooser, SLOT(updateViewSettings()));
            
    KisConfig cfg;
    m_d->detached = !cfg.paintopPopupDetached();
    m_d->ignoreHideEvents = false;
    m_d->minimumSettingsWidgetSize = QSize(0, 0);
}


KisPaintOpPresetsPopup::~KisPaintOpPresetsPopup()
{
    if (m_d->settingsWidget)
    {
        m_d->layout->removeWidget(m_d->settingsWidget);
        m_d->settingsWidget->hide();
        m_d->settingsWidget->setParent(0);
        m_d->settingsWidget = 0;
    }
    delete m_d;
}

void KisPaintOpPresetsPopup::slotCheckPresetValidity()
{
    if (m_d->settingsWidget){
        m_d->uiWdgPaintOpPresetSettings.bnSave->setEnabled( m_d->settingsWidget->presetIsValid() );
        m_d->uiWdgPaintOpPresetSettings.txtPreset->setEnabled( m_d->settingsWidget->presetIsValid() );
    }
}


void KisPaintOpPresetsPopup::setPaintOpSettingsWidget(QWidget * widget)
{
    if (m_d->settingsWidget) {
        m_d->layout->removeWidget(m_d->settingsWidget);
        m_d->uiWdgPaintOpPresetSettings.frmOptionWidgetContainer->updateGeometry();
    }
    m_d->layout->update();
    updateGeometry();

    m_d->settingsWidget = static_cast<KisPaintOpSettingsWidget*>(widget);
    if (m_d->settingsWidget){
        connect(m_d->settingsWidget,SIGNAL(sigConfigurationItemChanged()),this,SLOT(slotCheckPresetValidity()));
        slotCheckPresetValidity();
        if (m_d->settingsWidget->supportScratchBox()){
            showScratchPad();
        }else{
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

void KisPaintOpPresetsPopup::changeSavePresetButtonText(bool change)
{
    QPalette palette;

    if (change) {
        palette.setColor(QPalette::Base, QColor(255,200,200));
        m_d->uiWdgPaintOpPresetSettings.bnSave->setText(i18n("Overwrite Preset"));
        m_d->uiWdgPaintOpPresetSettings.txtPreset->setPalette(palette);
    }
    else {
        m_d->uiWdgPaintOpPresetSettings.bnSave->setText(i18n("Save to Presets"));
        m_d->uiWdgPaintOpPresetSettings.txtPreset->setPalette(palette);
    }
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
    menu.exec(e->globalPos());
}

void KisPaintOpPresetsPopup::switchDetached()
{
    if (parentWidget()) {

        m_d->detached = !m_d->detached;
        if (m_d->detached) {
            m_d->ignoreHideEvents = true;
            parentWidget()->setWindowFlags(Qt::Tool);
            parentWidget()->show();
            m_d->ignoreHideEvents = false;
        }
        else {
            parentWidget()->setWindowFlags(Qt::Popup);
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
}

void KisPaintOpPresetsPopup::setPaintOpList(const QList< KisPaintOpFactory* >& list)
{
   m_d->uiWdgPaintOpPresetSettings.paintopList->setPaintOpList(list);
}

void KisPaintOpPresetsPopup::setCurrentPaintOp(const QString& paintOpId)
{
    m_d->uiWdgPaintOpPresetSettings.paintopList->setCurrent(paintOpId);
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
    if(m_d->ignoreHideEvents) {
        return;
    }
    if (m_d->detached) {
        switchDetached();
    }
    QWidget::hideEvent(event);
}

void KisPaintOpPresetsPopup::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    emit sizeChanged();
}


#include "kis_paintop_presets_popup.moc"
