/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_fill_painter.h"

#include <stdlib.h>
#include <string.h>
#include <cfloat>
#include <stack>

#include <QMessageBox>
#include <QFontInfo>
#include <QFontMetrics>
#include <QPen>
#include <QMatrix>
#include <QImage>
#include <QMap>
#include <QPainter>
#include <QRect>
#include <QString>

#include <klocale.h>

#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include "generator/kis_generator.h"
#include "filter/kis_filter_configuration.h"
#include "generator/kis_generator_registry.h"
#include "kis_processing_information.h"
#include "kis_debug.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "KoPattern.h"
#include "KoColorSpace.h"
#include "kis_transaction.h"
#include "kis_types.h"

#include "kis_pixel_selection.h"

#include "kis_random_accessor_ng.h"

#include "KoColor.h"
#include "kis_selection.h"

#include "kis_selection_filters.h"

KisFillPainter::KisFillPainter()
        : KisPainter()
{
    initFillPainter();
}

KisFillPainter::KisFillPainter(KisPaintDeviceSP device)
        : KisPainter(device)
{
    initFillPainter();
}

KisFillPainter::KisFillPainter(KisPaintDeviceSP device, KisSelectionSP selection)
        : KisPainter(device, selection)
{
    initFillPainter();
}

void KisFillPainter::initFillPainter()
{
    m_width = m_height = -1;
    m_sampleMerged = false;
    m_careForSelection = false;
    m_fuzzy = false;
    m_sizemod = 0;
    m_feather = 0;
}

// 'regular' filling
// XXX: This also needs renaming, since filling ought to keep the opacity and the composite op in mind,
//      this is more eraseToColor.
void KisFillPainter::fillRect(qint32 x1, qint32 y1, qint32 w, qint32 h, const KoColor& kc, quint8 opacity)
{
    if (w > 0 && h > 0) {
        // Make sure we're in the right colorspace

        KoColor kc2(kc); // get rid of const
        kc2.convertTo(device()->colorSpace());
        quint8 * data = kc2.data();
        device()->colorSpace()->setOpacity(data, opacity, 1);

        device()->fill(x1, y1, w, h, data);

        addDirtyRect(QRect(x1, y1, w, h));
    }
}

void KisFillPainter::fillRect(qint32 x1, qint32 y1, qint32 w, qint32 h, const KoPattern * pattern)
{
    if (!pattern) return;
    if (!pattern->valid()) return;
    if (!device()) return;
    if (w < 1) return;
    if (h < 1) return;

    KisPaintDeviceSP patternLayer = new KisPaintDevice(device()->compositionSourceColorSpace(), pattern->name());
    patternLayer->convertFromQImage(pattern->image(), 0);

    fillRect(x1, y1, w, h, patternLayer, QRect(0, 0, pattern->width(), pattern->height()));
}

void KisFillPainter::fillRect(qint32 x1, qint32 y1, qint32 w, qint32 h, const KisPaintDeviceSP device, const QRect& deviceRect)
{
    Q_ASSERT(deviceRect.x() == 0); // the case x,y != 0,0 is not yet implemented
    Q_ASSERT(deviceRect.y() == 0);
    int sx, sy, sw, sh;

    int y = y1;

    if (y >= 0) {
        sy = y % deviceRect.height();
    } else {
        sy = deviceRect.height() - (((-y - 1) % deviceRect.height()) + 1);
    }

    while (y < y1 + h) {
        sh = qMin((y1 + h) - y, deviceRect.height() - sy);

        int x = x1;

        if (x >= 0) {
            sx = x % deviceRect.width();
        } else {
            sx = deviceRect.width() - (((-x - 1) % deviceRect.width()) + 1);
        }

        while (x < x1 + w) {
            sw = qMin((x1 + w) - x, deviceRect.width() - sx);

            bitBlt(x, y, device, sx, sy, sw, sh);
            x += sw; sx = 0;
        }

        y += sh; sy = 0;
    }

    addDirtyRect(QRect(x1, y1, w, h));
}

