/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_selection_tool_config_widget_helper.h"

#include <QKeyEvent>
#include "kis_selection_options.h"
#include "kis_canvas2.h"
#include "KisViewManager.h"
#include "kis_canvas_resource_provider.h"
#include "kis_signals_blocker.h"

#include <KConfigGroup>
#include <KSharedConfig>

KisSelectionToolConfigWidgetHelper::KisSelectionToolConfigWidgetHelper(const QString &windowTitle)
    : m_optionsWidget(0),
      m_windowTitle(windowTitle)
{
}

void KisSelectionToolConfigWidgetHelper::createOptionWidget(KisCanvas2 *canvas, const QString &toolId)
{
    m_optionsWidget = new KisSelectionOptions(canvas);
    Q_CHECK_PTR(m_optionsWidget);

    m_optionsWidget->setObjectName(toolId + "option widget");
    m_optionsWidget->setWindowTitle(m_windowTitle);
    slotToolActivatedChanged(true);

    // See https://bugs.kde.org/show_bug.cgi?id=316896
    QWidget *specialSpacer = new QWidget(m_optionsWidget);
    specialSpacer->setObjectName("SpecialSpacer");
    specialSpacer->setFixedSize(0, 0);
    m_optionsWidget->layout()->addWidget(specialSpacer);

    connect(m_optionsWidget, &KisSelectionOptions::actionChanged,
            this, &KisSelectionToolConfigWidgetHelper::slotWidgetActionChanged);

    connect(m_optionsWidget, &KisSelectionOptions::modeChanged,
            this, &KisSelectionToolConfigWidgetHelper::slotWidgetModeChanged);

    connect(m_optionsWidget, &KisSelectionOptions::antiAliasSelectionChanged,
            this, &KisSelectionToolConfigWidgetHelper::slotWidgetAntiAliasChanged);

    connect(m_optionsWidget, &KisSelectionOptions::selectedColorLabelsChanged,
            this, &KisSelectionToolConfigWidgetHelper::slotSelectedColorLabelsChanged);

    connect(m_optionsWidget, &KisSelectionOptions::sampleLayersModeChanged,
            this, &KisSelectionToolConfigWidgetHelper::slotSampleLayersModeChanged);


    m_optionsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_optionsWidget->adjustSize();

    m_sampleLayersMode = m_optionsWidget->sampleLayersMode();
}

KisSelectionOptions* KisSelectionToolConfigWidgetHelper::optionWidget() const
{
    return m_optionsWidget;
}

SelectionMode KisSelectionToolConfigWidgetHelper::selectionMode() const
{
    return m_selectionMode;
}

SelectionAction KisSelectionToolConfigWidgetHelper::selectionAction() const
{
    return m_selectionAction;
}

bool KisSelectionToolConfigWidgetHelper::antiAliasSelection() const
{
    return m_antiAliasSelection;
}

QList<int> KisSelectionToolConfigWidgetHelper::colorLabelsSelected() const
{
    return m_colorLabelsSelected;
}

QString KisSelectionToolConfigWidgetHelper::sampleLayersMode() const
{
    return m_sampleLayersMode;
}

void KisSelectionToolConfigWidgetHelper::setConfigGroupForExactTool(QString toolId)
{
    m_configGroupForTool = toolId;
    if (m_configGroupForTool != "") {
        KConfigGroup cfgToolSpecific = KSharedConfig::openConfig()->group(m_configGroupForTool);
        QString newSampleMode = cfgToolSpecific.readEntry("sampleLayersMode", m_optionsWidget->SAMPLE_LAYERS_MODE_CURRENT);
        if (newSampleMode != m_sampleLayersMode) {
            m_optionsWidget->setSampleLayersMode(newSampleMode);
        }
        m_sampleLayersMode = newSampleMode;
    }
}

void KisSelectionToolConfigWidgetHelper::slotWidgetActionChanged(int action)
{
    if (action >= SELECTION_REPLACE && action <= SELECTION_SYMMETRICDIFFERENCE) {
        m_selectionAction = (SelectionAction)action;

        KConfigGroup cfg = KSharedConfig::openConfig()->group("KisToolSelectBase");
        cfg.writeEntry("selectionAction", action);

        emit selectionActionChanged(action);
    }
}

void KisSelectionToolConfigWidgetHelper::slotWidgetModeChanged(int mode)
{
    m_selectionMode = (SelectionMode)mode;

    KConfigGroup cfg = KSharedConfig::openConfig()->group("KisToolSelectBase");
    cfg.writeEntry("selectionMode", mode);
}

void KisSelectionToolConfigWidgetHelper::slotWidgetAntiAliasChanged(bool value)
{
    m_antiAliasSelection = value;

    KConfigGroup cfg = KSharedConfig::openConfig()->group("KisToolSelectBase");
    cfg.writeEntry("antiAliasSelection", value);
}

void KisSelectionToolConfigWidgetHelper::slotSelectedColorLabelsChanged()
{
    m_colorLabelsSelected = m_optionsWidget->colorLabelsSelected();
}

void KisSelectionToolConfigWidgetHelper::slotSampleLayersModeChanged(QString mode)
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group(m_configGroupForTool);
    cfg.writeEntry("sampleLayersMode", mode);
    m_sampleLayersMode = mode;
}

void KisSelectionToolConfigWidgetHelper::slotReplaceModeRequested()
{
    m_optionsWidget->setAction(SELECTION_REPLACE);
    slotWidgetActionChanged(m_optionsWidget->action());
}

void KisSelectionToolConfigWidgetHelper::slotAddModeRequested()
{
    m_optionsWidget->setAction(SELECTION_ADD);
    slotWidgetActionChanged(m_optionsWidget->action());
}

void KisSelectionToolConfigWidgetHelper::slotSubtractModeRequested()
{
    m_optionsWidget->setAction(SELECTION_SUBTRACT);
    slotWidgetActionChanged(m_optionsWidget->action());
}

void KisSelectionToolConfigWidgetHelper::slotIntersectModeRequested()
{
    m_optionsWidget->setAction(SELECTION_INTERSECT);
    slotWidgetActionChanged(m_optionsWidget->action());
}

void KisSelectionToolConfigWidgetHelper::slotSymmetricDifferenceModeRequested()
{
    m_optionsWidget->setAction(SELECTION_SYMMETRICDIFFERENCE);
    slotWidgetActionChanged(m_optionsWidget->action());
}

void KisSelectionToolConfigWidgetHelper::slotToolActivatedChanged(bool isActivated)
{
    if (!isActivated) return;

    KConfigGroup cfg = KSharedConfig::openConfig()->group("KisToolSelectBase");
    m_selectionAction = (SelectionAction)cfg.readEntry("selectionAction", (int)SELECTION_REPLACE);
    m_selectionMode = (SelectionMode)cfg.readEntry("selectionMode", (int)SHAPE_PROTECTION);
    m_antiAliasSelection = cfg.readEntry("antiAliasSelection", true);
    if (m_configGroupForTool != "")
    {
        KConfigGroup cfgToolSpecific = KSharedConfig::openConfig()->group(m_configGroupForTool);
        m_sampleLayersMode = cfgToolSpecific.readEntry("sampleLayersMode", m_optionsWidget->SAMPLE_LAYERS_MODE_CURRENT);
    }

    KisSignalsBlocker b(m_optionsWidget);
    m_optionsWidget->setAction(m_selectionAction);
    m_optionsWidget->setMode(m_selectionMode);
    m_optionsWidget->setAntiAliasSelection(m_antiAliasSelection);
    m_optionsWidget->setSampleLayersMode(m_sampleLayersMode);
}
