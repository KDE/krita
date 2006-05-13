/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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

#include <stdlib.h>
#include <string.h>
#include <cfloat>
#include <stack>

#include "qbrush.h"
#include "qfontinfo.h"
#include "qfontmetrics.h"
#include "qpen.h"
#include "qregion.h"
#include "qmatrix.h"
#include <QImage>
#include <QMap>
#include <QPainter>
#include <QPixmap>
#include <q3pointarray.h>
#include <QRect>
#include <QString>

#include <kdebug.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_brush.h"
#include "kis_debug_areas.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_pattern.h"
#include "kis_rect.h"
#include "kis_colorspace.h"
#include "kis_transaction.h"
#include "kis_types.h"
#include "kis_vec.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "kis_iterators_pixel.h"
#include "kis_iterator.h"
#include "kis_color.h"
#include "kis_selection.h"

namespace {
}

KisFillPainter::KisFillPainter()
    : super()
{
    m_width = m_height = -1;
    m_sampleMerged = false;
    m_careForSelection = false;
    m_fuzzy = false;
}

KisFillPainter::KisFillPainter(KisPaintDeviceSP device) : super(device)
{
    m_width = m_height = -1;
    m_sampleMerged = false;
    m_careForSelection = false;
    m_fuzzy = false;
}

// 'regular' filling
// XXX: This also needs renaming, since filling ought to keep the opacity and the composite op in mind,
//      this is more eraseToColor.
void KisFillPainter::fillRect(qint32 x1, qint32 y1, qint32 w, qint32 h, const KisColor& kc, quint8 opacity)
{
    if (w > 0 && h > 0) {
        // Make sure we're in the right colorspace

        KisColor kc2(kc); // get rid of const
        kc2.convertTo(m_device->colorSpace());
        quint8 * data = kc2.data();
        m_device->colorSpace()->setAlpha(data, opacity, 1);

        m_device->fill(x1, y1, w, h, data);

        addDirtyRect(QRect(x1, y1, w, h));
    }
}

void KisFillPainter::fillRect(qint32 x1, qint32 y1, qint32 w, qint32 h, KisPattern * pattern) {
    if (!pattern) return;
    if (!pattern->valid()) return;
    if (!m_device) return;


    KisPaintDeviceSP patternLayer = pattern->image(m_device->colorSpace());

    int sx, sy, sw, sh;

    int y = y1;

    if (y >= 0) {
        sy = y % pattern->height();
    } else {
        sy = pattern->height() - (((-y - 1) % pattern->height()) + 1);
    }

    while (y < y1 + h) {
        sh = qMin((y1 + h) - y, pattern->height() - sy);

        int x = x1;

        if (x >= 0) {
            sx = x % pattern->width();
        } else {
            sx = pattern->width() - (((-x - 1) % pattern->width()) + 1);
        }

        while (x < x1 + w) {
            sw = qMin((x1 + w) - x, pattern->width() - sx);

            bitBlt(x, y, m_compositeOp, patternLayer, m_opacity, sx, sy, sw, sh);
            x += sw; sx = 0;
        }

        y+=sh; sy = 0;
    }

    addDirtyRect(QRect(x1, y1, w, h));
}

// flood filling

void KisFillPainter::fillColor(int startX, int startY) {
    genericFillStart(startX, startY);

    // Now create a layer and fill it
    KisPaintDeviceSP filled = KisPaintDeviceSP(new KisPaintDevice(m_device->colorSpace(), "filled"));
    Q_CHECK_PTR(filled);
    KisFillPainter painter(filled);
    painter.fillRect(0, 0, m_width, m_height, m_paintColor);
    painter.end();

    genericFillEnd(filled);
}

void KisFillPainter::fillPattern(int startX, int startY) {
    genericFillStart(startX, startY);

    // Now create a layer and fill it
    KisPaintDeviceSP filled = KisPaintDeviceSP(new KisPaintDevice(m_device->colorSpace(), "filled"));
    Q_CHECK_PTR(filled);
    KisFillPainter painter(filled);
    painter.fillRect(0, 0, m_width, m_height, m_pattern);
    painter.end();

    genericFillEnd(filled);
}

