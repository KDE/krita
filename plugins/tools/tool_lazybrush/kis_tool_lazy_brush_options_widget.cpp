/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_tool_lazy_brush_options_widget.h"

#include "ui_kis_tool_lazy_brush_options_widget.h"

#include <KoColorSpaceRegistry.h>
#include "KisPaletteModel.h"

#include "kis_config.h"
#include <resources/KoColorSet.h>
#include "kis_canvas_resource_provider.h"
#include "kis_signal_auto_connection.h"
#include "lazybrush/kis_colorize_mask.h"
#include "kis_image.h"
#include "kis_signals_blocker.h"
#include "kis_signal_compressor.h"
#include "kis_layer_properties_icons.h"


struct KisToolLazyBrushOptionsWidget::Private
{
    Private()
        : transparentColorIndex(-1),
          baseNodeChangedCompressor(500, KisSignalCompressor::FIRST_ACTIVE)
    {
    }

    Ui_KisToolLazyBrushOptionsWidget *ui;
    KisPaletteModel *colorModel;
    KisCanvasResourceProvider *provider;

    KisSignalAutoConnectionsStore providerSignals;
    KisSignalAutoConnectionsStore maskSignals;
    KisColorizeMaskSP activeMask;

    KoColorSet colorSet;
    int transparentColorIndex = -1;

    KisSignalCompressor baseNodeChangedCompressor;
};

KisToolLazyBrushOptionsWidget::KisToolLazyBrushOptionsWidget(KisCanvasResourceProvider *provider, QWidget *parent)
    : QWidget(parent),
      m_d(new Private)
{
    m_d->ui = new Ui_KisToolLazyBrushOptionsWidget();
    m_d->ui->setupUi(this);

    m_d->colorModel = new KisPaletteModel(this);
    m_d->ui->colorView->setPaletteModel(m_d->colorModel);
    m_d->ui->colorView->setAllowModification(false); //people proly shouldn't be able to edit the colorentries themselves.
    m_d->ui->colorView->setCrossedKeyword("transparent");

    connect(m_d->ui->colorView, SIGNAL(indexEntrySelected(QModelIndex)), this, SLOT(entrySelected(QModelIndex)));
    connect(m_d->ui->btnTransparent, SIGNAL(toggled(bool)), this, SLOT(slotMakeTransparent(bool)));
    connect(m_d->ui->btnRemove, SIGNAL(clicked()), this, SLOT(slotRemove()));

    connect(m_d->ui->chkAutoUpdates, SIGNAL(toggled(bool)), m_d->ui->btnUpdate, SLOT(setDisabled(bool)));

    connect(m_d->ui->btnUpdate, SIGNAL(clicked()), this, SLOT(slotUpdate()));
    connect(m_d->ui->chkAutoUpdates, SIGNAL(toggled(bool)), this, SLOT(slotSetAutoUpdates(bool)));
    connect(m_d->ui->chkShowKeyStrokes, SIGNAL(toggled(bool)), this, SLOT(slotSetShowKeyStrokes(bool)));
    connect(m_d->ui->chkShowOutput, SIGNAL(toggled(bool)), this, SLOT(slotSetShowOutput(bool)));

    connect(&m_d->baseNodeChangedCompressor, SIGNAL(timeout()), this, SLOT(slotUpdateNodeProperties()));


    m_d->provider = provider;

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();

    m_d->colorSet.add(KoColorSetEntry(KoColor(Qt::red, cs), "color1"));
    m_d->colorSet.add(KoColorSetEntry(KoColor(Qt::green, cs), "color2"));
    m_d->colorSet.add(KoColorSetEntry(KoColor(Qt::blue, cs), "color3"));

    m_d->colorModel->setColorSet(&m_d->colorSet);
}

KisToolLazyBrushOptionsWidget::~KisToolLazyBrushOptionsWidget()
{
}

void KisToolLazyBrushOptionsWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    m_d->providerSignals.addConnection(
        m_d->provider, SIGNAL(sigNodeChanged(KisNodeSP)),
        this, SLOT(slotCurrentNodeChanged(KisNodeSP)));

    m_d->providerSignals.addConnection(
        m_d->provider, SIGNAL(sigFGColorChanged(const KoColor&)),
        this, SLOT(slotCurrentFgColorChanged(const KoColor&)));

    slotCurrentNodeChanged(m_d->provider->currentNode());
    slotCurrentFgColorChanged(m_d->provider->fgColor());
}

void KisToolLazyBrushOptionsWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);

    m_d->providerSignals.clear();
}

void KisToolLazyBrushOptionsWidget::entrySelected(QModelIndex index)
{
    qDebug()<<"triggered";
    if (!index.isValid()) return;

    qDebug()<<index;
    const int i = m_d->colorModel->idFromIndex(index);
    qDebug()<<i;

    if (i >= 0 && i < (int)m_d->colorSet.nColors()) {
        KoColorSetEntry entry = m_d->colorModel->colorSetEntryFromIndex(index);
        m_d->provider->setFGColor(entry.color);
    }

    const bool transparentChecked = i >= 0 && i == m_d->transparentColorIndex;
    KisSignalsBlocker b(m_d->ui->btnTransparent);
    m_d->ui->btnTransparent->setChecked(transparentChecked);
}

void KisToolLazyBrushOptionsWidget::slotCurrentFgColorChanged(const KoColor &color)
{
    int selectedIndex = -1;

    for (quint32 i = 0; i < m_d->colorSet.nColors(); i++) {
        KoColorSetEntry entry = m_d->colorSet.getColorGlobal(i);
        if (entry.color == color) {
            selectedIndex = (int)i;
            break;
        }
    }

    m_d->ui->btnRemove->setEnabled(selectedIndex >= 0);
    m_d->ui->btnTransparent->setEnabled(selectedIndex >= 0);

    if (selectedIndex < 0) {
        KisSignalsBlocker b(m_d->ui->btnTransparent);
        m_d->ui->btnTransparent->setChecked(false);
    }

    QModelIndex newIndex = m_d->colorModel->indexFromId(selectedIndex);

    if (newIndex != m_d->ui->colorView->currentIndex()) {
        m_d->ui->colorView->setCurrentIndex(newIndex);
    }
}

void KisToolLazyBrushOptionsWidget::slotColorLabelsChanged()
{
    m_d->colorSet.clear();
    m_d->transparentColorIndex = -1;

    if (m_d->activeMask) {
        KisColorizeMask::KeyStrokeColors colors = m_d->activeMask->keyStrokesColors();
        m_d->transparentColorIndex = colors.transparentIndex;

        for (int i = 0; i < colors.colors.size(); i++) {
            const QString name = i == m_d->transparentColorIndex ? "transparent" : "";
            m_d->colorSet.add(KoColorSetEntry(colors.colors[i], name));
        }
    }

    m_d->colorModel->setColorSet(&m_d->colorSet);
    slotCurrentFgColorChanged(m_d->provider->fgColor());
}

void KisToolLazyBrushOptionsWidget::slotUpdateNodeProperties()
{
    KisSignalsBlocker b(m_d->ui->chkAutoUpdates,
                        m_d->ui->btnUpdate,
                        m_d->ui->chkShowKeyStrokes,
                        m_d->ui->chkShowOutput);

    // not implemented yet!
    //m_d->ui->chkAutoUpdates->setEnabled(m_d->activeMask);
    m_d->ui->chkAutoUpdates->setEnabled(false);

    bool value = false;

    value = m_d->activeMask && !KisLayerPropertiesIcons::nodeProperty(m_d->activeMask, KisLayerPropertiesIcons::colorizeNeedsUpdate, true).toBool();
    m_d->ui->btnUpdate->setEnabled(m_d->activeMask && !m_d->ui->chkAutoUpdates->isChecked());
    m_d->ui->btnUpdate->setChecked(value);

    value = m_d->activeMask && KisLayerPropertiesIcons::nodeProperty(m_d->activeMask, KisLayerPropertiesIcons::colorizeEditKeyStrokes, true).toBool();
    m_d->ui->chkShowKeyStrokes->setEnabled(m_d->activeMask);
    m_d->ui->chkShowKeyStrokes->setChecked(value);

    value = m_d->activeMask && KisLayerPropertiesIcons::nodeProperty(m_d->activeMask, KisLayerPropertiesIcons::colorizeShowColoring, true).toBool();
    m_d->ui->chkShowOutput->setEnabled(m_d->activeMask);
    m_d->ui->chkShowOutput->setChecked(value);
}

