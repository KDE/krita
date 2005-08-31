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

#include <qpixmap.h>
#include <qcolor.h>
#include <qcursor.h>

#include <kdialogbase.h>

#include <kis_types.h>

#include <kis_brush.h>
#include <kis_canvas_observer.h>
#include <kis_canvas_subject.h>
#include <kis_cursor.h>
#include <kis_filter.h>
#include <kis_filter_registry.h>
#include <kis_gradient.h>
#include <kis_id.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_paint_device_impl.h>
#include <kis_pattern.h>
#include <kis_selection.h>
#include <kis_selection_manager.h>
#include <kis_selected_transaction.h>
#include <kis_types.h>
#include <kis_undo_adapter.h>
#include <kis_view.h>
#include <kis_global.h>

#include "wdg_colorrange.h"


class KisView;
class KisCanvasSubject;
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


 /**
 * This dialog allows the user to create a selection mask based
 * on a (range of) colors.
 */
class DlgColorRange: public KDialogBase {
    typedef KDialogBase super;
    Q_OBJECT



public:

    DlgColorRange(KisView * view, KisLayerSP layer, QWidget * parent = 0, const char* name = 0);
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
    QImage createMask(KisSelectionSP selection, KisLayerSP layer);

private:

    WdgColorRange * m_page;
    KisSelectionSP m_selection;
    KisLayerSP m_layer;
    KisView * m_view;
    KisCanvasSubject * m_subject;
    enumSelectionMode m_mode;
    QCursor m_oldCursor;
    KisSelectedTransaction *m_transaction;
    enumAction m_currentAction;
    bool m_invert;
};


#endif // DLG_COLORRANGE
