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
#include "kis_update_outline_job.h"
#include "kis_selection_manager.h"
#include "canvas/kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_coordinates_converter.h"

KisSelectionDecoration::KisSelectionDecoration(KisView2* view)
    : KisCanvasDecoration("selection", i18n("Selection decoration"), view),
      m_signalCompressor(500 /*ms*/),
      m_mode(Ants)
{
    m_offset = 0;

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

    m_antsTimer = new QTimer(this);
    m_antsTimer->setInterval(300);
    m_antsTimer->setSingleShot(false);
    connect(m_antsTimer, SIGNAL(timeout()), SLOT(antsAttackEvent()));

    connect(&m_signalCompressor, SIGNAL(timeout()), SLOT(slotStartUpdateSelection()));
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
    Q_ASSERT(image);

    KisSelectionSP selection = view()->selection();
    return visible() && selection &&
        (selection->hasPixelSelection() || selection->hasShapeSelection()) &&
        selection->isVisible();
}

void KisSelectionDecoration::selectionChanged()
{
    KisSelectionSP selection = view()->selection();

    Q_ASSERT_X(m_mode == Ants, "KisSelectionDecoration", "Mask mode is not supported");

    if (selection && selectionIsActive()) {
        if (selection->outlineCacheValid()) {
            m_signalCompressor.stop();
            m_outlinePath = selection->outlineCache();
            view()->canvasBase()->updateCanvas();
            m_antsTimer->start();
        } else {
            m_signalCompressor.start();
        }
    } else {
        m_signalCompressor.stop();
        m_outlinePath = QPainterPath();
        view()->canvasBase()->updateCanvas();
        m_antsTimer->stop();
    }
}

void KisSelectionDecoration::slotStartUpdateSelection()
{
    KisSelectionSP selection = view()->selection();
    if (!selection) return;

    view()->image()->addSpontaneousJob(new KisUpdateOutlineJob(selection));
}

void KisSelectionDecoration::antsAttackEvent()
{
    KisSelectionSP selection = view()->selection();
    if (!selection) return;

    if (selectionIsActive()) {
        KisPaintDeviceSP dev = view()->activeDevice();
        if (dev) {
            m_offset = (m_offset + 1) % 8;
            view()->canvasBase()->updateCanvas();
        }
    }
}

void KisSelectionDecoration::drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter, KisCanvas2 *canvas)
{
    Q_UNUSED(updateRect);
    Q_UNUSED(canvas);
    Q_ASSERT_X(m_mode == Ants, "KisSelectionDecoration.cc", "MASK MODE NOT SUPPORTED YET!");

    if (!selectionIsActive()) return;
    KisSelectionSP selection = view()->selection();

    gc.save();
    gc.setTransform(QTransform(), false);
    gc.setRenderHints(0);

    QPen pen(m_brushes[m_offset], 0);
    QTransform transform = converter->imageToWidgetTransform();

    gc.setPen(pen);

    if (!m_outlinePath.isEmpty()) {
        gc.drawPath(transform.map(m_outlinePath));
    }

    gc.restore();
}

void KisSelectionDecoration::toggleVisibility()
{
    setVisible(!visible());
}

void KisSelectionDecoration::setVisible(bool v)
{
    KisCanvasDecoration::setVisible(v);
    selectionChanged();
}

#include "kis_selection_decoration.moc"

