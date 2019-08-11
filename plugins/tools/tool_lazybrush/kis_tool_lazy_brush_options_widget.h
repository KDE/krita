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

#ifndef __KIS_TOOL_LAZY_BRUSH_OPTIONS_WIDGET_H
#define __KIS_TOOL_LAZY_BRUSH_OPTIONS_WIDGET_H

#include <QScopedPointer>
#include <QWidget>
#include <QModelIndex>

#include "kis_types.h"

class KisCanvasResourceProvider;
class KoColor;


/**
 * @brief The KisToolLazyBrushOptionsWidget class
 */
class KisToolLazyBrushOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    KisToolLazyBrushOptionsWidget(KisCanvasResourceProvider *provider, QWidget *parent);
    ~KisToolLazyBrushOptionsWidget() override;

private Q_SLOTS:
    void entrySelected(QModelIndex index);
    void slotCurrentFgColorChanged(const KoColor &color);
    void slotCurrentNodeChanged(KisNodeSP node);
    void slotColorLabelsChanged();

    void slotMakeTransparent(bool value);
    void slotRemove();

    void slotUpdate();
    void slotSetAutoUpdates(bool value);
    void slotSetShowKeyStrokes(bool value);
    void slotSetShowOutput(bool value);

    void slotUseEdgeDetectionChanged(bool value);
    void slotEdgeDetectionSizeChanged(int value);
    void slotRadiusChanged(int value);
    void slotCleanUpChanged(int value);
    void slotLimitToDeviceChanged(bool value);


    void slotUpdateNodeProperties();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_TOOL_LAZY_BRUSH_OPTIONS_WIDGET_H */
