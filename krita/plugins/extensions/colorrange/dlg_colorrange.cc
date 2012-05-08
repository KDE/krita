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

#include "dlg_colorrange.h"
#include <QApplication>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>
#include <QComboBox>
#include <QImage>
#include <QLabel>
#include <QColor>
#include <QRadioButton>

#include <knuminput.h>
#include <klocale.h>
#include <kis_debug.h>
#include <kaction.h>

#include <KoColorConversions.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>

#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_selection.h>
#include <kis_selection_manager.h>
#include <kis_types.h>
#include <kis_undo_adapter.h>
#include <kis_view2.h>
#include <kis_transaction.h>
#include <kis_cursor.h>
#include "kis_iterator_ng.h"
#include "kis_selection_tool_helper.h"

namespace
{

bool isReddish(int h)
{
    return ((h > 330 && h < 360) || (h > 0 && h < 40));
}

bool isYellowish(int h)
{
    return (h > 40 && h < 65);
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
    rgb_to_hsv(r, g, b, &h, &s, &v);    // XXX: Map the degree in which the colors conform to the requirement
    //      to a range of selectedness between 0 and 255

    switch (action) {

    case REDS:
        if (isReddish(h))
            return MAX_SELECTED;
        else
            return MIN_SELECTED;
    case YELLOWS:
        if (isYellowish(h)) {
            return MAX_SELECTED;
        } else
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

DlgColorRange::DlgColorRange(KisView2 *view, QWidget *parent)
        : KDialog(parent)
        , m_transaction(0)
        , m_selectionCommandsAdded(0)
{
    setCaption(i18n("Color Range"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_view = view;

    m_page = new WdgColorRange(this);
    Q_CHECK_PTR(m_page);
    m_page->setObjectName("color_range");

    setCaption(i18n("Color Range"));
    setMainWidget(m_page);
    resize(m_page->sizeHint());

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

    connect(m_page->radioAdd, SIGNAL(toggled(bool)),
            this, SLOT(slotAdd(bool)));

    connect(m_page->radioSubtract, SIGNAL(toggled(bool)),
            this, SLOT(slotSubtract(bool)));

    connect(m_page->bnSelect, SIGNAL(clicked()),
            this, SLOT(slotSelectClicked()));

    connect(m_page->bnDeselect, SIGNAL(clicked()),
            this, SLOT(slotDeselectClicked()));

    m_page->bnDeselect->setEnabled(false);

}

DlgColorRange::~DlgColorRange()
{
    delete m_page;
}

void DlgColorRange::okClicked()
{
    accept();
}

void DlgColorRange::cancelClicked()
{
    if (!m_view) return;
    if (!m_view->image()) return;

    for (int i = 0; i < m_selectionCommandsAdded; i++) {
        m_view->undoAdapter()->undoLastCommand();
    }
    m_view->canvas()->update();
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
    qint32 x, y, w, h;
    QRect rc = m_view->activeDevice()->exactBounds();
    x = rc.x();
    y = rc.y();
    w = rc.width();
    h = rc.height();

    const KoColorSpace *cs = m_view->activeDevice()->colorSpace();

    KisSelectionSP selection = new KisSelection(new KisSelectionDefaultBounds(m_view->activeDevice(), m_view->image()));

    KisHLineConstIteratorSP hiter = m_view->activeDevice()->createHLineConstIteratorNG(x, y, w);
    KisHLineIteratorSP selIter = selection->getOrCreatePixelSelection()->createHLineIteratorNG(x, y, w);
    QColor c;
    for (int row = y; row < h - y; ++row) {
        do {
            cs->toQColor(hiter->oldRawData(), &c);
            // Don't try to select transparent pixels.
            if (c.alpha() > OPACITY_TRANSPARENT_U8) {
                quint8 match = matchColors(c, m_currentAction);

                if (match) {
                    if (!m_invert) {
                        if (m_mode == SELECTION_ADD) {
                            *(selIter->rawData()) =  match;
                        } else if (m_mode == SELECTION_SUBTRACT) {
                            quint8 selectedness = *(selIter->rawData());
                            if (match < selectedness) {
                                *(selIter->rawData()) = selectedness - match;
                            } else {
                                *(selIter->rawData()) = 0;
                            }
                        }
                    } else {
                        if (m_mode == SELECTION_ADD) {
                            quint8 selectedness = *(selIter->rawData());
                            if (match < selectedness) {
                                *(selIter->rawData()) = selectedness - match;
                            } else {
                                *(selIter->rawData()) = 0;
                            }
                        } else if (m_mode == SELECTION_SUBTRACT) {
                            *(selIter->rawData()) =  match;
                        }
                    }
                }
            }
        } while (hiter->nextPixel() && selIter->nextPixel());
        hiter->nextRow();
        selIter->nextRow();
    }

    // Enable translation after 2.4 release
    KisSelectionToolHelper helper(m_view->canvasBase(), m_view->activeNode(),"Color Range Selection");
    helper.selectPixelSelection(selection->pixelSelection(), m_mode);

    m_page->bnDeselect->setEnabled(true);
    m_selectionCommandsAdded++;
    QApplication::restoreOverrideCursor();
}

void DlgColorRange::slotDeselectClicked()
{
    if (!m_view) return;


    m_view->undoAdapter()->undoLastCommand();
    m_selectionCommandsAdded--;
    if (!m_selectionCommandsAdded) {
        m_page->bnDeselect->setEnabled(false);
    }
}


#include "dlg_colorrange.moc"