void KisFillPainter::genericFillStart(int startX, int startY) {
    m_cancelRequested = false;

    if (m_width < 0 || m_height < 0) {
        if (m_device->image()) {
            m_width = m_device->image()->width();
            m_height = m_device->image()->height();
        } else {
            m_width = m_height = 500;
        }
    }

    m_size = m_width * m_height;

    // Create a selection from the surrounding area
    m_selection = createFloodSelection(startX, startY);
}

void KisFillPainter::genericFillEnd(KisPaintDeviceSP filled) {
    if (m_cancelRequested) {
        m_width = m_height = -1;
        return;
    }

    bltSelection(0, 0, m_compositeOp, filled, m_selection, m_opacity,
                 0, 0, m_width, m_height);

    emit notifyProgressDone();

    m_width = m_height = -1;
}

struct FillSegment {
    FillSegment(int x, int y/*, FillSegment* parent*/) : x(x), y(y)/*, parent(parent)*/ {}
    int x;
    int y;
//    FillSegment* parent;
};

typedef enum { None = 0, Added = 1, Checked = 2 } Status;

KisSelectionSP KisFillPainter::createFloodSelection(int startX, int startY) {
    if (m_width < 0 || m_height < 0) {
        if (m_device->hasSelection() && m_careForSelection) {
            qint32 x,y,w,h;
            m_device->selection()->extent(x,y,w,h);
            m_width = w - (startX - x);
            m_height = h - (startY - y);
        } else if (m_device->image()) {
            m_width = m_device->image()->width();
            m_height = m_device->image()->height();
        } else {
            m_width = m_height = 500;
        }
    }

    // Don't try to fill if we start outside the borders, just return an empty 'fill'
    if (startX < 0 || startY < 0 || startX >= m_width || startY >= m_height)
        return KisSelectionSP(new KisSelection(m_device));

    KisPaintDeviceSP sourceDevice = KisPaintDeviceSP(0);

    // sample merged?
    if (m_sampleMerged) {
        if (!m_device->image()) {
            return KisSelectionSP(new KisSelection(m_device));
        }
        sourceDevice = m_device->image()->mergedImage();
    } else {
        sourceDevice = m_device;
    }

    m_size = m_width * m_height;

    KisSelectionSP selection = KisSelectionSP(new KisSelection(m_device));
    KisColorSpace * colorSpace = selection->colorSpace();
    KisColorSpace * devColorSpace = sourceDevice->colorSpace();

    quint8* source = new quint8[sourceDevice->pixelSize()];
    KisHLineIteratorPixel pixelIt = sourceDevice->createHLineIterator(startX, startY, startX+1, false);

    memcpy(source, pixelIt.rawData(), sourceDevice->pixelSize());

    std::stack<FillSegment*> stack;

    stack.push(new FillSegment(startX, startY/*, 0*/));

    Status* map = new Status[m_size];

    memset(map, None, m_size * sizeof(Status));

    int progressPercent = 0; int pixelsDone = 0; int currentPercent = 0;
    emit notifyProgressStage(i18n("Making fill outline..."), 0);

    bool hasSelection = m_careForSelection && sourceDevice->hasSelection();
    KisSelectionSP srcSel = KisSelectionSP(0);
    if (hasSelection)
        srcSel = sourceDevice->selection();

    while(!stack.empty()) {
        FillSegment* segment = stack.top();
        stack.pop();
        if (map[m_width * segment->y + segment->x] == Checked) {
            delete segment;
            continue;
        }
        map[m_width * segment->y + segment->x] = Checked;

        int x = segment->x;
        int y = segment->y;

        /* We need an iterator that is valid in the range (0,y) - (width,y). Therefore,
        it is needed to start the iterator at the first position, and then skip to (x,y). */
        pixelIt = sourceDevice->createHLineIterator(0, y, m_width, false);
        pixelIt += x;
        quint8 diff = devColorSpace->difference(source, pixelIt.rawData());

        if (diff >= m_threshold
            || (hasSelection && srcSel->selected(pixelIt.x(), pixelIt.y()) == MIN_SELECTED)) {
            delete segment;
            continue;
        }

        // Here as well: start the iterator at (0,y)
        KisHLineIteratorPixel selIt = selection->createHLineIterator(0, y, m_width, true);
        selIt += x;
        if (m_fuzzy)
            colorSpace->fromQColor(Qt::white, MAX_SELECTED - diff, selIt.rawData());
        else
            colorSpace->fromQColor(Qt::white, MAX_SELECTED, selIt.rawData());

        if (y > 0 && (map[m_width * (y - 1) + x] == None)) {
            map[m_width * (y - 1) + x] = Added;
            stack.push(new FillSegment(x, y-1));
        }
        if (y < (m_height - 1) && (map[m_width * (y + 1) + x] == None)) {
            map[m_width * (y + 1) + x] = Added;
            stack.push(new FillSegment(x, y+1));
        }

        ++pixelsDone;

        bool stop = false;

        --pixelIt;
        --selIt;
        --x;

        // go to the left
        while(!stop && x >= 0 && (map[m_width * y + x] != Checked) ) { // FIXME optimizeable?
            map[m_width * y + x] = Checked;
            diff = devColorSpace->difference(source, pixelIt.rawData());
            if (diff >= m_threshold
                || (hasSelection && srcSel->selected(pixelIt.x(), pixelIt.y()) == MIN_SELECTED)) {
                stop = true;
                continue;
            }

            if (m_fuzzy)
                colorSpace->fromQColor(Qt::white, MAX_SELECTED - diff, selIt.rawData());
            else
                colorSpace->fromQColor(Qt::white, MAX_SELECTED, selIt.rawData());

            if (y > 0 && (map[m_width * (y - 1) + x] == None)) {
                map[m_width * (y - 1) + x] = Added;
                stack.push(new FillSegment(x, y-1));
            }
            if (y < (m_height - 1) && (map[m_width * (y + 1) + x] == None)) {
                map[m_width * (y + 1) + x] = Added;
                stack.push(new FillSegment(x, y+1));
            }
            ++pixelsDone;
            --pixelIt;
            --selIt;
            --x;
        }

        x = segment->x + 1;
        delete segment;

        if (map[m_width * y + x] == Checked)
            continue;

        // and go to the right
        pixelIt = sourceDevice->createHLineIterator(x, y, m_width, false);
        selIt = selection->createHLineIterator(x, y, m_width, true);

        stop = false;
        while(!stop && x < m_width && (map[m_width * y + x] != Checked) ) {
            diff = devColorSpace->difference(source, pixelIt.rawData());
            map[m_width * y + x] = Checked;

            if (diff >= m_threshold
                || (hasSelection && srcSel->selected(pixelIt.x(), pixelIt.y()) == MIN_SELECTED) ) {
                stop = true;
                continue;
            }

            if (m_fuzzy)
                colorSpace->fromQColor(Qt::white, MAX_SELECTED - diff, selIt.rawData());
            else
                colorSpace->fromQColor(Qt::white, MAX_SELECTED, selIt.rawData());

            if (y > 0 && (map[m_width * (y - 1) + x] == None)) {
                map[m_width * (y - 1) + x] = Added;
                stack.push(new FillSegment(x, y-1));
            }
            if (y < (m_height - 1) && (map[m_width * (y + 1) + x] == None)) {
                map[m_width * (y + 1) + x] = Added;
                stack.push(new FillSegment(x, y+1));
            }
            ++pixelsDone;
            ++pixelIt;
            ++selIt;
            ++x;
        }

        if (m_size > 0) {
            progressPercent = (pixelsDone * 100) / m_size;
            if (progressPercent > currentPercent) {
                emit notifyProgress(progressPercent);
                currentPercent = progressPercent;
            }
        }
    }


    delete[] map;
    delete[] source;

    return selection;
}
