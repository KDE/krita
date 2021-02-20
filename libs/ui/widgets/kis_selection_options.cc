/*
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_selection_options.h"

#include <QWidget>
#include <QRadioButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLayout>
#include <QButtonGroup>

#include <kis_icon.h>
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_image.h"
#include "kis_selection.h"
#include "kis_paint_device.h"
#include "canvas/kis_canvas2.h"
#include "KisViewManager.h"
#include "kis_signal_compressor.h"
#include "kis_shape_controller.h"
#include "kis_canvas2.h"
#include "KisDocument.h"
#include "kis_dummies_facade_base.h"

#include <ksharedconfig.h>
#include <kconfiggroup.h>

KisSelectionOptions::KisSelectionOptions(KisCanvas2 * /*canvas*/)
    : m_colorLabelsCompressor(900, KisSignalCompressor::FIRST_INACTIVE)
{
    m_page = new WdgSelectionOptions(this);
    Q_CHECK_PTR(m_page);

    QVBoxLayout * l = new QVBoxLayout(this);
    l->addWidget(m_page);
    l->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Preferred, QSizePolicy::Expanding));
    l->setContentsMargins(0,0,0,0);

    m_mode = new QButtonGroup(this);
    m_mode->addButton(m_page->pixel, PIXEL_SELECTION);
    m_mode->addButton(m_page->shape, SHAPE_PROTECTION);

    m_action = new QButtonGroup(this);
    m_action->addButton(m_page->add, SELECTION_ADD);
    m_action->addButton(m_page->subtract, SELECTION_SUBTRACT);
    m_action->addButton(m_page->replace, SELECTION_REPLACE);
    m_action->addButton(m_page->intersect, SELECTION_INTERSECT);
    m_action->addButton(m_page->symmetricdifference, SELECTION_SYMMETRICDIFFERENCE);

    m_page->pixel->setGroupPosition(KoGroupButton::GroupLeft);
    m_page->shape->setGroupPosition(KoGroupButton::GroupRight);
    m_page->pixel->setIcon(KisIconUtils::loadIcon("select_pixel"));
    m_page->shape->setIcon(KisIconUtils::loadIcon("select_shape"));

    m_page->add->setGroupPosition(KoGroupButton::GroupCenter);
    m_page->subtract->setGroupPosition(KoGroupButton::GroupCenter);
    m_page->replace->setGroupPosition(KoGroupButton::GroupLeft);
    m_page->intersect->setGroupPosition(KoGroupButton::GroupCenter);
    m_page->symmetricdifference->setGroupPosition(KoGroupButton::GroupRight);
    m_page->add->setIcon(KisIconUtils::loadIcon("selection_add"));
    m_page->subtract->setIcon(KisIconUtils::loadIcon("selection_subtract"));
    m_page->replace->setIcon(KisIconUtils::loadIcon("selection_replace"));
    m_page->intersect->setIcon(KisIconUtils::loadIcon("selection_intersect"));
    m_page->symmetricdifference->setIcon(KisIconUtils::loadIcon("selection_symmetric_difference"));

    m_page->cmbSampleLayersMode->addItem(sampleLayerModeToUserString(SAMPLE_LAYERS_MODE_CURRENT), SAMPLE_LAYERS_MODE_CURRENT);
    m_page->cmbSampleLayersMode->addItem(sampleLayerModeToUserString(SAMPLE_LAYERS_MODE_ALL), SAMPLE_LAYERS_MODE_ALL);
    m_page->cmbSampleLayersMode->addItem(sampleLayerModeToUserString(SAMPLE_LAYERS_MODE_COLOR_LABELED), SAMPLE_LAYERS_MODE_COLOR_LABELED);
    m_page->cmbSampleLayersMode->setEditable(false);

    m_page->cmbColorLabels->setModes(false, false);

    connect(m_mode, SIGNAL(buttonClicked(int)), this, SIGNAL(modeChanged(int)));
    connect(m_action, SIGNAL(buttonClicked(int)), this, SIGNAL(actionChanged(int)));
    connect(m_mode, SIGNAL(buttonClicked(int)), this, SLOT(hideActionsForSelectionMode(int)));
    connect(m_page->chkAntiAliasing, SIGNAL(toggled(bool)), this, SIGNAL(antiAliasSelectionChanged(bool)));
    connect(m_page->cmbColorLabels, SIGNAL(selectedColorsChanged()), this, SIGNAL(selectedColorLabelsChanged()));
    connect(m_page->cmbSampleLayersMode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSampleLayersModeChanged(int)));

    KConfigGroup cfg = KSharedConfig::openConfig()->group("KisToolSelectBase");
    m_page->chkAntiAliasing->setChecked(cfg.readEntry("antiAliasSelection", true));

    connect(&m_colorLabelsCompressor, SIGNAL(timeout()), this, SLOT(slotUpdateAvailableColorLabels()));
}

