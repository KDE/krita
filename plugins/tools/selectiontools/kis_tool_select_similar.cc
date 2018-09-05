/*
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2015 Michael Abrahams <miabraha@gmail.com>
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

#include "kis_tool_select_similar.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <ksharedconfig.h>

#include <KoColorSpace.h>

#include <kis_cursor.h>
#include <KoPointerEvent.h>
#include <kis_selection_options.h>
#include <kis_paint_device.h>
#include "kis_canvas2.h"
#include <kis_pixel_selection.h>
#include "kis_selection_tool_helper.h"
#include "kis_slider_spin_box.h"
#include "kis_iterator_ng.h"
#include "kis_image.h"

void selectByColor(KisPaintDeviceSP dev, KisPixelSelectionSP selection, const quint8 *c, int fuzziness, const QRect & rc)
{
    if (rc.isEmpty()) {
        return;
    }
    // XXX: Multithread this!
    qint32 x, y, w, h;
    x = rc.x();
    y = rc.y();
    w = rc.width();
    h = rc.height();

    const KoColorSpace * cs = dev->colorSpace();

    KisHLineConstIteratorSP hiter = dev->createHLineConstIteratorNG(x, y, w);
    KisHLineIteratorSP selIter = selection->createHLineIteratorNG(x, y, w);

    for (int row = y; row < y + h; ++row) {
        do {
            //if (dev->colorSpace()->hasAlpha())
            //    opacity = dev->colorSpace()->alpha(hiter->rawData());
            if (fuzziness == 1) {
                if (memcmp(c, hiter->oldRawData(), cs->pixelSize()) == 0) {
                    *(selIter->rawData()) = MAX_SELECTED;
                }
            }
            else {
                quint8 match = cs->difference(c, hiter->oldRawData());
                if (match <= fuzziness) {
                    *(selIter->rawData()) = MAX_SELECTED;
                }
            }
        }
        while (hiter->nextPixel() && selIter->nextPixel());
        hiter->nextRow();
        selIter->nextRow();
    }

}


KisToolSelectSimilar::KisToolSelectSimilar(KoCanvasBase * canvas)
    : KisToolSelect(canvas,
                    KisCursor::load("tool_similar_selection_cursor.png", 6, 6),
                    i18n("Similar Color Selection")),
      m_fuzziness(20)
{
    connect(&m_widgetHelper, &KisSelectionToolConfigWidgetHelper::selectionActionChanged,
            this, &KisToolSelectSimilar::setSelectionAction);
}

void KisToolSelectSimilar::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KisToolSelect::activate(toolActivation, shapes);
    m_configGroup =  KSharedConfig::openConfig()->group(toolId());
}

void KisToolSelectSimilar::beginPrimaryAction(KoPointerEvent *event)
{
    KisToolSelectBase::beginPrimaryAction(event);
    KisPaintDeviceSP dev;

    if (!currentNode() ||
        !(dev = currentNode()->projection()) ||
        !currentNode()->visible() ||
        !selectionEditable()) {

        event->ignore();
        return;
    }

    QPointF pos = convertToPixelCoord(event);

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);

    QApplication::setOverrideCursor(KisCursor::waitCursor());

    KoColor c;
    dev->pixel(pos.x(), pos.y(), &c);

    // XXX we should make this configurable: "allow to select transparent"
    // if (opacity > OPACITY_TRANSPARENT)
    KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());

    QRect rc;
    if (dev->colorSpace()->difference(c.data(), dev->defaultPixel().data()) <= m_fuzziness) {
        rc = image()->bounds();
    } else {
        rc = dev->exactBounds();
    }
    selectByColor(dev, tmpSel, c.data(), m_fuzziness, rc);

    tmpSel->invalidateOutlineCache();
    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Select Similar Color"));
    helper.selectPixelSelection(tmpSel, selectionAction());

    QApplication::restoreOverrideCursor();

}

void KisToolSelectSimilar::slotSetFuzziness(int fuzziness)
{
    m_fuzziness = fuzziness;
    m_configGroup.writeEntry("fuzziness", fuzziness);
}

QWidget* KisToolSelectSimilar::createOptionWidget()
{
    KisToolSelectBase::createOptionWidget();
    KisSelectionOptions *selectionWidget = selectionOptionWidget();
    selectionWidget->disableAntiAliasSelectionOption();
    selectionWidget->disableSelectionModeOption();

    QHBoxLayout* fl = new QHBoxLayout();
    QLabel * lbl = new QLabel(i18n("Fuzziness: "), selectionWidget);
    fl->addWidget(lbl);

    KisSliderSpinBox* input = new KisSliderSpinBox(selectionWidget);
    input->setObjectName("fuzziness");
    input->setRange(0, 200);
    input->setSingleStep(10);
    fl->addWidget(input);
    connect(input, SIGNAL(valueChanged(int)), this, SLOT(slotSetFuzziness(int)));

    QVBoxLayout* l = dynamic_cast<QVBoxLayout*>(selectionWidget->layout());
    Q_ASSERT(l);
    l->insertLayout(1, fl);

    // load setting from config
    input->setValue(m_configGroup.readEntry("fuzziness", 20));
    return selectionWidget;
}

void KisToolSelectSimilar::setSelectionAction(int action)
{
    changeSelectionAction(action);
}

QMenu* KisToolSelectSimilar::popupActionsMenu()
{
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kisCanvas);


    return KisSelectionToolHelper::getSelectionContextMenu(kisCanvas);
}
