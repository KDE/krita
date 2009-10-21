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

#include <KoViewConverter.h>

#include "kis_types.h"
#include "kis_view2.h"
#include "kis_selection.h"
#include "kis_image.h"
#include "flake/kis_shape_selection.h"
#include "kis_pixel_selection.h"
#include "kis_selection_manager.h"
#include "canvas/kis_canvas2.h"
#include "kis_canvas_resource_provider.h"

KisSelectionDecoration::KisSelectionDecoration(KisView2* view)
        : KisCanvasDecoration("selection", i18n("Selection decoration"), view), m_mode(Ants)
{
    offset = 0;
    timer = new QTimer(this);

    QRgb white = QColor(Qt::white).rgb();
    QRgb black = QColor(Qt::black).rgb();

    for (int i = 0; i < 8; i++) {
        QImage texture(8, 8, QImage::Format_RGB32);
        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x++)
                texture.setPixel(x, y, ((x + y + i) % 8 < 4) ? black : white);

        QBrush brush;
        brush.setTextureImage(texture);
        brushes << brush;
    }

    int width = KisSelectionDecoration::view()->image()->bounds().width();
    int height = KisSelectionDecoration::view()->image()->bounds().height();
    m_image = QImage(width, height, QImage::Format_ARGB32);

    // XXX: Make sure no timers are running all the time! We need to
    // provide a signal to tell the selection manager that we've got a
    // current selection now (global or local).
    connect(timer, SIGNAL(timeout()), this, SLOT(selectionTimerEvent()));

    KoCanvasResourceProvider* resourceProvider = view->canvasBase()->resourceProvider();
    connect(resourceProvider, SIGNAL(resourceChanged(int, const QVariant &)),
            this, SLOT(resourceChanged(int, const QVariant&)));
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
    KisImageWSP img = view()->image();
    if (img) {
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
        outline.clear();

        if (selection && !selection->isDeselected()) {
            if (selection->hasPixelSelection() || selection->hasShapeSelection()) {
                if (!timer->isActive())
                    timer->start(300);
            }
            if (selection->hasPixelSelection()) {
                KisPixelSelectionSP getOrCreatePixelSelection = selection->getOrCreatePixelSelection();
                outline = getOrCreatePixelSelection->outline();
                updateSimpleOutline();
            }
        } else
            timer->stop();
    } else // TODO: optimize this
        updateMaskVisualisation(view()->image()->bounds());

    view()->canvasBase()->updateCanvas();
}

void KisSelectionDecoration::selectionTimerEvent()
{
    dbgKrita << "selection timer";

    KisSelectionSP selection = view()->selection();
    if (!selection) return;

    if (selectionIsActive()) {
        KisPaintDeviceSP dev = view()->activeDevice();
        if (dev) {
            offset++;
            if (offset > 7) offset = 0;

            dbgKrita << "offset is: " << offset;
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
    simpleOutline.clear();
    foreach(const QPolygon & polygon, outline) {
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

            } else
                simplePolygon << polygon.at(i);

            previousDelta = currentDelta;
            pointsSinceLastRemoval++;
        }
        simplePolygon << polygon.at(polygon.size() - 1);

        simpleOutline.push_back(simplePolygon);
    }
}

void KisSelectionDecoration::drawDecoration(QPainter& painter, const QPoint& documentOffset, const QRect& area, const KoViewConverter& converter)
{

    Q_UNUSED(documentOffset)
    Q_UNUSED(area);
    KisSelectionSP selection = view()->selection();

    if (!selection || selection->isDeselected())
        return;

    qreal sx, sy;
    converter.zoom(&sx, &sy);

    if (m_mode == Mask) {
        painter.save();
        painter.scale(sx / view()->image()->xRes(), sy / view()->image()->yRes());
        painter.drawImage(0, 0, m_image);
        painter.restore();
    }
    if (m_mode == Ants && selection && selection->hasPixelSelection()) {

        QMatrix matrix;
        matrix.scale(sx / view()->image()->xRes(), sy / view()->image()->yRes());

        QMatrix oldWorldMatrix = painter.worldMatrix();
        painter.setWorldMatrix(matrix, true);

        QTime t;
        t.start();
        painter.setRenderHints(0);

        QPen pen(brushes[offset], 0);

        int i = 0;
        painter.setPen(pen);
        if (1 / view()->image()->xRes()*sx < 3)
            foreach(const QPolygon & polygon, simpleOutline) {
            painter.drawPolygon(polygon);
            i++;
        } else
            foreach(const QPolygon & polygon, outline) {
            painter.drawPolygon(polygon);
            i++;
        }

        dbgRender << "Polygons :" << i;
        dbgRender << "Painting marching ants :" << t.elapsed();

        painter.setWorldMatrix(oldWorldMatrix);
    }
    if (m_mode == Ants && selection && selection->hasShapeSelection()) {
        KisShapeSelection* shapeSelection = static_cast<KisShapeSelection*>(selection->shapeSelection());

        QVector<qreal> dashes;
        qreal space = 4;
        dashes << 4 << space;

        QPainterPathStroker stroker;
        stroker.setWidth(0);
        stroker.setDashPattern(dashes);
        stroker.setDashOffset(offset - 4);

        painter.setRenderHint(QPainter::Antialiasing);
        QColor outlineColor = Qt::black;

        QMatrix zoomMatrix;
        zoomMatrix.scale(sx, sy);

        QPen backgroundPen(Qt::white);
        backgroundPen.setCosmetic(true);
        painter.strokePath(zoomMatrix.map(shapeSelection->selectionOutline()), backgroundPen);

        QPainterPath stroke = stroker.createStroke(zoomMatrix.map(shapeSelection->selectionOutline()));
        painter.fillPath(stroke, outlineColor);
    }
}

void KisSelectionDecoration::updateMaskVisualisation(const QRect & r)
{
    if (m_image.isNull() || !view()->selection()) {
        return;
    }

    qint32 width = r.width();
    qint32 height = r.height();

    QVarLengthArray<quint8> buffer(width*height);
    view()->selection()->readBytes(&buffer[0], r.x(), r.y(), width, height);

    for (qint32 y = 0; y < height; y++) {

        QRgb *imagePixel = reinterpret_cast<QRgb *>(m_image.scanLine(y));
        for (qint32 x = 0; x < width; x++) {
            *imagePixel = qRgba(128, 128, 165, 0.5 * (MAX_SELECTED - buffer[y*width+x]));
            imagePixel++;
        }
    }
}

void KisSelectionDecoration::resourceChanged(int key, const QVariant & v)
{
    Q_UNUSED(v);
    if (key == KoCanvasResource::PageSize) {
        int width = KisSelectionDecoration::view()->image()->bounds().width();
        int height = KisSelectionDecoration::view()->image()->bounds().height();
        m_image = QImage(width, height, QImage::Format_ARGB32);

        updateMaskVisualisation(view()->image()->bounds());
        view()->canvasBase()->updateCanvas();
    }
}

#include "kis_selection_decoration.moc"