void KisFillPainter::fillRect(qint32 x1, qint32 y1, qint32 w, qint32 h, const KisFilterConfiguration* generator)
{
    if (!generator) return;
    KisGeneratorSP g = KisGeneratorRegistry::instance()->value(generator->name());
    if (!generator) return;
    if (!device()) return;
    if (w < 1) return;
    if (h < 1) return;

    QRect tmpRc(x1, y1, w, h);

    KisProcessingInformation dstCfg(device(), tmpRc.topLeft(), 0);

    g->generate(dstCfg, tmpRc.size(), generator);

    addDirtyRect(tmpRc);
}

// flood filling

void KisFillPainter::fillColor(int startX, int startY, KisPaintDeviceSP projection)
{
    genericFillStart(startX, startY, projection);

    // Now create a layer and fill it
    KisPaintDeviceSP filled = device()->createCompositionSourceDevice();
    Q_CHECK_PTR(filled);
    KisFillPainter painter(filled);
    painter.fillRect(0, 0, m_width, m_height, paintColor());
    painter.end();

    genericFillEnd(filled);
}

void KisFillPainter::fillPattern(int startX, int startY, KisPaintDeviceSP projection)
{
    genericFillStart(startX, startY, projection);

    // Now create a layer and fill it
    KisPaintDeviceSP filled = device()->createCompositionSourceDevice();
    Q_CHECK_PTR(filled);
    KisFillPainter painter(filled);
    painter.fillRect(0, 0, m_width, m_height, pattern());
    painter.end();

    genericFillEnd(filled);
}

void KisFillPainter::genericFillStart(int startX, int startY, KisPaintDeviceSP projection)
{
    Q_ASSERT(m_width > 0);
    Q_ASSERT(m_height > 0);

    m_size = m_width * m_height;

    // Create a selection from the surrounding area
    m_fillSelection = createFloodSelection(startX, startY, projection);
}

void KisFillPainter::genericFillEnd(KisPaintDeviceSP filled)
{
    if (progressUpdater() && progressUpdater()->interrupted()) {
        m_width = m_height = -1;
        return;
    }

//  TODO: filling using the correct bound of the selection would be better, *but*
//  the selection is limited to the exact bound of a layer, while in reality, we don't
//  want that, since we want a transparent layer to be completely filled
//     QRect rc = m_fillSelection->selectedExactRect();

    KisSelectionSP tmpSelection = selection();
    setSelection(m_fillSelection);
    bitBlt(0, 0, filled, 0, 0, m_width, m_height);
    setSelection(tmpSelection);

    if (progressUpdater()) progressUpdater()->setProgress(100);

    m_width = m_height = -1;
}

struct FillSegment {
    FillSegment(int x, int y/*, FillSegment* parent*/) : x(x), y(y)/*, parent(parent)*/ {}
    int x;
    int y;
//    FillSegment* parent;
};

static const quint8 None = 0;
static const quint8 Added = 1;
static const quint8 Checked = 2;

KisSelectionSP KisFillPainter::createFloodSelection(int startX, int startY, KisPaintDeviceSP projection)
{
    if (m_width < 0 || m_height < 0) {
        if (selection() && m_careForSelection) {
            QRect rc = selection()->selectedExactRect();
            m_width = rc.width() - (startX - rc.x());
            m_height = rc.height() - (startY - rc.y());
        }
    }
    dbgImage << "Width: " << m_width << " Height: " << m_height;
    // Otherwise the width and height should have been set
    Q_ASSERT(m_width > 0 && m_height > 0);

    // Don't try to fill if we start outside the borders, just return an empty 'fill'
    if (startX < 0 || startY < 0 || startX >= m_width || startY >= m_height)
        return new KisSelection(new KisSelectionDefaultBounds(device()));

    m_size = m_width * m_height;

    try {
        quint8 *map = new quint8[m_size];
        memset(map, None, m_size);
        return createFloodSelectionFast(startX, startY, projection, map); // deletes map
    }
    catch (std::bad_alloc) {
        warnKrita << "KisFillPainter::createFloodSelection failed to allocate" << m_width * m_height << "bytes.";
        return createFloodSelectionFrugal(startX, startY, projection);
    }

}

