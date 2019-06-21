/*
 * Copyright (c) 2008 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_selection_decoration.h"

#include <QPainter>
#include <QVarLengthArray>

#include <kis_debug.h>
#include <klocalizedstring.h>

#include "kis_types.h"
#include "KisViewManager.h"
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
#include "kis_config_notifier.h"
#include "kis_image_config.h"
#include "KisImageConfigNotifier.h"
#include "kis_painting_tweaks.h"
#include "KisView.h"
#include "kis_selection_mask.h"
#include <KisPart.h>

static const unsigned int ANT_LENGTH = 4;
static const unsigned int ANT_SPACE = 4;
static const unsigned int ANT_ADVANCE_WIDTH = ANT_LENGTH + ANT_SPACE;

KisSelectionDecoration::KisSelectionDecoration(QPointer<KisView>view)
    : KisCanvasDecoration("selection", view),
      m_signalCompressor(500 /*ms*/, KisSignalCompressor::FIRST_INACTIVE),
      m_offset(0),
      m_mode(Ants)
{
    KisPaintingTweaks::initAntsPen(&m_antsPen, &m_outlinePen,
                                   ANT_LENGTH, ANT_SPACE);

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    connect(KisImageConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    slotConfigChanged();

    m_antsTimer = new QTimer(this);
    m_antsTimer->setInterval(150);
    m_antsTimer->setSingleShot(false);
    connect(m_antsTimer, SIGNAL(timeout()), SLOT(antsAttackEvent()));

    connect(&m_signalCompressor, SIGNAL(timeout()), SLOT(slotStartUpdateSelection()));

    // selections should be at the top of the stack
    setPriority(100);
}

KisSelectionDecoration::~KisSelectionDecoration()
{
}

KisSelectionDecoration::Mode KisSelectionDecoration::mode() const
{
    return m_mode;
}

void KisSelectionDecoration::setMode(Mode mode)
{
    m_mode = mode;
    selectionChanged();
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
    KisSelectionMaskSP mask = qobject_cast<KisSelectionMask*>(view()->currentNode().data());
    if (!mask || !mask->active() || !mask->visible(true)) {
        mask = 0;
    }

    if (!view()->isCurrent() ||
        view()->viewManager()->mainWindow() == KisPart::instance()->currentMainwindow()) {

        view()->image()->setOverlaySelectionMask(mask);
    }

    KisSelectionSP selection = view()->selection();

    if (!mask && selection && selectionIsActive()) {
        if ((m_mode == Ants && selection->outlineCacheValid()) ||
            (m_mode == Mask && selection->thumbnailImageValid())) {

            m_signalCompressor.stop();

            if (m_mode == Ants) {
                m_outlinePath = selection->outlineCache();
                m_antsTimer->start();
            } else {
                m_thumbnailImage = selection->thumbnailImage();
                m_thumbnailImageTransform = selection->thumbnailImageTransform();
            }
            if (view() && view()->canvasBase()) {
                view()->canvasBase()->updateCanvas();
            }

        } else {
            m_signalCompressor.start();
        }
    } else {
        m_signalCompressor.stop();
        m_outlinePath = QPainterPath();
        m_thumbnailImage = QImage();
        m_thumbnailImageTransform = QTransform();
        view()->canvasBase()->updateCanvas();
        m_antsTimer->stop();
    }
}

void KisSelectionDecoration::slotStartUpdateSelection()
{
    KisSelectionSP selection = view()->selection();
    if (!selection) return;

    view()->image()->addSpontaneousJob(new KisUpdateOutlineJob(selection, m_mode == Mask, m_maskColor));
}

void KisSelectionDecoration::slotConfigChanged()
{
    KisImageConfig imageConfig(true);
    KisConfig cfg(true);

    m_maskColor = imageConfig.selectionOverlayMaskColor();
    m_antialiasSelectionOutline = cfg.antialiasSelectionOutline();
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

    if (!selectionIsActive()) return;
    if ((m_mode == Ants && m_outlinePath.isEmpty()) ||
        (m_mode == Mask && m_thumbnailImage.isNull())) return;

    QTransform transform = converter->imageToWidgetTransform();

    gc.save();
    gc.setTransform(transform, false);

    if (m_mode == Mask) {
        gc.setRenderHints(QPainter::SmoothPixmapTransform |
                          QPainter::HighQualityAntialiasing, false);

        gc.setTransform(m_thumbnailImageTransform, true);
        gc.drawImage(QPoint(), m_thumbnailImage);

        QRect r1 = m_thumbnailImageTransform.inverted().mapRect(view()->image()->bounds());
        QRect r2 = m_thumbnailImage.rect();

        QPainterPath p1;
        p1.addRect(r1);

        QPainterPath p2;
        p2.addRect(r2);

        gc.setBrush(m_maskColor);
        gc.setPen(Qt::NoPen);
        gc.drawPath(p1 - p2);

    } else /* if (m_mode == Ants) */ {
        gc.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing, m_antialiasSelectionOutline);

        // render selection outline in white
        gc.setPen(m_outlinePen);
        gc.drawPath(m_outlinePath);

        // render marching ants in black (above the white outline)
        gc.setPen(m_antsPen);
        gc.drawPath(m_outlinePath);
    }
    gc.restore();
}

void KisSelectionDecoration::setVisible(bool v)
{
    KisCanvasDecoration::setVisible(v);
    selectionChanged();
}


