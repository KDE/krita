/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */


#ifndef KIS_COLOR_SELECTOR_NG_DOCKER_WIDGET_H
#define KIS_COLOR_SELECTOR_NG_DOCKER_WIDGET_H

#include <QWidget>
#include <QPointer>
#include <QToolButton>

#include <kis_canvas2.h>

class QAction;

class KisCommonColors;
class KisColorHistory;
class KisColorSelectorContainer;

class QVBoxLayout;
class QHBoxLayout;

class KisColorSelectorNgDockerWidget : public QWidget
{
Q_OBJECT
public:
    explicit KisColorSelectorNgDockerWidget(QWidget *parent = 0);
    void setCanvas(KisCanvas2* canvas);
    void unsetCanvas();
public Q_SLOTS:
    void openSettings();

Q_SIGNALS:
    void settingsChanged();

protected Q_SLOTS:
    void updateLayout();

private:
    KisColorSelectorContainer* m_colorSelectorContainer;
    KisColorHistory* m_colorHistoryWidget;
    KisCommonColors* m_commonColorsWidget;

    QAction * m_colorHistoryAction;
    QAction * m_commonColorsAction;

    QHBoxLayout* m_widgetLayout;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_horizontalPatchesContainer;
    QVBoxLayout* m_sidebarLayout;

    QHBoxLayout* m_verticalColorPatchesLayout; // vertical color patches should be added here
    QVBoxLayout* m_horizontalColorPatchesLayout;//horizontal ----------"----------------------
    QToolButton* m_fallbackSettingsButton;
    QToolButton* m_clearColorHistoryButton;

    QPointer<KisCanvas2> m_canvas;

};

#endif