KisSelectionSP KisFillPainter::createFloodSelectionFast(int startX, int startY, KisPaintDeviceSP projection, quint8 *map)
{
    bool canOptimizeDifferences = (projection->colorSpace()->pixelSize() == 4);
    QHash<quint32, quint8> differences;

    KisPaintDeviceSP sourceDevice = KisPaintDeviceSP(0);

    // sample merged?
    if (m_sampleMerged) {
        if (!projection) {
            return new KisSelection(new KisSelectionDefaultBounds(device()));
        }
        sourceDevice = projection;
    } else {
        sourceDevice = device();
    }

    KisSelectionSP selection = new KisSelection(new KisSelectionDefaultBounds(device()));
    KisPixelSelectionSP pSel = selection->pixelSelection();

    const KoColorSpace * devColorSpace = sourceDevice->colorSpace();

    quint8* source = new quint8[sourceDevice->pixelSize()];
    KisRandomAccessorSP pixelIt = sourceDevice->createRandomAccessorNG(startX, startY);

    memcpy(source, pixelIt->rawData(), sourceDevice->pixelSize());

    std::stack<FillSegment> stack;
    stack.push(FillSegment(startX, startY/*, 0*/));

    int progressPercent = 0; int pixelsDone = 0; int currentPercent = 0;
    if (progressUpdater()) progressUpdater()->setProgress(0);

    bool hasSelection = m_careForSelection && this->selection();
    KisSelectionSP srcSel;

    if (hasSelection) {
        srcSel = this->selection();
    }

    while (!stack.empty()) {
        FillSegment segment = stack.top();
        stack.pop();
        if (map[m_width * segment.y + segment.x] == Checked) {
            continue;
        }
        map[m_width * segment.y + segment.x] = Checked;

        int x = segment.x;
        int y = segment.y;
        Q_ASSERT(x >= 0);
        Q_ASSERT(x < m_width);
        Q_ASSERT(y >= 0);
        Q_ASSERT(y < m_height);

        pixelIt->moveTo(x,y);
        quint8 diff;
        if (canOptimizeDifferences) {
            if (differences.contains(*reinterpret_cast<quint32*>(pixelIt->rawData()))) {
                diff = differences[*reinterpret_cast<quint32*>(pixelIt->rawData())];
            }
            else {
                diff = devColorSpace->difference(source, pixelIt->rawData());
                differences[*reinterpret_cast<quint32*>(pixelIt->rawData())] = diff;
            }
        }
        else {
            diff = devColorSpace->difference(source, pixelIt->rawData());
        }

        if (diff > m_threshold
                || (hasSelection && srcSel->selected(x, y) == MIN_SELECTED)) {
            continue;
        }

        KisRandomAccessorSP selIt = pSel->createRandomAccessorNG(x, y);

        if (m_fuzzy) {
            *selIt->rawData() = quint8(MAX_SELECTED - diff);
        } else
            *selIt->rawData() = MAX_SELECTED;

        if (y > 0 && (map[m_width *(y - 1) + x] == None)) {
            map[m_width *(y - 1) + x] = Added;
            Q_ASSERT(x >= 0);
            Q_ASSERT(x < m_width);
            Q_ASSERT(y - 1 >= 0);
            Q_ASSERT(y - 1 < m_height);
            stack.push(FillSegment(x, y - 1));
        }

        if (y < (m_height - 1) && (map[m_width *(y + 1) + x] == None)) {
            map[m_width *(y + 1) + x] = Added;
            Q_ASSERT(x >= 0);
            Q_ASSERT(x < m_width);
            Q_ASSERT(y + 1 >= 0);
            Q_ASSERT(y + 1 < m_height);
            stack.push(FillSegment(x, y + 1));
        }

        ++pixelsDone;

        bool stop = false;

        --x;
        pixelIt->moveTo(x,y);
        selIt->moveTo(x,y);

        if (x > 0) {
            // go to the left
            while (!stop && x >= 0 && (map[m_width * y + x] != Checked)) { // FIXME optimizeable?
                map[m_width * y + x] = Checked;
                quint8 diff;
                if (canOptimizeDifferences) {
                    if (differences.contains(*reinterpret_cast<quint32*>(pixelIt->rawData()))) {
                        diff = differences[*reinterpret_cast<quint32*>(pixelIt->rawData())];
                    }
                    else {
                        diff = devColorSpace->difference(source, pixelIt->rawData());
                        differences[*reinterpret_cast<quint32*>(pixelIt->rawData())] = diff;
                    }
                }
                else {
                    diff = devColorSpace->difference(source, pixelIt->rawData());
                }
                if (diff > m_threshold
                        || (hasSelection && srcSel->selected(x, y) == MIN_SELECTED)) {
                    stop = true;
                    continue;
                }

                if (m_fuzzy) {
                    *selIt->rawData() = quint8(MAX_SELECTED - diff);
                } else{
                    *selIt->rawData() = MAX_SELECTED;
                }

                if (y > 0 && (map[m_width *(y - 1) + x] == None)) {
                    map[m_width *(y - 1) + x] = Added;
                    Q_ASSERT(x >= 0);
                    Q_ASSERT(x < m_width);
                    Q_ASSERT(y - 1 >= 0);
                    Q_ASSERT(y - 1 < m_height);
                    stack.push(FillSegment(x, y - 1));
                }

                if (y < (m_height - 1) && (map[m_width *(y + 1) + x] == None)) {
                    map[m_width *(y + 1) + x] = Added;
                    Q_ASSERT(x >= 0);
                    Q_ASSERT(x < m_width);
                    Q_ASSERT(y + 1 >= 0);
                    Q_ASSERT(y + 1 < m_height);
                    stack.push(FillSegment(x, y + 1));
                }
                ++pixelsDone;

                --x;
                pixelIt->moveTo(x,y);
                selIt->moveTo(x,y);

            }
        }

        x = segment.x + 1;

        if (x >= m_width) {
            continue;
        }
        Q_ASSERT(x >= 0);
        Q_ASSERT(x < m_width);
        Q_ASSERT(y >= 0);
        Q_ASSERT(y < m_height);

        if (map[m_width * y + x] == Checked)
            continue;

        // and go to the right
        pixelIt->moveTo(x, y);
        selIt->moveTo(x, y);

        stop = false;
        while (!stop && x < m_width && (map[m_width * y + x] != Checked)) {
            quint8 diff;
            if (canOptimizeDifferences) {
                if (differences.contains(*reinterpret_cast<quint32*>(pixelIt->rawData()))) {
                    diff = differences[*reinterpret_cast<quint32*>(pixelIt->rawData())];
                }
                else {
                    diff = devColorSpace->difference(source, pixelIt->rawData());
                    differences[*reinterpret_cast<quint32*>(pixelIt->rawData())] = diff;
                }
            }
            else {
                diff = devColorSpace->difference(source, pixelIt->rawData());
            }
            map[m_width * y + x] = Checked;

            if (diff > m_threshold
                    || (hasSelection && srcSel->selected(x, y) == MIN_SELECTED)) {
                stop = true;
                continue;
            }

            if (m_fuzzy) {
                *selIt->rawData() = quint8(MAX_SELECTED - diff);
            } else {
                *selIt->rawData() = MAX_SELECTED;
            }

            if (y > 0 && (map[m_width *(y - 1) + x] == None)) {
                map[m_width *(y - 1) + x] = Added;
                Q_ASSERT(x >= 0);
                Q_ASSERT(x < m_width);
                Q_ASSERT(y >= 0);
                Q_ASSERT(y - 1 < m_height);
                stack.push(FillSegment(x, y - 1));
            }

            if (y < (m_height - 1) && (map[m_width *(y + 1) + x] == None)) {
                map[m_width *(y + 1) + x] = Added;
                Q_ASSERT(x >= 0);
                Q_ASSERT(x < m_width);
                Q_ASSERT(y + 1 >= 0);
                Q_ASSERT(y + 1 < m_height);
                stack.push(FillSegment(x, y + 1));
            }
            ++pixelsDone;

            ++x;
            pixelIt->moveTo(x,y);
            selIt->moveTo(x,y);
        }

        if (m_size > 0) {
            progressPercent = (pixelsDone * 100) / m_size;
            if (progressPercent > currentPercent) {
                if (progressUpdater()) progressUpdater()->setProgress(progressPercent);
                currentPercent = progressPercent;
            }
        }
    }

    delete[] map;
    delete[] source;

    if (m_sizemod > 0) {
        KisGrowSelectionFilter biggy(m_sizemod, m_sizemod);
        biggy.process(pSel, selection->selectedRect().adjusted(-m_sizemod, -m_sizemod, m_sizemod, m_sizemod));
    }
    else if (m_sizemod < 0) {
        KisShrinkSelectionFilter tiny(-m_sizemod, -m_sizemod, false);
        tiny.process(pSel, selection->selectedRect());
    }
    if (m_feather > 0) {
        KisFeatherSelectionFilter feathery(m_feather);
        feathery.process(pSel, selection->selectedRect().adjusted(-m_feather, -m_feather, m_feather, m_feather));
    }

    selection->updateProjection();

    return selection;}


