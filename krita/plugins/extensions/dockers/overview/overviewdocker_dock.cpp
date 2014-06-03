/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "overviewdocker_dock.h"

#include <QLabel>
#include <QHBoxLayout>

#include <klocale.h>

#include "kis_canvas2.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_signal_compressor.h"


OverviewDockerDock::OverviewDockerDock( )
    : QDockWidget(i18n("Overview"))
    , m_canvas(0)
    , m_compressor(new KisSignalCompressor(500, KisSignalCompressor::POSTPONE, this))
{
    QWidget *page = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(page);

    m_preview = new QLabel(page);
    m_preview->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_preview->setAlignment(Qt::AlignCenter);
    m_preview->setFrameStyle(QFrame::Sunken);

    layout->addWidget(m_preview);

    setWidget(page);

    connect(m_compressor, SIGNAL(timeout()), SLOT(startUpdateCanvasProjection()));
}

void OverviewDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->image()->disconnect(this);
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    KIS_ASSERT_RECOVER_RETURN(m_canvas);

    connect(m_canvas->image(), SIGNAL(sigImageUpdated(QRect)), m_compressor, SLOT(start()), Qt::UniqueConnection);
    m_compressor->start();
}

QSize OverviewDockerDock::calculatePreviewSize(const QSize &widgetSize)
{
    const int previewMargin = 5;

    QSize imageSize(m_canvas->image()->bounds().size());
    imageSize.scale(widgetSize - QSize(previewMargin, previewMargin),
                    Qt::KeepAspectRatio);

    return imageSize;
}

void OverviewDockerDock::startUpdateCanvasProjection()
{
    if (!m_canvas) return;

    KisImageSP image = m_canvas->image();
    QSize previewSize = calculatePreviewSize(m_preview->size());

    if (isVisible() && previewSize.isValid()) {
        QImage img =
            image->projection()->
            createThumbnail(previewSize.width(), previewSize.height(), image->bounds());

        m_originalPixmap = QPixmap::fromImage(img);
        m_preview->setPixmap(m_originalPixmap);
    }
}

void OverviewDockerDock::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    m_compressor->start();
}

void OverviewDockerDock::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if (m_canvas && m_preview->pixmap()) {
        if (!m_originalPixmap.isNull()) {
            QSize newSize = calculatePreviewSize(m_preview->size());
            m_preview->setPixmap(m_originalPixmap.scaled(newSize));
        }
        m_compressor->start();
    }
}


#include "overviewdocker_dock.moc"
