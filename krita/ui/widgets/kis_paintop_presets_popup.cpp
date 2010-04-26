/* This file is part of the KDE project
 * Copyright (C) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include <kconfig.h>
#include <kglobalsettings.h>
#include <kicon.h>
#include <klocale.h>

#include <KoColorSpaceRegistry.h>

#include <kis_paintop_preset.h>
#include <kis_canvas_resource_provider.h>
#include <widgets/kis_preset_chooser.h>

#include <ui_wdgpaintoppresets.h>
#include <kis_node.h>
#include "kis_config.h"

class KisPaintOpPresetsPopup::Private
{

public:

    Ui_WdgPaintOpPresets uiWdgPaintOpPresets;
    QGridLayout *layout;
    QWidget *settingsWidget;
    QFont smallFont;
    KisCanvasResourceProvider *resourceProvider;
    bool detached;
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

    m_d->uiWdgPaintOpPresets.setupUi(this);

    m_d->layout = new QGridLayout(m_d->uiWdgPaintOpPresets.frmOptionWidgetContainer);
    m_d->layout->setSizeConstraint(QLayout::SetFixedSize);

    m_d->uiWdgPaintOpPresets.scratchPad->setCanvasColor(Qt::white);
    m_d->uiWdgPaintOpPresets.scratchPad->setColorSpace(KoColorSpaceRegistry::instance()->rgb8());
    m_d->uiWdgPaintOpPresets.scratchPad->setCutoutOverlay(QRect(0, 0, 250, 60));
    m_d->uiWdgPaintOpPresets.fillLayer->setIcon(KIcon("newlayer"));
    m_d->uiWdgPaintOpPresets.fillGradient->setIcon(KIcon("krita_tool_gradient"));
    m_d->uiWdgPaintOpPresets.fillSolid->setIcon(KIcon("krita_tool_color_fill"));
    m_d->uiWdgPaintOpPresets.eraseScratchPad->setIcon(KIcon("list-remove"));

    connect(m_d->uiWdgPaintOpPresets.eraseScratchPad, SIGNAL(clicked()),
            m_d->uiWdgPaintOpPresets.scratchPad, SLOT(clear()));

    connect(m_d->resourceProvider, SIGNAL(sigFGColorChanged(const KoColor &)),
            m_d->uiWdgPaintOpPresets.scratchPad, SLOT(setPaintColor(const KoColor &)));

    connect(m_d->resourceProvider, SIGNAL(sigBGColorChanged(const KoColor &)),
            m_d->uiWdgPaintOpPresets.scratchPad, SLOT(setBackgroundColor(const KoColor &)));

    connect(m_d->uiWdgPaintOpPresets.fillLayer, SIGNAL(clicked()),
            this, SLOT(fillScratchPadLayer()));

    connect(m_d->uiWdgPaintOpPresets.fillGradient, SIGNAL(clicked()),
            this, SLOT(fillScratchPadGradient()));

    connect(m_d->uiWdgPaintOpPresets.fillSolid, SIGNAL(clicked()),
            this, SLOT(fillScratchPadSolid()));

    m_d->settingsWidget = 0;
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    connect(m_d->uiWdgPaintOpPresets.bnSave, SIGNAL(clicked()),
            this, SIGNAL(savePresetClicked()));
            
    connect(m_d->uiWdgPaintOpPresets.bnDefaultPreset, SIGNAL(clicked()),
            this, SIGNAL(defaultPresetClicked()));
            

    connect(m_d->uiWdgPaintOpPresets.wdgPresetChooser, SIGNAL(resourceSelected(KoResource*)),
            this, SIGNAL(resourceSelected(KoResource*)));

    connect(m_d->uiWdgPaintOpPresets.searchBar, SIGNAL(textChanged(const QString&)),
            m_d->uiWdgPaintOpPresets.wdgPresetChooser, SLOT(searchTextChanged(const QString&)));

    connect(m_d->uiWdgPaintOpPresets.showAllCheckBox, SIGNAL(toggled(bool)),
            m_d->uiWdgPaintOpPresets.wdgPresetChooser, SLOT(setShowAll(bool)));

    KisConfig cfg;
    m_d->detached = !cfg.paintopPopupDetached();

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

void KisPaintOpPresetsPopup::setPaintOpSettingsWidget(QWidget * widget)
{
    if (m_d->settingsWidget) {
        m_d->layout->removeWidget(m_d->settingsWidget);
        m_d->uiWdgPaintOpPresets.frmOptionWidgetContainer->updateGeometry();
    }
    m_d->layout->update();
    updateGeometry();

    m_d->settingsWidget = widget;

    if (widget) {

        widget->setFont(m_d->smallFont);

        m_d->settingsWidget->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
        m_d->layout->addWidget(widget);

        m_d->layout->update();
        widget->show();
    }
}

QString KisPaintOpPresetsPopup::getPresetName() const
{
    return m_d->uiWdgPaintOpPresets.txtPreset->text();
}

void KisPaintOpPresetsPopup::setPresetFilter(const KoID& paintopID)
{
    m_d->uiWdgPaintOpPresets.wdgPresetChooser->setPresetFilter(paintopID);
}

void KisPaintOpPresetsPopup::setPreset(KisPaintOpPresetSP preset)
{
    m_d->uiWdgPaintOpPresets.scratchPad->setPreset(preset);
}

QImage KisPaintOpPresetsPopup::cutOutOverlay()
{
    return m_d->uiWdgPaintOpPresets.scratchPad->cutoutOverlay();
}

void KisPaintOpPresetsPopup::fillScratchPadGradient()
{
    m_d->uiWdgPaintOpPresets.scratchPad->fillGradient(m_d->resourceProvider->currentGradient());
}

void KisPaintOpPresetsPopup::fillScratchPadSolid()
{
    m_d->uiWdgPaintOpPresets.scratchPad->fillSolid(m_d->resourceProvider->bgColor());
}

void KisPaintOpPresetsPopup::fillScratchPadLayer()
{
    //TODO
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

        qDebug() << parentWidget()->objectName() << m_d->detached;

        m_d->detached = !m_d->detached;
        if (m_d->detached) {
            parentWidget()->setWindowFlags(Qt::Tool);
            parentWidget()->show();
        }
        else {
            parentWidget()->setWindowFlags(Qt::Popup);
        }

        KisConfig cfg;
        cfg.setPaintopPopupDetached(m_d->detached);
    }
}

#include "kis_paintop_presets_popup.moc"
