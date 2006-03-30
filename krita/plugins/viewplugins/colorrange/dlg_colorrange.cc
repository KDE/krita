/*
 *  dlg_colorrange.cc - part of KimageShop^WKrayon^WKrita
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
#include <qapplication.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qcombobox.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qradiobutton.h>

#include <knuminput.h>
#include <klocale.h>
#include <kdebug.h>
#include <kaction.h>

#include <kis_canvas_subject.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_selection.h>
#include <kis_selection_manager.h>
#include <kis_types.h>
#include <kis_undo_adapter.h>
#include <kis_view.h>
#include <kis_colorspace.h>
#include <kis_profile.h>
#include <kis_color_conversions.h>
#include <kis_selected_transaction.h>
#include <kis_cursor.h>

#include "dlg_colorrange.h"
#include "wdg_colorrange.h"

namespace {

// XXX: Poynton says: hsv/hls is not what one ought to use for colour calculations.
//      Unfortunately, I don't know enough to be able to use anything else.

    bool isReddish(int h)
    {
        return ((h > 330 && h < 360) || ( h > 0 && h < 40));
    }

    bool isYellowish(int h)
    {
        return (h> 40 && h < 65);
    }

    bool isGreenish(int h)
    {
        return (h > 70 && h < 155);
    }

    bool isCyanish(int h)
    {
        return (h > 150 && h < 190);
    }

    bool isBlueish(int h)
    {
        return (h > 185 && h < 270);
    }

    bool isMagentaish(int h)
    {
        return (h > 265 && h < 330);
    }

    bool isHighlight(int v)
    {
        return (v > 200);
    }

    bool isMidTone(int v)
    {
        return (v > 100 && v < 200);
    }

    bool isShadow(int v)
    {
        return (v < 100);
    }

}

quint32 matchColors(const QColor & c, enumAction action)
{
    int r = c.red();
    int g = c.green();
    int b = c.blue();

    int h, s, v;
    rgb_to_hsv(r, g, b, &h, &s, &v);



    // XXX: Map the degree in which the colors conform to the requirement
    //      to a range of selectedness between 0 and 255

    // XXX: Implement out-of-gamut using lcms

    switch(action) {

        case REDS:
            if (isReddish(h))
                return MAX_SELECTED;
            else
                return MIN_SELECTED;
        case YELLOWS:
            if (isYellowish(h)) {
                return MAX_SELECTED;
            }
            else
                return MIN_SELECTED;
        case GREENS:
            if (isGreenish(h))
                return MAX_SELECTED;
            else
                return MIN_SELECTED;
        case CYANS:
            if (isCyanish(h))
                return MAX_SELECTED;
            else
                return MIN_SELECTED;
        case BLUES:
            if (isBlueish(h))
                return MAX_SELECTED;
            else
                return MIN_SELECTED;
        case MAGENTAS:
            if (isMagentaish(h))
                return MAX_SELECTED;
            else
                return MIN_SELECTED;
        case HIGHLIGHTS:
            if (isHighlight(v))
                return MAX_SELECTED;
            else
                return MIN_SELECTED;
        case MIDTONES:
            if (isMidTone(v))
                return MAX_SELECTED;
            else
                return MIN_SELECTED;
        case SHADOWS:
            if (isShadow(v))
                return MAX_SELECTED;
            else
                return MIN_SELECTED;
    };

    return MIN_SELECTED;
}



DlgColorRange::DlgColorRange( KisView * view, KisPaintDeviceSP dev, QWidget *  parent, const char * name)
    : super (parent, name, true, i18n("Color Range"), Ok | Cancel, Ok)
{
    m_dev = dev;
    m_view = view;

    m_subject = view->canvasSubject();

    m_page = new WdgColorRange(this, "color_range");
    Q_CHECK_PTR(m_page);

    setCaption(i18n("Color Range"));
    setMainWidget(m_page);
    resize(m_page->sizeHint());

    if (m_dev->image()->undo()) m_transaction = new KisSelectedTransaction(i18n("Select by Color Range"), m_dev);

    if(! m_dev->hasSelection())
        m_dev->selection()->clear();
    m_selection = m_dev->selection();

        updatePreview();

    m_invert = false;
    m_mode = SELECTION_ADD;
    m_currentAction = REDS;

    connect(this, SIGNAL(okClicked()),
        this, SLOT(okClicked()));

    connect(this, SIGNAL(cancelClicked()),
        this, SLOT(cancelClicked()));

    connect(m_page->chkInvert, SIGNAL(clicked()),
        this, SLOT(slotInvertClicked()));

    connect(m_page->cmbSelect, SIGNAL(activated(int)),
        this, SLOT(slotSelectionTypeChanged(int)));

    connect (m_page->radioAdd, SIGNAL(toggled(bool)),
         this, SLOT(slotAdd(bool)));

    connect (m_page->radioSubtract, SIGNAL(toggled(bool)),
         this, SLOT(slotSubtract(bool)));

    connect (m_page->bnSelect, SIGNAL(clicked()),
        this, SLOT(slotSelectClicked()));

    connect (m_page->bnDeselect, SIGNAL(clicked()),
        this, SLOT(slotDeselectClicked()));

}

DlgColorRange::~DlgColorRange()
{
    delete m_page;
}


void DlgColorRange::updatePreview()
{
    if (!m_selection) return;

    qint32 x, y, w, h;
    m_dev->exactBounds(x, y, w, h);
    QPixmap pix = QPixmap(m_selection->maskImage().smoothScale(350, 350, Qt::KeepAspectRatio));
    m_subject->canvasController()->updateCanvas();
    m_page->pixSelection->setPixmap(pix);
}

void DlgColorRange::okClicked()
{
    if (m_dev->image()->undo()) m_subject->undoAdapter()->addCommand(m_transaction);
    accept();
}

void DlgColorRange::cancelClicked()
{
    if (m_dev->image()->undo()) m_transaction->unexecute();

    m_subject->canvasController()->updateCanvas();
    reject();
}

void DlgColorRange::slotInvertClicked()
{
    m_invert = m_page->chkInvert->isChecked();
}

void DlgColorRange::slotSelectionTypeChanged(int index)
{
    m_currentAction = (enumAction)index;
}

void DlgColorRange::slotSubtract(bool on)
{
    if (on)
        m_mode = SELECTION_SUBTRACT;
}
void DlgColorRange::slotAdd(bool on)
{
    if (on)
        m_mode = SELECTION_ADD;
}

void DlgColorRange::slotSelectClicked()
{
    QApplication::setOverrideCursor(KisCursor::waitCursor());
    // XXX: Multithread this!
    qint32 x, y, w, h;
    m_dev->exactBounds(x, y, w, h);
    KisColorSpace * cs = m_dev->colorSpace();
    quint8 opacity;
    for (int y2 = y; y2 < h - y; ++y2) {
        KisHLineIterator hiter = m_dev->createHLineIterator(x, y2, w, false);
        KisHLineIterator selIter = m_selection ->createHLineIterator(x, y2, w, true);
        while (!hiter.isDone()) {
            QColor c;

            cs->toQColor(hiter.rawData(), &c, &opacity);
            // Don't try to select transparent pixels.
            if (opacity > OPACITY_TRANSPARENT) {
                quint8 match = matchColors(c, m_currentAction);

                if (match) {
                    // Personally, I think the invert option a bit silly. But it's possible I don't quite understand it. BSAR.
                    if (!m_invert) {
                        if (m_mode == SELECTION_ADD) {
                            *(selIter.rawData()) =  match;
                        }
                        else if (m_mode == SELECTION_SUBTRACT) {
                            quint8 selectedness = *(selIter.rawData());
                            if (match < selectedness) {
                                *(selIter.rawData()) = selectedness - match;
                            }
                            else {
                                *(selIter.rawData()) = 0;
                            }
                        }
                    }
                    else {
                        if (m_mode == SELECTION_ADD) {
                            quint8 selectedness = *(selIter.rawData());
                            if (match < selectedness) {
                                *(selIter.rawData()) = selectedness - match;
                            }
                            else {
                                *(selIter.rawData()) = 0;
                            }
                        }
                        else if (m_mode == SELECTION_SUBTRACT) {
                            *(selIter.rawData()) =  match;
                        }
                    }
                }
            }
            ++hiter;
            ++selIter;
        }
    }
    updatePreview();
    QApplication::restoreOverrideCursor();
}

void DlgColorRange::slotDeselectClicked()
{
    m_dev->selection()->clear();
    updatePreview();
}


#include "dlg_colorrange.moc"
