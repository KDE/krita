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
        : baseNodeChangedCompressor(500, KisSignalCompressor::FIRST_ACTIVE)
    {
    }

    Ui_KisToolLazyBrushOptionsWidget *ui;
    KisPaletteModel *colorModel;
    KisCanvasResourceProvider *provider;

    KisSignalAutoConnectionsStore providerSignals;
    KisSignalAutoConnectionsStore maskSignals;
    KisColorizeMaskSP activeMask;

    KoColorSetSP colorSet {new KoColorSet(QString())};
    int transparentColorIndex;

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

    connect(m_d->ui->chkUseEdgeDetection, SIGNAL(toggled(bool)), SLOT(slotUseEdgeDetectionChanged(bool)));
    connect(m_d->ui->intEdgeDetectionSize, SIGNAL(valueChanged(int)), SLOT(slotEdgeDetectionSizeChanged(int)));
    connect(m_d->ui->intRadius, SIGNAL(valueChanged(int)), SLOT(slotRadiusChanged(int)));
    connect(m_d->ui->intCleanUp, SIGNAL(valueChanged(int)), SLOT(slotCleanUpChanged(int)));
    connect(m_d->ui->chkLimitToDevice, SIGNAL(toggled(bool)), SLOT(slotLimitToDeviceChanged(bool)));

    m_d->ui->intEdgeDetectionSize->setRange(0, 100);
    m_d->ui->intEdgeDetectionSize->setExponentRatio(2.0);
    m_d->ui->intEdgeDetectionSize->setSuffix(i18n(" px"));
    m_d->ui->intEdgeDetectionSize->setPrefix(i18n("Edge detection: "));
    m_d->ui->intEdgeDetectionSize->setToolTip(
        i18nc("@info:tooltip",
              "Activate for images with vast solid areas. "
              "Set the value to the width of the thinnest "
              "lines on the image"));

    m_d->ui->intRadius->setRange(0, 1000);
    m_d->ui->intRadius->setExponentRatio(3.0);
    m_d->ui->intRadius->setSuffix(i18n(" px"));
    m_d->ui->intRadius->setPrefix(i18n("Gap close hint: "));
    m_d->ui->intRadius->setToolTip(
        i18nc("@info:tooltip",
              "The mask will try to close non-closed contours "
              "if the gap is smaller than \"Gap close hint\" value"));

    m_d->ui->intCleanUp->setRange(0, 100);
    m_d->ui->intCleanUp->setSuffix(i18n(" %"));
    m_d->ui->intCleanUp->setPrefix(i18n("Clean up: "));
    m_d->ui->intCleanUp->setToolTip(
        i18nc("@info:tooltip",
              "The mask will try to remove parts of the key strokes "
              "that are placed outside the closed contours. 0% - no effect, 100% - max effect"));


    connect(m_d->ui->colorView, SIGNAL(sigIndexSelected(QModelIndex)), this, SLOT(entrySelected(QModelIndex)));
    connect(m_d->ui->btnTransparent, SIGNAL(toggled(bool)), this, SLOT(slotMakeTransparent(bool)));
    connect(m_d->ui->btnRemove, SIGNAL(clicked()), this, SLOT(slotRemove()));

    connect(m_d->ui->chkAutoUpdates, SIGNAL(toggled(bool)), m_d->ui->btnUpdate, SLOT(setDisabled(bool)));

    connect(m_d->ui->btnUpdate, SIGNAL(clicked()), this, SLOT(slotUpdate()));
    connect(m_d->ui->chkAutoUpdates, SIGNAL(toggled(bool)), this, SLOT(slotSetAutoUpdates(bool)));
    connect(m_d->ui->chkShowKeyStrokes, SIGNAL(toggled(bool)), this, SLOT(slotSetShowKeyStrokes(bool)));
    connect(m_d->ui->chkShowOutput, SIGNAL(toggled(bool)), this, SLOT(slotSetShowOutput(bool)));

    connect(&m_d->baseNodeChangedCompressor, SIGNAL(timeout()), this, SLOT(slotUpdateNodeProperties()));

    m_d->provider = provider;

    m_d->colorSet->setIsGlobal(false);
    m_d->colorSet->setIsEditable(true);
    m_d->colorModel->setPalette(m_d->colorSet);

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();

    m_d->colorModel->addEntry(KisSwatch(KoColor(Qt::red, cs), "color1"));
    m_d->colorModel->addEntry(KisSwatch(KoColor(Qt::green, cs), "color2"));
    m_d->colorModel->addEntry(KisSwatch(KoColor(Qt::blue, cs), "color3"));
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
        m_d->provider, SIGNAL(sigFGColorChanged(KoColor)),
        this, SLOT(slotCurrentFgColorChanged(KoColor)));

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
    if (!index.isValid()) return;
    if (!qvariant_cast<bool>(index.data(KisPaletteModel::CheckSlotRole))) return;

    KisSwatch entry = m_d->colorModel->getEntry(index);
    m_d->provider->setFGColor(entry.color());

    int idxInList = m_d->activeMask->keyStrokesColors().colors.indexOf(entry.color());

    if (idxInList != -1) {
        const bool transparentChecked = idxInList == m_d->transparentColorIndex;
        KisSignalsBlocker b(m_d->ui->btnTransparent);
        m_d->ui->btnTransparent->setChecked(transparentChecked);
    }
}

