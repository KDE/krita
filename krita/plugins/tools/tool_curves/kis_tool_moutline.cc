/*
 *  kis_tool_moutline.cc -- part of Krita
 *
 *  Copyright (c) 2006 Emanuele Tamponi <emanuele@valinor.it>
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

#include <math.h>

#include <qpainter.h>
#include <qlayout.h>
#include <qrect.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdebug.h>
#include <knuminput.h>

#include "kis_global.h"
#include "kis_doc.h"
#include "kis_painter.h"
#include "kis_point.h"
#include "kis_canvas_subject.h"
#include "kis_canvas_controller.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_canvas.h"
#include "kis_canvas_painter.h"
#include "kis_cursor.h"
#include "kis_tool_controller.h"
#include "kis_vec.h"
#include "kis_selection.h"
#include "kis_selection_options.h"
#include "kis_selected_transaction.h"
#include "kis_paintop_registry.h"

#include "kis_tool_moutline.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define RMS(a, b) (sqrt ((a) * (a) + (b) * (b)))
#define ROUND(x) ((int) ((x) + 0.5))

KisCurveMagnetic::KisCurveMagnetic (KisToolMagnetic *parent)
    : m_parent(parent)
{

}

KisCurveMagnetic::~KisCurveMagnetic ()
{

}

void KisCurveMagnetic::calculateCurve (KisCurve::iterator p1, KisCurve::iterator p2, KisCurve::iterator it)
{
    KisPaintDeviceSP src = m_parent->m_currentImage->activeDevice();
    QRect rc = QRect((*p1).point().roundQPoint(),(*p2).point().roundQPoint()).normalize();
    Q_UINT32 pixelSize = src->pixelSize();

    BoolMatrix dst = sobel (rc, src);

    QString line;
    for (int i = 0; i < rc.height(); i++) {
        line = "";
        for (int j = 0; j < rc.width(); j++)
            line += (dst[i][j] ? "1" : "0");
        kdDebug(0) << line << endl;
    }
}

void KisCurveMagnetic::prepareRow (KisPaintDeviceSP src, Q_UINT8* data, Q_UINT32 x, Q_UINT32 y, Q_UINT32 w, Q_UINT32 h)
{
    if (y > h -1) y = h -1;
    Q_UINT32 pixelSize = src->pixelSize();

    src->readBytes( data, x, y, w, 1 );

    for (Q_UINT32 b = 0; b < pixelSize; b++) {
        int offset = pixelSize - b;
        data[-offset] = data[b];
        data[w * pixelSize + b] = data[(w - 1) * pixelSize + b];
    }
}

BoolMatrix KisCurveMagnetic::sobel(const QRect & rc, KisPaintDeviceSP src)
{
    QRect rect = rc; // src->exactBounds();
    Q_UINT32 x = rect.x();
    Q_UINT32 y = rect.y();
    Q_UINT32 width = rect.width();
    Q_UINT32 height = rect.height();
    Q_UINT32 pixelSize = src->pixelSize();
    BoolMatrix dst;

    /*  allocate row buffers  */
    Q_UINT8* prevRow = new Q_UINT8[ (width + 2) * pixelSize];
    Q_CHECK_PTR(prevRow);
    Q_UINT8* curRow = new Q_UINT8[ (width + 2) * pixelSize];
    Q_CHECK_PTR(curRow);
    Q_UINT8* nextRow = new Q_UINT8[ (width + 2) * pixelSize];
    Q_CHECK_PTR(nextRow);

    Q_UINT8* pr = prevRow + pixelSize;
    Q_UINT8* cr = curRow + pixelSize;
    Q_UINT8* nr = nextRow + pixelSize;

    prepareRow (src, pr, x, y - 1, width, height);
    prepareRow (src, cr, x, y, width, height);

    bool isEdge;
    Q_UINT8* tmp;
    Q_INT32 gradient, horGradient, verGradient;
    // loop through the rows, applying the sobel convolution
    for (Q_UINT32 row = y; row < (y+height); row++) {
        // prepare the next row
        prepareRow (src, nr, x, row + 1, width, height);
        
        dst.append(QValueList<bool>());
        for (Q_UINT32 col = x; col < (x+width)*pixelSize; col+=pixelSize) {
            int positive = col + pixelSize;
            int negative = col - pixelSize;
            horGradient = ((pr[negative] + 2 * pr[col] + pr[positive]) -
                           (nr[negative] + 2 * nr[col] + nr[positive]));
            verGradient = ((pr[negative] + 2 * cr[negative] + nr[negative]) -
                           (pr[positive] + 2 * cr[positive] + nr[positive]));
            gradient = (Q_INT32) (ROUND (RMS (horGradient, verGradient)) / 5.66);
            
            if (gradient > 10)
                isEdge = true;
            else
                isEdge = false;

            dst[row-y].append(isEdge);
        }

        //  shuffle the row pointers
        tmp = pr;
        pr = cr;
        cr = nr;
        nr = tmp;
    }

    delete[] prevRow;
    delete[] curRow;
    delete[] nextRow;

    return dst;
}

KisToolMagnetic::KisToolMagnetic ()
    : super("Magnetic Outline Tool")
{
    setName("tool_moutline");
    setCursor(KisCursor::load("tool_moutline_cursor.png", 6, 6));

    m_derived = new KisCurveMagnetic(this);
    m_curve = m_derived;

    m_transactionMessage = i18n("Magnetic Outline Selection");
}

KisToolMagnetic::~KisToolMagnetic ()
{

}

void KisToolMagnetic::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        KShortcut shortcut(Qt::Key_Plus);
        shortcut.append(KShortcut(Qt::Key_F9));
        m_action = new KRadioAction(i18n("&Magnetic Outline"),
                                    "tool_moutline",
                                    shortcut,
                                    this,
                                    SLOT(activate()),
                                    collection,
                                    name());
        Q_CHECK_PTR(m_action);

        m_action->setToolTip(i18n("Magnetic Selection: click around an edge to select the area inside."));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

#include "kis_tool_moutline.moc"
