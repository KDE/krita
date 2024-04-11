/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_selection_tool_config_widget_helper.h"

#include "kis_selection_options.h"
#include <kis_signals_blocker.h>

#include <KConfigGroup>
#include <KSharedConfig>

KisSelectionToolConfigWidgetHelper::KisSelectionToolConfigWidgetHelper(
    const QString &windowTitle)
    : m_optionsWidget(0)
    , m_windowTitle(windowTitle)
{
}

void KisSelectionToolConfigWidgetHelper::createOptionWidget(
    const QString &toolId)
{
    m_optionsWidget = new KisSelectionOptions;
    Q_CHECK_PTR(m_optionsWidget);

    m_optionsWidget->setObjectName(toolId + "option widget");
    slotToolActivatedChanged(true);

    connect(m_optionsWidget, &KisSelectionOptions::modeChanged,
            this, &KisSelectionToolConfigWidgetHelper::slotWidgetModeChanged);
    connect(m_optionsWidget,
            &KisSelectionOptions::actionChanged,
            this,
            &KisSelectionToolConfigWidgetHelper::slotWidgetActionChanged);
    connect(m_optionsWidget, &KisSelectionOptions::antiAliasSelectionChanged,
            this, &KisSelectionToolConfigWidgetHelper::slotWidgetAntiAliasChanged);
    connect(m_optionsWidget,
            &KisSelectionOptions::growSelectionChanged,
            this,
            &KisSelectionToolConfigWidgetHelper::slotWidgetGrowChanged);
    connect(m_optionsWidget,
            &KisSelectionOptions::stopGrowingAtDarkestPixelChanged,
            this,
            &KisSelectionToolConfigWidgetHelper::slotWidgetStopGrowingAtDarkestPixelChanged);
    connect(m_optionsWidget,
            &KisSelectionOptions::featherSelectionChanged,
            this,
            &KisSelectionToolConfigWidgetHelper::slotWidgetFeatherChanged);
    connect(m_optionsWidget,
            &KisSelectionOptions::referenceLayersChanged,
            this,
            &KisSelectionToolConfigWidgetHelper::slotReferenceLayersChanged);
    connect(m_optionsWidget, &KisSelectionOptions::selectedColorLabelsChanged,
            this, &KisSelectionToolConfigWidgetHelper::slotSelectedColorLabelsChanged);
}

KisSelectionOptions* KisSelectionToolConfigWidgetHelper::optionWidget() const
{
    return m_optionsWidget;
}

SelectionMode KisSelectionToolConfigWidgetHelper::selectionMode() const
{
    if (!m_optionsWidget) {
        return SHAPE_PROTECTION;
    }
    return m_optionsWidget->mode();
}

SelectionAction KisSelectionToolConfigWidgetHelper::selectionAction() const
{
    if (!m_optionsWidget) {
        return SELECTION_DEFAULT;
    }
    return m_optionsWidget->action();
}

bool KisSelectionToolConfigWidgetHelper::antiAliasSelection() const
{
    if (!m_optionsWidget) {
        return true;
    }
    return m_optionsWidget->antiAliasSelection();
}

int KisSelectionToolConfigWidgetHelper::growSelection() const
{
    if (!m_optionsWidget) {
        return 0;
    }
    return m_optionsWidget->growSelection();
}

bool KisSelectionToolConfigWidgetHelper::stopGrowingAtDarkestPixel() const
{
    if (!m_optionsWidget) {
        return false;
    }
    return m_optionsWidget->stopGrowingAtDarkestPixel();
}

int KisSelectionToolConfigWidgetHelper::featherSelection() const
{
    if (!m_optionsWidget) {
        return 0;
    }
    return m_optionsWidget->featherSelection();
}

KisSelectionOptions::ReferenceLayers
KisSelectionToolConfigWidgetHelper::referenceLayers() const
{
    if (!m_optionsWidget) {
        return KisSelectionOptions::CurrentLayer;
    }
    return m_optionsWidget->referenceLayers();
}

QList<int> KisSelectionToolConfigWidgetHelper::selectedColorLabels() const
{
    if (!m_optionsWidget) {
        return {};
    }
    return m_optionsWidget->selectedColorLabels();
}

void KisSelectionToolConfigWidgetHelper::setConfigGroupForExactTool(
    QString toolId)
{
    m_configGroupForTool = toolId;
    reloadExactToolConfig();
}

void KisSelectionToolConfigWidgetHelper::slotWidgetModeChanged(
    SelectionMode mode)
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("KisToolSelectBase");
    cfg.writeEntry("selectionMode", static_cast<int>(mode));
}

void KisSelectionToolConfigWidgetHelper::slotWidgetActionChanged(
    SelectionAction action)
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("KisToolSelectBase");
    cfg.writeEntry("selectionAction", static_cast<int>(action));
    Q_EMIT selectionActionChanged(action);
}