void KisToolLazyBrushOptionsWidget::slotCurrentNodeChanged(KisNodeSP node)
{
    m_d->maskSignals.clear();

    KisColorizeMask *mask = dynamic_cast<KisColorizeMask*>(node.data());
    m_d->activeMask = mask;

    if (m_d->activeMask) {
        m_d->maskSignals.addConnection(
            m_d->activeMask, SIGNAL(sigKeyStrokesListChanged()),
            this, SLOT(slotColorLabelsChanged()));

        m_d->maskSignals.addConnection(
            m_d->provider->currentImage(), SIGNAL(sigNodeChanged(KisNodeSP)),
            this, SLOT(slotUpdateNodeProperties()));
    }

    slotColorLabelsChanged();
    slotUpdateNodeProperties();
    m_d->ui->colorView->setEnabled(m_d->activeMask);
}

void KisToolLazyBrushOptionsWidget::slotMakeTransparent(bool value)
{
    KIS_ASSERT_RECOVER_RETURN(m_d->activeMask);

    QModelIndex index = m_d->ui->colorView->currentIndex();
    if (!index.isValid()) return;

    const int activeIndex = m_d->colorModel->idFromIndex(index);
    KIS_ASSERT_RECOVER_RETURN(activeIndex >= 0);

    KisColorizeMask::KeyStrokeColors colors;

    for (quint32 i = 0; i < m_d->colorSet.nColors(); i++) {
        colors.colors << m_d->colorSet.getColorGlobal(i).color;
    }

    colors.transparentIndex = value ? activeIndex : -1;

    m_d->activeMask->setKeyStrokesColors(colors);
}

void KisToolLazyBrushOptionsWidget::slotRemove()
{
    KIS_ASSERT_RECOVER_RETURN(m_d->activeMask);

    QModelIndex index = m_d->ui->colorView->currentIndex();
    if (!index.isValid()) return;

    const int activeIndex = m_d->colorModel->idFromIndex(index);
    KIS_ASSERT_RECOVER_RETURN(activeIndex >= 0);


    const KoColor color = m_d->colorSet.getColorGlobal((quint32)activeIndex).color;
    m_d->activeMask->removeKeyStroke(color);
}

void KisToolLazyBrushOptionsWidget::slotUpdate()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->activeMask);
    KisLayerPropertiesIcons::setNodeProperty(m_d->activeMask, KisLayerPropertiesIcons::colorizeNeedsUpdate, false, m_d->provider->currentImage());
}

void KisToolLazyBrushOptionsWidget::slotSetAutoUpdates(bool value)
{
    ENTER_FUNCTION() << ppVar(value);
}

void KisToolLazyBrushOptionsWidget::slotSetShowKeyStrokes(bool value)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->activeMask);
    KisLayerPropertiesIcons::setNodeProperty(m_d->activeMask, KisLayerPropertiesIcons::colorizeEditKeyStrokes, value, m_d->provider->currentImage());
}

void KisToolLazyBrushOptionsWidget::slotSetShowOutput(bool value)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->activeMask);
    KisLayerPropertiesIcons::setNodeProperty(m_d->activeMask, KisLayerPropertiesIcons::colorizeShowColoring, value, m_d->provider->currentImage());
}

