/*
 *  dlg_colorrange.h -- part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef DLG_COLORRANGE
#define DLG_COLORRANGE

#include <QCursor>

#include <KoDialog.h>

#include <kis_selection.h> // For enums
#include <kis_pixel_selection.h>
#include <kis_types.h>
#include <kis_global.h>

#include "ui_wdg_colorrange.h"

class KisViewManager;
class DlgColorRange;

enum enumAction {
    REDS,
    YELLOWS,
    GREENS,
    CYANS,
    BLUES,
    MAGENTAS,
    HIGHLIGHTS,
    MIDTONES,
    SHADOWS
};

class WdgColorRange : public QWidget, public Ui::WdgColorRange
{
    Q_OBJECT

public:
    WdgColorRange(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

/**
* This dialog allows the user to create a selection mask based
* on a (range of) colors.
*/
class DlgColorRange: public KoDialog
{

    Q_OBJECT

public:

    DlgColorRange(KisViewManager *viewManager, QWidget *parent = 0);
    ~DlgColorRange() override;

private Q_SLOTS:

    void okClicked();
    void cancelClicked();

    void slotInvertClicked();
    void slotSelectionTypeChanged(int index);
    void slotSubtract(bool on);
    void slotAdd(bool on);
    void slotSelectClicked();
    void slotDeselectClicked();

private:
    QImage createMask(KisSelectionSP selection, KisPaintDeviceSP layer);

private:

    WdgColorRange *m_page;
    int m_selectionCommandsAdded;
    KisViewManager *m_viewManager;
    SelectionAction m_mode;
    QCursor m_oldCursor;
    enumAction m_currentAction;
    bool m_invert;
};


#endif // DLG_COLORRANGE
