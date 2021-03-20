/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DEFAULTTOOLTABBEDWIDGET_H
#define DEFAULTTOOLTABBEDWIDGET_H

#include <KoTitledTabWidget.h>
#include "KoShapeMeshGradientHandles.h"

class KoInteractionTool;
class KoFillConfigWidget;
class KoStrokeConfigWidget;
class DefaultToolGeometryWidget;

class DefaultToolTabbedWidget : public KoTitledTabWidget
{
    Q_OBJECT

public:
    explicit DefaultToolTabbedWidget(KoInteractionTool *tool, QWidget *parent = 0);
    ~DefaultToolTabbedWidget() override;

    enum TabType {
        GeometryTab,
        StrokeTab,
        FillTab
    };

    void activate();
    void deactivate();

    bool useUniformScaling() const;

Q_SIGNALS:
    void sigSwitchModeEditFillGradient(bool value);
    void sigSwitchModeEditStrokeGradient(bool value);
    void sigMeshGradientResetted();

public Q_SLOTS:
    void slotMeshGradientHandleSelected(KoShapeMeshGradientHandles::Handle h);

private Q_SLOTS:
    void slotCurrentIndexChanged(int current);

private:
    int m_oldTabIndex;

    DefaultToolGeometryWidget *m_geometryWidget;
    KoFillConfigWidget *m_fillWidget;
    KoStrokeConfigWidget *m_strokeWidget;
};


#endif // DEFAULTTOOLTABBEDWIDGET_H
