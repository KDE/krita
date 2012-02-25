/*
 * Copyright (c) 2008 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_selection_decoration.h"

#include <QPainter>
#include <QVarLengthArray>

#include <kdebug.h>
#include <klocale.h>

#include "kis_types.h"
#include "kis_view2.h"
#include "kis_selection.h"
#include "kis_image.h"
#include "flake/kis_shape_selection.h"
#include "kis_pixel_selection.h"
#include "kis_selection_manager.h"
#include "canvas/kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_coordinates_converter.h"

KisSelectionDecoration::KisSelectionDecoration(KisView2* view)
    : KisCanvasDecoration("selection", i18n("Selection decoration"), view), m_mode(Ants)
{
    m_offset = 0;
    m_timer = new QTimer(this);

    QRgb white = QColor(Qt::white).rgb();
    QRgb black = QColor(Qt::black).rgb();

    for (int i = 0; i < 8; i++) {
        QImage texture(8, 8, QImage::Format_RGB32);
        for (int y = 0; y < 8; y++) {
            QRgb *pixel = reinterpret_cast<QRgb *>(texture.scanLine(y));
            for (int x = 0; x < 8; x++) {
                pixel[x] = ((x + y + i) % 8 < 4) ? black : white;
            }
        }
        QBrush brush;
        brush.setTextureImage(texture);
        m_brushes << brush;
    }

    // XXX: Make sure no timers are running all the time! We need to
    // provide a signal to tell the selection manager that we've got a
    // current selection now (global or local).
    connect(m_timer, SIGNAL(timeout()), this, SLOT(selectionTimerEvent()));

}

KisSelectionDecoration::~KisSelectionDecoration()
{
}

void KisSelectionDecoration::setMode(Mode mode)
{
    m_mode = mode;
}

bool KisSelectionDecoration::selectionIsActive()
{
    KisImageWSP image = view()->image();
    if (image) {
        KisSelectionSP selection = view()->selection();
        if (selection && (selection->hasPixelSelection() || selection->hasShapeSelection())) {
            return true;
        }
    }
    return false;
}

void KisSelectionDecoration::selectionChanged()
{
    KisSelectionSP selection = view()->selection();

    if (m_mode == Ants) {
        m_outline.clear();

        if (selection) {
            if (selection->hasPixelSelection() || selection->hasShapeSelection()) {
                if (!m_timer->isActive())
                    m_timer->start(300);
            }
            m_outline = selection->outline();
            updateSimpleOutline();
        } else {
            m_timer->stop();
        }
    } else {
        // TODO: optimize this
        updateMaskVisualisation(view()->image()->bounds());
    }

    view()->canvasBase()->updateCanvas();
}

void KisSelectionDecoration::selectionTimerEvent()
{
    KisSelectionSP selection = view()->selection();
    if (!selection) return;

    if (selectionIsActive()) {
        KisPaintDeviceSP dev = view()->activeDevice();
        if (dev) {
            m_offset++;
            if (m_offset > 7) m_offset = 0;

            QRect bound = selection->selectedRect();
            double xRes = view()->image()->xRes();
            double yRes = view()->image()->yRes();
            QRectF rect(int(bound.left()) / xRes, int(bound.top()) / yRes,
                        int(1 + bound.right()) / xRes, int(1 + bound.bottom()) / yRes);
            view()->canvasBase()->updateCanvas(rect);
        }
    }
}

void KisSelectionDecoration::updateSimpleOutline()
{
    m_simpleOutline.clear();
    foreach(const QPolygon & polygon, m_outline) {
        QPolygon simplePolygon;

        simplePolygon << polygon.at(0);
        QPoint previousDelta = polygon.at(1) - polygon.at(0);
        QPoint currentDelta;
        int pointsSinceLastRemoval = 3;
        for (int i = 1; i < polygon.size() - 1; ++i) {
            //check for left turns and turn them into diagonals
            currentDelta = polygon.at(i + 1) - polygon.at(i);
            if ((previousDelta.y() == 1 && currentDelta.x() == 1) || (previousDelta.x() == -1 && currentDelta.y() == 1) ||
                (previousDelta.y() == -1 && currentDelta.x() == -1) || (previousDelta.x() == 1 && currentDelta.y() == -1)) {
                //Turning point found. The point at position i won't be in the simple outline.
                //If there is a staircase, the points in between will be removed.
                if (pointsSinceLastRemoval == 2)
                    simplePolygon.pop_back();
                pointsSinceLastRemoval = 0;

            } else {
                simplePolygon << polygon.at(i);
            }

            previousDelta = currentDelta;
            pointsSinceLastRemoval++;
        }
        simplePolygon << polygon.at(polygon.size() - 1);

        m_simpleOutline.push_back(simplePolygon);
    }
}

void KisSelectionDecoration::drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter)
{
    Q_UNUSED(updateRect);

    KisSelectionSP selection = view()->selection();
    if (!selection || !selection->isVisible())
        return;

    if (m_mode == Mask) {
        Q_ASSERT_X(0, "KisSelectionDecoration.cc", "MASK MODE NOT SUPPORTED YET!");
    }

    if (m_mode == Ants) {

        QTransform transform = converter->imageToWidgetTransform();

        qreal scaleX, scaleY;
        converter->imageScale(&scaleX, &scaleY);

        gc.save();
        gc.setTransform(transform);
        gc.setRenderHints(0);

        QPen pen(m_brushes[m_offset], 0);

        int i = 0;
        gc.setPen(pen);
        if (0.5 * (scaleX + scaleY) < 3) {
            foreach(const QPolygon & polygon, m_simpleOutline) {
                gc.drawPolygon(polygon);
                i++;
            }
        } else {
            foreach(const QPolygon & polygon, m_outline) {
                gc.drawPolygon(polygon);
                i++;
            }
        }

        gc.restore();
    }
}

void KisSelectionDecoration::updateMaskVisualisation(const QRect & r)
{
    Q_UNUSED(r);
    Q_ASSERT_X(0, "KisSelectionDecoration.cc", "MASK MODE NOT SUPPORTED YET!");
}

#include "kis_selection_decoration.moc"

