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
#include "kis_config.h"
#include "krita_utils.h"

static const unsigned int ANT_LENGTH = 4;
static const unsigned int ANT_SPACE = 4;
static const unsigned int ANT_ADVANCE_WIDTH = ANT_LENGTH + ANT_SPACE;

KisSelectionDecoration::KisSelectionDecoration(KisView2* view)
    : KisCanvasDecoration("selection", i18n("Selection decoration"), view),
      m_signalCompressor(500 /*ms*/, KisSignalCompressor::FIRST_INACTIVE),
      m_offset(0),
      m_mode(Ants)
{
    KritaUtils::initAntsPen(&m_antsPen, &m_outlinePen,
                            ANT_LENGTH, ANT_SPACE);

    m_antsTimer = new QTimer(this);
    m_antsTimer->setInterval(150);
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
    Q_ASSERT(image); Q_UNUSED(image);

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
        m_offset = (m_offset + 1) % ANT_ADVANCE_WIDTH;
        m_antsPen.setDashOffset(m_offset);
        view()->canvasBase()->updateCanvas();
    }
}

void KisSelectionDecoration::drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter, KisCanvas2 *canvas)
{
    Q_UNUSED(updateRect);
    Q_UNUSED(canvas);
    Q_ASSERT_X(m_mode == Ants, "KisSelectionDecoration.cc", "MASK MODE NOT SUPPORTED YET!");

    if (!selectionIsActive()) return;
    if (m_outlinePath.isEmpty()) return;

    gc.save();
    gc.setTransform(QTransform(), false);

    KisConfig cfg;
    gc.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing, cfg.antialiasSelectionOutline());

    QTransform transform = converter->imageToWidgetTransform();

    // render selection outline in white
    gc.setPen(m_outlinePen);
    gc.drawPath(transform.map(m_outlinePath));

    // render marching ants in black (above the white outline)
    gc.setPen(m_antsPen);
    gc.drawPath(transform.map(m_outlinePath));

    gc.restore();
}

void KisSelectionDecoration::setVisible(bool v)
{
    KisCanvasDecoration::setVisible(v);
    selectionChanged();
}

#include "kis_selection_decoration.moc"

