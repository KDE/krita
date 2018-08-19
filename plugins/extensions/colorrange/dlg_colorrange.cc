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

#include <klocalizedstring.h>
#include <kis_debug.h>

#include <KoColorConversions.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColor.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceTraits.h>

#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_selection.h>
#include <kis_selection_manager.h>
#include <kis_types.h>
#include <kis_undo_adapter.h>
#include <KisViewManager.h>
#include <kis_transaction.h>
#include <kis_cursor.h>
#include "kis_iterator_ng.h"
#include "kis_selection_tool_helper.h"
#include <kis_slider_spin_box.h>

DlgColorRange::DlgColorRange(KisViewManager *viewManager, QWidget *parent)
    : KoDialog(parent)
    , m_selectionCommandsAdded(0)
    , m_viewManager(viewManager)
{
    setCaption(i18n("Color Range"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_page = new WdgColorRange(this);
    Q_CHECK_PTR(m_page);
    m_page->setObjectName("color_range");

    setCaption(i18n("Color Range"));
    setMainWidget(m_page);
    resize(m_page->sizeHint());

    m_page->intFuzziness->setObjectName("fuzziness");
    m_page->intFuzziness->setRange(0, 200);
    m_page->intFuzziness->setSingleStep(10);
    m_page->intFuzziness->setValue(100);

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
    if (!m_viewManager) return;
    if (!m_viewManager->image()) return;

    for (int i = 0; i < m_selectionCommandsAdded; i++) {
        m_viewManager->undoAdapter()->undoLastCommand();
    }
    m_viewManager->canvas()->update();
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
    KisPaintDeviceSP device = m_viewManager->activeDevice();
    KIS_ASSERT_RECOVER_RETURN(device);

    QRect rc = device->exactBounds();
    if (rc.isEmpty()) return;

    QApplication::setOverrideCursor(KisCursor::waitCursor());

    qint32 x, y, w, h;
    rc.getRect(&x, &y, &w, &h);

    const KoColorSpace *cs = m_viewManager->activeDevice()->colorSpace();
    const KoColorSpace *lab = KoColorSpaceRegistry::instance()->lab16();

    KoColor match;
    switch (m_currentAction) {
    case REDS:
        match = KoColor(QColor(Qt::red), cs);
        break;
    case YELLOWS:
        match = KoColor(QColor(Qt::yellow), cs);
        break;
    case GREENS:
        match = KoColor(QColor(Qt::green), cs);
        break;
    case CYANS:
        match = KoColor(QColor(Qt::cyan), cs);
        break;
    case BLUES:
        match = KoColor(QColor(Qt::blue), cs);
        break;
    case MAGENTAS:
        match = KoColor(QColor(Qt::magenta), cs);
        break;
    default:
        ;
    };

    int fuzziness = m_page->intFuzziness->value();

    KisSelectionSP selection = new KisSelection(new KisSelectionDefaultBounds(m_viewManager->activeDevice(), m_viewManager->image()));

    KisHLineConstIteratorSP hiter = m_viewManager->activeDevice()->createHLineConstIteratorNG(x, y, w);
    KisHLineIteratorSP selIter = selection->pixelSelection()->createHLineIteratorNG(x, y, w);

    for (int row = y; row < h - y; ++row) {
        do {
            // Don't try to select transparent pixels.
            if (cs->opacityU8(hiter->oldRawData()) >  OPACITY_TRANSPARENT_U8) {

                bool selected = false;

                KoColor c(hiter->oldRawData(), cs);
                if (m_currentAction > MAGENTAS) {
                    c.convertTo(lab);
                    quint8 L = lab->scaleToU8(c.data(), 0);

                    switch (m_currentAction) {
                    case HIGHLIGHTS:
                        selected = (L > MAX_SELECTED - fuzziness);
                        break;
                    case MIDTONES:
                        selected = (L > MAX_SELECTED / 2 - fuzziness && L < MAX_SELECTED / 2 + fuzziness);
                        break;
                    case SHADOWS:
                        selected = (L < MIN_SELECTED + fuzziness);
                        break;
                    default:
                        ;
                    }
                }
                else {
                    quint8 difference = cs->difference(match.data(), c.data());
                    selected = (difference <= fuzziness);
                }

                if (selected) {
                    if (!m_invert) {
                        if (m_mode == SELECTION_ADD) {
                            *(selIter->rawData()) =  MAX_SELECTED;
                        } else if (m_mode == SELECTION_SUBTRACT) {
                            *(selIter->rawData()) = MIN_SELECTED;
                        }
                    } else {
                        if (m_mode == SELECTION_ADD) {
                            *(selIter->rawData()) = MIN_SELECTED;
                        } else if (m_mode == SELECTION_SUBTRACT) {
                            *(selIter->rawData()) =  MAX_SELECTED;
                        }
                    }
                }
            }
        } while (hiter->nextPixel() && selIter->nextPixel());
        hiter->nextRow();
        selIter->nextRow();
    }

    selection->pixelSelection()->invalidateOutlineCache();
    KisSelectionToolHelper helper(m_viewManager->canvasBase(), kundo2_i18n("Color Range Selection"));
    helper.selectPixelSelection(selection->pixelSelection(), m_mode);

    m_page->bnDeselect->setEnabled(true);
    m_selectionCommandsAdded++;
    QApplication::restoreOverrideCursor();
}

void DlgColorRange::slotDeselectClicked()
{
    if (!m_viewManager) return;


    m_viewManager->undoAdapter()->undoLastCommand();
    m_selectionCommandsAdded--;
    if (!m_selectionCommandsAdded) {
        m_page->bnDeselect->setEnabled(false);
    }
}