void KisToolLazyBrushOptionsWidget::slotCurrentFgColorChanged(const KoColor &color)
{
    bool found = false;

    QModelIndex candidateIdx = m_d->colorModel->indexForClosest(color);
    if (m_d->colorModel->getEntry(candidateIdx).color() == color) {
        found = true;
    }

    m_d->ui->btnRemove->setEnabled(found);
    m_d->ui->btnTransparent->setEnabled(found);

    if (!found) {
        KisSignalsBlocker b(m_d->ui->btnTransparent);
        m_d->ui->btnTransparent->setChecked(false);
    }

    QModelIndex newIndex = found ? candidateIdx : QModelIndex();

    if (!found) {
        m_d->ui->colorView->selectionModel()->clear();
    }
    if (newIndex.isValid() && newIndex != m_d->ui->colorView->currentIndex()) {
        m_d->ui->colorView->setCurrentIndex(newIndex);
        m_d->ui->colorView->update(newIndex);
    }
}

void KisToolLazyBrushOptionsWidget::slotColorLabelsChanged()
{
    m_d->colorModel->clear();

    m_d->transparentColorIndex = -1;

    if (m_d->activeMask) {
        KisColorizeMask::KeyStrokeColors colors = m_d->activeMask->keyStrokesColors();
        m_d->transparentColorIndex = colors.transparentIndex;

        for (int i = 0; i < colors.colors.size(); i++) {
            const QString name = i == m_d->transparentColorIndex ? "transparent" : "";
            m_d->colorModel->addEntry(KisSwatch(colors.colors[i], name));
        }
    }

    slotCurrentFgColorChanged(m_d->provider->fgColor());
}

