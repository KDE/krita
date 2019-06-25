/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef __KIS_SELECTION_OPTIONS_H__
#define __KIS_SELECTION_OPTIONS_H__

#include <QWidget>

#include "kritaui_export.h"

#include "ui_wdgselectionoptions.h"

class KisCanvas2;
class QButtonGroup;
class QKeySequence;

class WdgSelectionOptions : public QWidget, public Ui::WdgSelectionOptions
{
    Q_OBJECT

public:
    WdgSelectionOptions(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

/**
 */
class KRITAUI_EXPORT KisSelectionOptions : public QWidget
{

    Q_OBJECT

public:
    KisSelectionOptions(KisCanvas2 * subject);
    ~KisSelectionOptions() override;

    int action();

    bool antiAliasSelection();
    void disableAntiAliasSelectionOption();
    void disableSelectionModeOption();
    void setAction(int);
    void setMode(int);
    void setAntiAliasSelection(bool value);
    void enablePixelOnlySelectionMode();

    void updateActionButtonToolTip(int action, const QKeySequence &shortcut);

Q_SIGNALS:
    void actionChanged(int);
    void modeChanged(int);
    void antiAliasSelectionChanged(bool);

private Q_SLOTS:
    void hideActionsForSelectionMode(int mode);

private:
    WdgSelectionOptions * m_page;
    QButtonGroup* m_mode;
    QButtonGroup* m_action;
};

#endif

