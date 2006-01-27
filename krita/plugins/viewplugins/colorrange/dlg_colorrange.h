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

#include <qcursor.h>

#include <kdialogbase.h>

#include <kis_types.h>

#include <kis_selection.h> // For enums
#include <kis_types.h>
#include <kis_global.h>


class KisView;
class KisCanvasSubject;
class DlgColorRange;
class KisSelectedTransaction;
class WdgColorRange;

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


 /**
 * This dialog allows the user to create a selection mask based
 * on a (range of) colors.
 */
class DlgColorRange: public KDialogBase {
    typedef KDialogBase super;
    Q_OBJECT



public:

    DlgColorRange(KisView * view, KisPaintDeviceSP layer, QWidget * parent = 0, const char* name = 0);
    ~DlgColorRange();

private slots:

    void okClicked();
    void cancelClicked();

    void slotInvertClicked();
    void slotSelectionTypeChanged(int index);
    void updatePreview();
    void slotSubtract(bool on);
    void slotAdd(bool on);
    void slotSelectClicked();
    void slotDeselectClicked();

private:
    QImage createMask(KisSelectionSP selection, KisPaintDeviceSP layer);

private:

    WdgColorRange * m_page;
    KisSelectionSP m_selection;
    KisPaintDeviceSP m_dev;
    KisView * m_view;
    KisCanvasSubject * m_subject;
    enumSelectionMode m_mode;
    QCursor m_oldCursor;
    KisSelectedTransaction *m_transaction;
    enumAction m_currentAction;
    bool m_invert;
};


#endif // DLG_COLORRANGE