void KisSelectionToolConfigWidgetHelper::slotWidgetAntiAliasChanged(bool value)
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group(m_configGroupForTool);
    cfg.writeEntry("antiAliasSelection", value);
}

void KisSelectionToolConfigWidgetHelper::slotWidgetGrowChanged(int value)
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group(m_configGroupForTool);
    cfg.writeEntry("growSelection", value);
}

void KisSelectionToolConfigWidgetHelper::slotWidgetStopGrowingAtDarkestPixelChanged(bool value)
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group(m_configGroupForTool);
    cfg.writeEntry("stopGrowingAtDarkestPixel", value);
}

void KisSelectionToolConfigWidgetHelper::slotWidgetFeatherChanged(int value)
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group(m_configGroupForTool);
    cfg.writeEntry("featherSelection", value);
}

void KisSelectionToolConfigWidgetHelper::slotReferenceLayersChanged(
    KisSelectionOptions::ReferenceLayers referenceLayers)
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group(m_configGroupForTool);
    cfg.writeEntry(
        "sampleLayersMode",
        referenceLayers == KisSelectionOptions::AllLayers
            ? "sampleAllLayers"
            : (referenceLayers == KisSelectionOptions::ColorLabeledLayers
                   ? "sampleColorLabeledLayers"
                   : "sampleCurrentLayer"));
}

void KisSelectionToolConfigWidgetHelper::slotSelectedColorLabelsChanged()
{
    const QList<int> colorLabels = m_optionsWidget->selectedColorLabels();
    if (colorLabels.isEmpty()) {
        return;
    }
    QString colorLabelsStr = QString::number(colorLabels.first());
    for (int i = 1; i < colorLabels.size(); ++i) {
        colorLabelsStr += "," + QString::number(colorLabels[i]);
    }

    KConfigGroup cfg = KSharedConfig::openConfig()->group(m_configGroupForTool);
    cfg.writeEntry("colorLabels", colorLabelsStr);
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
    if (!isActivated || !m_optionsWidget) {
        return;
    }

    KConfigGroup cfg = KSharedConfig::openConfig()->group("KisToolSelectBase");

    const SelectionMode selectionMode =
        (SelectionMode)cfg.readEntry("selectionMode",
                                     static_cast<int>(SHAPE_PROTECTION));
    const SelectionAction selectionAction =
        (SelectionAction)cfg.readEntry("selectionAction",
                                       static_cast<int>(SELECTION_REPLACE));

    KisSignalsBlocker b(m_optionsWidget);
    m_optionsWidget->setMode(selectionMode);
    m_optionsWidget->setAction(selectionAction);

    reloadExactToolConfig();
}

void KisSelectionToolConfigWidgetHelper::reloadExactToolConfig()
{
    if (m_configGroupForTool == "") {
        return;
    }

    KConfigGroup cfgToolSpecific =
        KSharedConfig::openConfig()->group(m_configGroupForTool);
    const bool antiAliasSelection =
        cfgToolSpecific.readEntry("antiAliasSelection", true);
    const int growSelection = cfgToolSpecific.readEntry("growSelection", 0);
    const bool stopGrowingAtDarkestPixel =
        cfgToolSpecific.readEntry("stopGrowingAtDarkestPixel", false);
    const int featherSelection =
        cfgToolSpecific.readEntry("featherSelection", 0);
    const QString referenceLayersStr =
        cfgToolSpecific.readEntry("sampleLayersMode", "sampleCurrentLayer");

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    const QStringList colorLabelsStr =
        cfgToolSpecific.readEntry<QString>("colorLabels", "")
            .split(',', Qt::SkipEmptyParts);
#else
    const QStringList colorLabelsStr =
        cfgToolSpecific.readEntry<QString>("colorLabels", "")
            .split(',', QString::SkipEmptyParts);
#endif

    const KisSelectionOptions::ReferenceLayers referenceLayers =
        referenceLayersStr == "sampleAllLayers"
        ? KisSelectionOptions::AllLayers
        : (referenceLayersStr == "sampleColorLabeledLayers"
               ? KisSelectionOptions::ColorLabeledLayers
               : KisSelectionOptions::CurrentLayer);
    QList<int> colorLabels;
    for (const QString &colorLabelStr : colorLabelsStr) {
        bool ok;
        const int colorLabel = colorLabelStr.toInt(&ok);
        if (ok) {
            colorLabels << colorLabel;
        }
    }

    KisSignalsBlocker b(m_optionsWidget);
    m_optionsWidget->setAntiAliasSelection(antiAliasSelection);
    m_optionsWidget->setGrowSelection(growSelection);
    m_optionsWidget->setStopGrowingAtDarkestPixel(stopGrowingAtDarkestPixel);
    m_optionsWidget->setFeatherSelection(featherSelection);
    m_optionsWidget->setReferenceLayers(referenceLayers);
    m_optionsWidget->setSelectedColorLabels(colorLabels);
}