KisSelectionSP KisFillPainter::createFloodSelectionFrugal(int startX, int startY, KisPaintDeviceSP projection)
{
    bool canOptimizeDifferences = (projection->colorSpace()->pixelSize() == 4);
    QHash<quint32, quint8> differences;
    KisPaintDeviceSP sourceDevice = KisPaintDeviceSP(0);

    // sample merged?
    if (m_sampleMerged) {
        if (!projection) {
            return new KisSelection(new KisSelectionDefaultBounds(device()));
        }
        sourceDevice = projection;
    } else {
        sourceDevice = device();
    }

    KisSelectionSP selection = new KisSelection(new KisSelectionDefaultBounds(device()));
    KisPixelSelectionSP pSel = selection->pixelSelection();

    const KoColorSpace * devColorSpace = sourceDevice->colorSpace();

    quint8* source = new quint8[sourceDevice->pixelSize()];
    KisRandomAccessorSP pixelIt = sourceDevice->createRandomAccessorNG(startX, startY);

    memcpy(source, pixelIt->rawData(), sourceDevice->pixelSize());

    std::stack<FillSegment> stack;
    stack.push(FillSegment(startX, startY/*, 0*/));

    KisPaintDeviceSP map = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8(), "map");

    const quint8 defPixel = 0;
    map->setDefaultPixel(&defPixel);

    KisRandomAccessorSP mapAccessor = map->createRandomAccessorNG(startX, startY);

    int progressPercent = 0; int pixelsDone = 0; int currentPercent = 0;
    if (progressUpdater()) progressUpdater()->setProgress(0);

    bool hasSelection = m_careForSelection && this->selection();
    KisSelectionSP srcSel;

    if (hasSelection) {
        srcSel = this->selection();
    }

    while (!stack.empty()) {
        FillSegment segment = stack.top();
        stack.pop();
        mapAccessor->moveTo(segment.x, segment.y);
        if (mapAccessor->rawDataConst()[0] == Checked) {
            continue;
        }
        memset(mapAccessor->rawData(), Checked, 1);

        int x = segment.x;
        int y = segment.y;
        Q_ASSERT(x >= 0);
        Q_ASSERT(x < m_width);
        Q_ASSERT(y >= 0);
        Q_ASSERT(y < m_height);

        pixelIt->moveTo(x,y);
        quint8 diff;
        if (canOptimizeDifferences) {
            if (differences.contains(*reinterpret_cast<quint32*>(pixelIt->rawData()))) {
                diff = differences[*reinterpret_cast<quint32*>(pixelIt->rawData())];
            }
            else {
                diff = devColorSpace->difference(source, pixelIt->rawData());
                differences[*reinterpret_cast<quint32*>(pixelIt->rawData())] = diff;
            }
        }
        else {
            diff = devColorSpace->difference(source, pixelIt->rawData());
        }

        if (diff > m_threshold
                || (hasSelection && srcSel->selected(x, y) == MIN_SELECTED)) {
            continue;
        }

        KisRandomAccessorSP selIt = pSel->createRandomAccessorNG(x, y);

        if (m_fuzzy) {
            *selIt->rawData() = quint8(MAX_SELECTED - diff);
        } else
            *selIt->rawData() = MAX_SELECTED;

        mapAccessor->moveTo(x, y - 1);
        if (y > 0 && (mapAccessor->rawDataConst()[0] == None)) {
            memset(mapAccessor->rawData(), None, 1);
            Q_ASSERT(x >= 0);
            Q_ASSERT(x < m_width);
            Q_ASSERT(y - 1 >= 0);
            Q_ASSERT(y - 1 < m_height);
            stack.push(FillSegment(x, y - 1));
        }
        mapAccessor->moveTo(x, y + 1);
        if (y < (m_height - 1) && (mapAccessor->rawDataConst()[0] == None)) {
            memset(mapAccessor->rawData(), Added, 1);
            Q_ASSERT(x >= 0);
            Q_ASSERT(x < m_width);
            Q_ASSERT(y + 1 >= 0);
            Q_ASSERT(y + 1 < m_height);
            stack.push(FillSegment(x, y + 1));
        }

        ++pixelsDone;

        bool stop = false;

        --x;
        pixelIt->moveTo(x,y);
        selIt->moveTo(x,y);

        if (x > 0) {
            // go to the left
            while (!stop && x >= 0) {
                mapAccessor->moveTo(x, y);
                if (mapAccessor->rawDataConst()[0] == Checked) {
                    break;
                }
                memset(mapAccessor->rawData(), Checked, 1);
                quint8 diff;
                if (canOptimizeDifferences) {
                    if (differences.contains(*reinterpret_cast<quint32*>(pixelIt->rawData()))) {
                        diff = differences[*reinterpret_cast<quint32*>(pixelIt->rawData())];
                    }
                    else {
                        diff = devColorSpace->difference(source, pixelIt->rawData());
                        differences[*reinterpret_cast<quint32*>(pixelIt->rawData())] = diff;
                    }
                }
                else {
                    diff = devColorSpace->difference(source, pixelIt->rawData());
                }
                if (diff > m_threshold
                        || (hasSelection && srcSel->selected(x, y) == MIN_SELECTED)) {
                    stop = true;
                    continue;
                }

                if (m_fuzzy) {
                    *selIt->rawData() = quint8(MAX_SELECTED - diff);
                } else{
                    *selIt->rawData() = MAX_SELECTED;
                }

                mapAccessor->moveTo(x, y - 1);
                if (y > 0 && (mapAccessor->rawDataConst()[0] == None)) {
                    memset(mapAccessor->rawData(), Added, 1);
                    Q_ASSERT(x >= 0);
                    Q_ASSERT(x < m_width);
                    Q_ASSERT(y - 1 >= 0);
                    Q_ASSERT(y - 1 < m_height);
                    stack.push(FillSegment(x, y - 1));
                }
                mapAccessor->moveTo(x, y + 1);
                if (y < (m_height - 1) && (mapAccessor->rawDataConst()[0] == None)) {
                    memset(mapAccessor->rawData(), Added, 1);
                    Q_ASSERT(x >= 0);
                    Q_ASSERT(x < m_width);
                    Q_ASSERT(y + 1 >= 0);
                    Q_ASSERT(y + 1 < m_height);
                    stack.push(FillSegment(x, y + 1));
                }
                ++pixelsDone;

                --x;
                pixelIt->moveTo(x,y);
                selIt->moveTo(x,y);
            }
        }

        x = segment.x + 1;

        if (x >= m_width) {
            continue;
        }
        Q_ASSERT(x >= 0);
        Q_ASSERT(x < m_width);
        Q_ASSERT(y >= 0);
        Q_ASSERT(y < m_height);

        mapAccessor->moveTo(x, y);
        if (mapAccessor->rawDataConst()[0] == Checked) {
            continue;
        }

        // and go to the right
        pixelIt->moveTo(x, y);
        selIt->moveTo(x, y);

        stop = false;
        while (!stop && x < m_width) {
            mapAccessor->moveTo(x, y);
            if (mapAccessor->rawDataConst()[0] == Checked) {
                break;
            }
            quint8 diff;
            if (canOptimizeDifferences) {
                if (differences.contains(*reinterpret_cast<quint32*>(pixelIt->rawData()))) {
                    diff = differences[*reinterpret_cast<quint32*>(pixelIt->rawData())];
                }
                else {
                    diff = devColorSpace->difference(source, pixelIt->rawData());
                    differences[*reinterpret_cast<quint32*>(pixelIt->rawData())] = diff;
                }
            }
            else {
                diff = devColorSpace->difference(source, pixelIt->rawData());
            }
            memset(mapAccessor->rawData(), Checked, 1);

            if (diff > m_threshold
                    || (hasSelection && srcSel->selected(x, y) == MIN_SELECTED)) {
                stop = true;
                continue;
            }

            if (m_fuzzy) {
                *selIt->rawData() = quint8(MAX_SELECTED - diff);
            } else {
                *selIt->rawData() = MAX_SELECTED;
            }

            mapAccessor->moveTo(x, y - 1);
            if (y > 0 && (mapAccessor->rawDataConst()[0] == None)) {
                memset(mapAccessor->rawData(), Added, 1);
                Q_ASSERT(x >= 0);
                Q_ASSERT(x < m_width);
                Q_ASSERT(y >= 0);
                Q_ASSERT(y - 1 < m_height);
                stack.push(FillSegment(x, y - 1));
            }

            mapAccessor->moveTo(x, y + 1);
            if (y < (m_height - 1) && (mapAccessor->rawDataConst()[0] == None)) {
                //map[m_width *(y + 1) + x] = Added;
                memset(mapAccessor->rawData(), Added, 1);
                Q_ASSERT(x >= 0);
                Q_ASSERT(x < m_width);
                Q_ASSERT(y + 1 >= 0);
                Q_ASSERT(y + 1 < m_height);
                stack.push(FillSegment(x, y + 1));
            }
            ++pixelsDone;

            ++x;
            pixelIt->moveTo(x,y);
            selIt->moveTo(x,y);
        }

        if (m_size > 0) {
            progressPercent = (pixelsDone * 100) / m_size;
            if (progressPercent > currentPercent) {
                if (progressUpdater()) progressUpdater()->setProgress(progressPercent);
                currentPercent = progressPercent;
            }
        }
    }

    delete[] source;

    if (m_sizemod > 0) {
        KisGrowSelectionFilter biggy(m_sizemod, m_sizemod);
        biggy.process(pSel, selection->selectedRect().adjusted(-m_sizemod, -m_sizemod, m_sizemod, m_sizemod));
    }
    else if (m_sizemod < 0) {
        KisShrinkSelectionFilter tiny(-m_sizemod, -m_sizemod, false);
        tiny.process(pSel, selection->selectedRect());
    }
    if (m_feather > 0) {
        KisFeatherSelectionFilter feathery(m_feather);
        feathery.process(pSel, selection->selectedRect().adjusted(-m_feather, -m_feather, m_feather, m_feather));
    }

    selection->updateProjection();

    return selection;
}