KisSelectionOptions::~KisSelectionOptions()
{
}

int KisSelectionOptions::action()
{
    return m_action->checkedId();
}

void KisSelectionOptions::setAction(int action) {
    QAbstractButton* button = m_action->button(action);
    KIS_SAFE_ASSERT_RECOVER_RETURN(button);

    button->setChecked(true);
}

void KisSelectionOptions::setMode(int mode) {
    QAbstractButton* button = m_mode->button(mode);
    KIS_SAFE_ASSERT_RECOVER_RETURN(button);

    button->setChecked(true);
    hideActionsForSelectionMode(mode);
}

void KisSelectionOptions::setAntiAliasSelection(bool value)
{
    m_page->chkAntiAliasing->setChecked(value);
}

void KisSelectionOptions::setSampleLayersMode(QString mode)
{
    if (mode != SAMPLE_LAYERS_MODE_ALL && mode != SAMPLE_LAYERS_MODE_COLOR_LABELED && mode != SAMPLE_LAYERS_MODE_CURRENT) {
        mode = SAMPLE_LAYERS_MODE_CURRENT;
    }
    setCmbSampleLayersMode(mode);
}

void KisSelectionOptions::enablePixelOnlySelectionMode()
{
    setMode(PIXEL_SELECTION);
    disableSelectionModeOption();
}

void KisSelectionOptions::setColorLabelsEnabled(bool enabled)
{
    if (enabled) {
        m_page->cmbColorLabels->show();
        m_page->cmbSampleLayersMode->show();
    } else {
        m_page->cmbColorLabels->hide();
        m_page->cmbSampleLayersMode->hide();
    }
}

void KisSelectionOptions::updateActionButtonToolTip(int action, const QKeySequence &shortcut)
{
    const QString shortcutString = shortcut.toString(QKeySequence::NativeText);
    QString toolTipText;
    switch ((SelectionAction)action) {
    case SELECTION_DEFAULT:
    case SELECTION_REPLACE:
        toolTipText = shortcutString.isEmpty() ?
            i18nc("@info:tooltip", "Replace") :
            i18nc("@info:tooltip", "Replace (%1)", shortcutString);

        m_action->button(SELECTION_REPLACE)->setToolTip(toolTipText);
        break;
    case SELECTION_ADD:
        toolTipText = shortcutString.isEmpty() ?
            i18nc("@info:tooltip", "Add") :
            i18nc("@info:tooltip", "Add (%1)", shortcutString);

        m_action->button(SELECTION_ADD)->setToolTip(toolTipText);
        break;
    case SELECTION_SUBTRACT:
        toolTipText = shortcutString.isEmpty() ?
            i18nc("@info:tooltip", "Subtract") :
            i18nc("@info:tooltip", "Subtract (%1)", shortcutString);

        m_action->button(SELECTION_SUBTRACT)->setToolTip(toolTipText);

        break;
    case SELECTION_INTERSECT:
        toolTipText = shortcutString.isEmpty() ?
            i18nc("@info:tooltip", "Intersect") :
            i18nc("@info:tooltip", "Intersect (%1)", shortcutString);

        m_action->button(SELECTION_INTERSECT)->setToolTip(toolTipText);

        break;
        
    case SELECTION_SYMMETRICDIFFERENCE:
        toolTipText = shortcutString.isEmpty() ?
            i18nc("@info:tooltip", "Symmetric Difference") :
            i18nc("@info:tooltip", "Symmetric Difference (%1)", shortcutString);

        m_action->button(SELECTION_SYMMETRICDIFFERENCE)->setToolTip(toolTipText);

        break;
    }
}

void KisSelectionOptions::attachToImage(KisImageSP image, KisCanvas2* canvas)
{
    m_image = image;
    m_canvas = canvas;
    activateConnectionToImage();
}

