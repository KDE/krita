/*
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include "kis_tool_select_similar.h"

#include <QPoint>
#include <QLayout>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <klocale.h>
#include <knuminput.h>

#include <KoColorSpace.h>
#include <KoCompositeOp.h>

#include <kis_cursor.h>
#include <kis_selection_manager.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <KoPointerEvent.h>
#include <kis_selection_options.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_iterators_pixel.h>
#include <kis_selected_transaction.h>
#include <canvas/kis_canvas2.h>
#include <kis_pixel_selection.h>
#include "kis_selection_tool_helper.h"

void selectByColor(KisPaintDeviceSP dev, KisPixelSelectionSP selection, const quint8 * c, int fuzziness)
{
    // XXX: Multithread this!
    qint32 x, y, w, h;

    dev->exactBounds(x, y, w, h);

    const KoColorSpace * cs = dev->colorSpace();

    KisHLineConstIterator hiter = dev->createHLineConstIterator(x, y, w);
    KisHLineIterator selIter = selection->createHLineIterator(x, y, w);

    for (int row = y; row < y + h; ++row) {
        while (!hiter.isDone()) {
            //if (dev->colorSpace()->hasAlpha())
            //    opacity = dev->colorSpace()->alpha(hiter.rawData());

            quint8 match = cs->difference(c, hiter.rawData());

            if (match <= fuzziness) {
                *(selIter.rawData()) = MAX_SELECTED;
            }
            ++hiter;
            ++selIter;
        }
        hiter.nextRow();
        selIter.nextRow();
    }

}

KisToolSelectSimilar::KisToolSelectSimilar(KoCanvasBase * canvas)
        : KisToolSelectBase(canvas, KisCursor::load("tool_similar_selection_cursor.png", 6, 6))
{
    m_fuzziness = 20;
}

KisToolSelectSimilar::~KisToolSelectSimilar()
{
}


void KisToolSelectSimilar::mousePressEvent(KoPointerEvent *e)
{
    if (m_canvas) {
        QApplication::setOverrideCursor(KisCursor::waitCursor());
        quint8 opacity = OPACITY_OPAQUE;

        if (e->button() != Qt::LeftButton)
            return;

        if (!currentImage())
            return;

        if (!currentNode())
            return;

        KisPaintDeviceSP dev = currentNode()->paintDevice();

        if (!dev || !currentNode()->visible())
            return;

        QPointF pos = convertToPixelCoord(e);

        KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(m_canvas);
        if (!kisCanvas)
            return;

        KoColor c;
        dev->pixel(pos.x(), pos.y(), &c);
        opacity = dev->colorSpace()->alpha(c.data());

        // XXX we should make this configurable: "allow to select transparent"
        // if (opacity > OPACITY_TRANSPARENT)
        KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());
        selectByColor(dev, tmpSel, c.data(), m_fuzziness);

        KisSelectionToolHelper helper(kisCanvas, currentNode(), i18n("Similar Selection"));
        QUndoCommand* cmd = helper.selectPixelSelection(tmpSel, m_selectAction);

        m_canvas->addCommand(cmd);
        QApplication::restoreOverrideCursor();
    }
}


void KisToolSelectSimilar::slotSetFuzziness(int fuzziness)
{
    m_fuzziness = fuzziness;
}

QWidget* KisToolSelectSimilar::createOptionWidget()
{
    KisToolSelectBase::createOptionWidget();
    m_optWidget->setWindowTitle(i18n("Similar Selection"));
    m_optWidget->disableAntiAliasSelectionOption();
    m_optWidget->disableSelectionModeOption();

    QHBoxLayout* fl = new QHBoxLayout();
    QLabel * lbl = new QLabel(i18n("Fuzziness: "), m_optWidget);
    fl->addWidget(lbl);

    KIntNumInput * input = new KIntNumInput(m_optWidget);
    input->setObjectName("fuzziness");
    input->setRange(0, 200, 10);
    input->setValue(m_fuzziness);
    fl->addWidget(input);
    connect(input, SIGNAL(valueChanged(int)), this, SLOT(slotSetFuzziness(int)));

    QVBoxLayout* l = dynamic_cast<QVBoxLayout*>(m_optWidget->layout());
    Q_ASSERT(l);
    l->addLayout(fl);

    return m_optWidget;
}


#include "kis_tool_select_similar.moc"
