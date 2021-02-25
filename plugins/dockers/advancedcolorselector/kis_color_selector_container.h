/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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
 */

#ifndef KIS_COLOR_SELECTOR_CONTAINER_H
#define KIS_COLOR_SELECTOR_CONTAINER_H

#include <QWidget>
#include <QPointer>
#include <kis_canvas2.h>

class KisColorSelector;
class KisMyPaintShadeSelector;
class KisMinimalShadeSelector;
class QBoxLayout;
class QAction;
class KisGamutMaskToolbar;

class KisColorSelectorContainer : public QWidget
{
Q_OBJECT
public:
    explicit KisColorSelectorContainer(QWidget *parent = 0);
    void setCanvas(KisCanvas2* canvas);
    void unsetCanvas();
    bool doesAtleastOneDocumentExist();

    enum ShadeSelectorType{MyPaintSelector, MinimalSelector, NoSelector};

public Q_SLOTS:
    void slotUpdateIcons();

Q_SIGNALS:
    void openSettings();
    void settingsChanged();

protected Q_SLOTS:
    void updateSettings();

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    KisColorSelector* m_colorSelector {0};
    KisMyPaintShadeSelector* m_myPaintShadeSelector {0};
    KisMinimalShadeSelector* m_minimalShadeSelector {0};
    QWidget* m_shadeSelector {0};
    KisGamutMaskToolbar* m_gamutMaskToolbar {0};

    int m_onDockerResizeSetting {0};
    bool m_showColorSelector {true};

    QBoxLayout* m_widgetLayout {0};

    QAction * m_colorSelAction {0};
    QAction * m_mypaintAction {0};
    QAction * m_minimalAction {0};

    QPointer<KisCanvas2> m_canvas;
};

#endif // KIS_COLOR_SELECTOR_CONTAINER_H