void KisSelectionOptions::activateConnectionToImage()
{
    if (m_image && m_canvas) {
        m_page->cmbColorLabels->updateAvailableLabels(m_image->root());
        KIS_SAFE_ASSERT_RECOVER_RETURN(m_canvas);
        KisDocument *doc = m_canvas->imageView()->document();

        KisShapeController *kritaShapeController =
                dynamic_cast<KisShapeController*>(doc->shapeController());
        KisDummiesFacadeBase* m_dummiesFacade = static_cast<KisDummiesFacadeBase*>(kritaShapeController);
        if (m_dummiesFacade) {
            m_nodesUpdatesConnectionsStore.addConnection(m_dummiesFacade, SIGNAL(sigEndInsertDummy(KisNodeDummy*)),
                                                         &m_colorLabelsCompressor, SLOT(start()));
            m_nodesUpdatesConnectionsStore.addConnection(m_dummiesFacade, SIGNAL(sigEndRemoveDummy()),
                                                         &m_colorLabelsCompressor, SLOT(start()));
            m_nodesUpdatesConnectionsStore.addConnection(m_dummiesFacade, SIGNAL(sigDummyChanged(KisNodeDummy*)),
                                                         &m_colorLabelsCompressor, SLOT(start()));
        }
    }
}

void KisSelectionOptions::deactivateConnectionToImage()
{
    m_nodesUpdatesConnectionsStore.clear();
}

//hide action buttons and antialiasing, if shape selection is active (actions currently don't work on shape selection)
void KisSelectionOptions::hideActionsForSelectionMode(int mode) {
    const bool isPixelSelection = (mode == (int)PIXEL_SELECTION);

    m_page->chkAntiAliasing->setVisible(isPixelSelection);
}

void KisSelectionOptions::slotUpdateAvailableColorLabels()
{
    if (m_image) {
        m_page->cmbColorLabels->updateAvailableLabels(m_image->root());
    }
}

void KisSelectionOptions::slotSampleLayersModeChanged(int index)
{
    QString newSampleLayersMode = m_page->cmbSampleLayersMode->itemData(index).toString();
    m_page->cmbColorLabels->setEnabled(newSampleLayersMode == SAMPLE_LAYERS_MODE_COLOR_LABELED);
    emit sampleLayersModeChanged(newSampleLayersMode);
}

QString KisSelectionOptions::sampleLayerModeToUserString(QString sampleLayersModeId)
{
    QString currentLayer = i18nc("Option in selection tool: take only the current layer into account when calculating the selection", "Current Layer");
    if (sampleLayersModeId == SAMPLE_LAYERS_MODE_CURRENT) {
        return currentLayer;
    } else if (sampleLayersModeId == SAMPLE_LAYERS_MODE_ALL) {
        return i18nc("Option in selection tool: take all layers (merged) into account when calculating the selection", "All Layers");
    } else if (sampleLayersModeId == SAMPLE_LAYERS_MODE_COLOR_LABELED) {
        return i18nc("Option in selection tool: take all layers that were marked with specific color labels (more precisely, all of them merged) "
                     "into account when calculating the selection", "Color Labeled Layers");
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(false, currentLayer);
    return currentLayer;
}

void KisSelectionOptions::setCmbSampleLayersMode(QString sampleLayersModeId)
{
    for (int i = 0; i < m_page->cmbSampleLayersMode->count(); i++) {
        if (m_page->cmbSampleLayersMode->itemData(i).toString() == sampleLayersModeId)
        {
            m_page->cmbSampleLayersMode->setCurrentIndex(i);
            break;
        }
    }
    m_page->cmbColorLabels->setEnabled(sampleLayersModeId == SAMPLE_LAYERS_MODE_COLOR_LABELED);
}

bool KisSelectionOptions::antiAliasSelection()
{
    return m_page->chkAntiAliasing->isChecked();
}

QList<int> KisSelectionOptions::colorLabelsSelected()
{
    return m_page->cmbColorLabels->selectedColors();
}

QString KisSelectionOptions::sampleLayersMode()
{
    return m_page->cmbSampleLayersMode->currentData().toString();
}

void KisSelectionOptions::disableAntiAliasSelectionOption()
{
    m_page->chkAntiAliasing->hide();
    disconnect(m_page->pixel, SIGNAL(clicked()), m_page->chkAntiAliasing, SLOT(show()));
}

void KisSelectionOptions::disableSelectionModeOption()
{
    m_page->lblMode->hide();
    m_page->pixel->hide();
    m_page->shape->hide();
}

