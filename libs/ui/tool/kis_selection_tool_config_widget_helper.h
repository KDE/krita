/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SELECTION_TOOL_CONFIG_WIDGET_HELPER_H
#define __KIS_SELECTION_TOOL_CONFIG_WIDGET_HELPER_H

#include <QObject>
#include <QList>

#include "kis_canvas_resource_provider.h"
#include "kis_image.h"
#include "kis_selection.h"
#include "kritaui_export.h"
#include <kis_selection_options.h>

class KoCanvasResourceProvider;

class KRITAUI_EXPORT KisSelectionToolConfigWidgetHelper : public QObject
{
    Q_OBJECT
public:
    KisSelectionToolConfigWidgetHelper(const QString &windowTitle);

    void createOptionWidget(const QString &toolId);
    KisSelectionOptions* optionWidget() const;

    SelectionMode selectionMode() const;
    SelectionAction selectionAction() const;
    bool antiAliasSelection() const;
    int growSelection() const;
    bool stopGrowingAtDarkestPixel() const;
    int featherSelection() const;
    KisSelectionOptions::ReferenceLayers referenceLayers() const;
    QList<int> selectedColorLabels() const;

    int action() const { return selectionAction(); }

    void setConfigGroupForExactTool(QString toolId);

Q_SIGNALS:
    void selectionActionChanged(SelectionAction newAction);

public Q_SLOTS:
    void slotToolActivatedChanged(bool isActivated);

    void slotWidgetModeChanged(SelectionMode mode);
    void slotWidgetActionChanged(SelectionAction action);
    void slotWidgetAntiAliasChanged(bool value);
    void slotWidgetGrowChanged(int value);
    void slotWidgetStopGrowingAtDarkestPixelChanged(bool value);
    void slotWidgetFeatherChanged(int value);
    void slotReferenceLayersChanged(
        KisSelectionOptions::ReferenceLayers referenceLayers);
    void slotSelectedColorLabelsChanged();

    void slotReplaceModeRequested();
    void slotAddModeRequested();
    void slotSubtractModeRequested();
    void slotIntersectModeRequested();
    void slotSymmetricDifferenceModeRequested();

private:
    KisSelectionOptions* m_optionsWidget;
    QString m_windowTitle;
    QString m_configGroupForTool {""};

    void reloadExactToolConfig();
};

#endif /* __KIS_SELECTION_TOOL_CONFIG_WIDGET_HELPER_H */
