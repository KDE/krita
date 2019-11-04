/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_SELECTION_TOOL_CONFIG_WIDGET_HELPER_H
#define __KIS_SELECTION_TOOL_CONFIG_WIDGET_HELPER_H

#include <QObject>

#include "kritaui_export.h"
#include "kis_selection.h"
#include "kis_canvas_resource_provider.h"

class QKeyEvent;
class KisCanvas2;
class KisSelectionOptions;
class KoCanvasResourceProvider;


class KRITAUI_EXPORT KisSelectionToolConfigWidgetHelper : public QObject
{
    Q_OBJECT
public:
    KisSelectionToolConfigWidgetHelper(const QString &windowTitle);

    void createOptionWidget(KisCanvas2 *canvas, const QString &toolId);
    KisSelectionOptions* optionWidget() const;

    SelectionMode selectionMode() const;
    SelectionAction selectionAction() const;
    bool antiAliasSelection() const;
    int action() const { return selectionAction(); }

Q_SIGNALS:
    void selectionActionChanged(int newAction);

public Q_SLOTS:
    void slotToolActivatedChanged(bool isActivated);

    void slotWidgetActionChanged(int action);
    void slotWidgetModeChanged(int mode);
    void slotWidgetAntiAliasChanged(bool value);

    void slotReplaceModeRequested();
    void slotAddModeRequested();
    void slotSubtractModeRequested();
    void slotIntersectModeRequested();
    void slotSymmetricDifferenceModeRequested();

private:
    KisSelectionOptions* m_optionsWidget;

    QString m_windowTitle;

    SelectionMode m_selectionMode {SHAPE_PROTECTION};
    SelectionAction m_selectionAction {SELECTION_DEFAULT};
    bool m_antiAliasSelection {true};
};

#endif /* __KIS_SELECTION_TOOL_CONFIG_WIDGET_HELPER_H */