void KisToolLazyBrushOptionsWidget::slotUpdateNodeProperties()
{
    KisSignalsBlocker b1(m_d->ui->chkAutoUpdates,
                         m_d->ui->btnUpdate,
                         m_d->ui->chkShowKeyStrokes,
                         m_d->ui->chkShowOutput);
    KisSignalsBlocker b2(m_d->ui->chkUseEdgeDetection,
                         m_d->ui->intEdgeDetectionSize,
                         m_d->ui->intRadius,
                         m_d->ui->intCleanUp,
                         m_d->ui->chkLimitToDevice);

    // not implemented yet!
    //m_d->ui->chkAutoUpdates->setEnabled(m_d->activeMask);
    m_d->ui->chkAutoUpdates->setEnabled(false);
    m_d->ui->chkAutoUpdates->setVisible(false);

    bool value = false;

    value = m_d->activeMask && KisLayerPropertiesIcons::nodeProperty(m_d->activeMask, KisLayerPropertiesIcons::colorizeNeedsUpdate, true).toBool();
    m_d->ui->btnUpdate->setEnabled(m_d->activeMask && !m_d->ui->chkAutoUpdates->isChecked() && value);

    value = m_d->activeMask && KisLayerPropertiesIcons::nodeProperty(m_d->activeMask, KisLayerPropertiesIcons::colorizeEditKeyStrokes, true).toBool();
    m_d->ui->chkShowKeyStrokes->setEnabled(m_d->activeMask);
    m_d->ui->chkShowKeyStrokes->setChecked(value);

    value = m_d->activeMask && KisLayerPropertiesIcons::nodeProperty(m_d->activeMask, KisLayerPropertiesIcons::colorizeShowColoring, true).toBool();
    m_d->ui->chkShowOutput->setEnabled(m_d->activeMask);
    m_d->ui->chkShowOutput->setChecked(value);

    m_d->ui->chkUseEdgeDetection->setEnabled(m_d->activeMask);
    m_d->ui->chkUseEdgeDetection->setChecked(m_d->activeMask && m_d->activeMask->useEdgeDetection());

    m_d->ui->intEdgeDetectionSize->setEnabled(m_d->activeMask && m_d->ui->chkUseEdgeDetection->isChecked());
    m_d->ui->intEdgeDetectionSize->setValue(m_d->activeMask ? m_d->activeMask->edgeDetectionSize() : 4.0);
    m_d->ui->intRadius->setEnabled(m_d->activeMask);
    m_d->ui->intRadius->setValue(2 * (m_d->activeMask ? m_d->activeMask->fuzzyRadius() : 15));
    m_d->ui->intCleanUp->setEnabled(m_d->activeMask);
    m_d->ui->intCleanUp->setValue(100 * (m_d->activeMask ? m_d->activeMask->cleanUpAmount() : 0.7));

    m_d->ui->chkLimitToDevice->setEnabled(m_d->activeMask);
    m_d->ui->chkLimitToDevice->setChecked(m_d->activeMask && m_d->activeMask->limitToDeviceBounds());
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
    KisSwatch activeSwatch = m_d->colorModel->getEntry(index);
    if (!index.isValid()) return;

    int activeIndex = -1;

    KisColorizeMask::KeyStrokeColors colors;

    int i = 0;
    Q_FOREACH (const QString &groupName, m_d->colorSet->getGroupNames()) {
        KisSwatchGroup *group = m_d->colorSet->getGroup(groupName);
        Q_FOREACH (const KisSwatchGroup::SwatchInfo &info, group->infoList()) {
            colors.colors << info.swatch.color();
            if (activeSwatch == info.swatch) { activeIndex = i; }
            i++;
        }
    }

    colors.transparentIndex = value ? activeIndex : -1;

    m_d->activeMask->setKeyStrokesColors(colors);
}

void KisToolLazyBrushOptionsWidget::slotRemove()
{
    KIS_ASSERT_RECOVER_RETURN(m_d->activeMask);

    QModelIndex index = m_d->ui->colorView->currentIndex();
    if (!index.isValid()) return;

    const KoColor color = m_d->colorModel->getEntry(index).color();
    m_d->activeMask->removeKeyStroke(color);
}

void KisToolLazyBrushOptionsWidget::slotUpdate()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->activeMask);
    KisLayerPropertiesIcons::setNodeProperty(m_d->activeMask, KisLayerPropertiesIcons::colorizeNeedsUpdate, false, m_d->provider->currentImage());
}

void KisToolLazyBrushOptionsWidget::slotSetAutoUpdates(bool value)
{
    // not implemented yet!
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

void KisToolLazyBrushOptionsWidget::slotUseEdgeDetectionChanged(bool value)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->activeMask);
    m_d->activeMask->setUseEdgeDetection(value);
    m_d->ui->intEdgeDetectionSize->setEnabled(value);
}

void KisToolLazyBrushOptionsWidget::slotEdgeDetectionSizeChanged(int value)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->activeMask);
    m_d->activeMask->setEdgeDetectionSize(value);
}

void KisToolLazyBrushOptionsWidget::slotRadiusChanged(int value)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->activeMask);
    m_d->activeMask->setFuzzyRadius(0.5 * value);
}

void KisToolLazyBrushOptionsWidget::slotCleanUpChanged(int value)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->activeMask);
    m_d->activeMask->setCleanUpAmount(qreal(value) / 100.0);
}

void KisToolLazyBrushOptionsWidget::slotLimitToDeviceChanged(bool value)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->activeMask);
    m_d->activeMask->setLimitToDeviceBounds(value);
}

