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

#include <KoCanvasBase.h>

#include "kis_canvas2.h"
#include "kis_view2.h"
#include "kis_doc2.h"
#include "kis_image.h"
#include "kis_paint_device.h"

OverviewDockerDock::OverviewDockerDock( )
    : QDockWidget(i18n("Overview"))
    , m_canvas(0)
{
    QWidget *page = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(page);

    m_preview = new QLabel(page);
    m_preview->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_preview->setAlignment(Qt::AlignCenter);
    m_preview->setFrameStyle(QFrame::Sunken);

    layout->addWidget(m_preview);

    setWidget(page);
    m_delayTimer.setSingleShot(true);
    connect(&m_delayTimer, SIGNAL(timeout()), SLOT(startUpdateCanvasProjection()));
}

void OverviewDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    Q_ASSERT(m_canvas);
    if (!m_canvas) return;

    connect(m_canvas->image(), SIGNAL(sigImageUpdated(QRect)), SLOT(kickTimer()), Qt::UniqueConnection);
    kickTimer();
}

void OverviewDockerDock::kickTimer()
{
    m_delayTimer.start(100);
}

void OverviewDockerDock::startUpdateCanvasProjection()
{
    if (!m_canvas) return;
    QImage img = m_canvas->image()->projection()->createThumbnail(m_preview->width() -5, m_preview->height() -5);
    img = img.scaled(m_preview->width() - 5, m_preview->height() -5, Qt::KeepAspectRatio);

    m_preview->setPixmap(QPixmap::fromImage(img));
}

void OverviewDockerDock::resizeEvent(QResizeEvent * event)
{
    if (m_canvas && m_preview->pixmap()) {
        m_preview->setPixmap(m_preview->pixmap()->scaled(event->size().width()/2, event->size().height()/2, Qt::KeepAspectRatio));
        kickTimer();
    }
}



#include "overviewdocker_dock.moc"
