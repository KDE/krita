/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_filters_listview.h"

#include <QApplication>
#include <QTimer>
#include <QPainter>
#include <QPixmap>
#include <QTime>

//Added by qt3to4:
#include <QCustomEvent>

#include <kglobalsettings.h>

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_filter.h"
#include "kis_filter_strategy.h"
#include "kis_thread_pool.h"

// ------------------------------------------------

KisFiltersThumbnailThread::KisFiltersThumbnailThread(Q3IconView * parent, KisFiltersIconViewItem * iconItem, KisFilterConfiguration * config, KisFilter * filter, KisPaintDeviceSP dev, const QRect & bounds, KoColorProfile * profile)
    : m_parent(parent)
    , m_iconItem(iconItem)
    , m_config(config)
    , m_filter(filter)
    , m_dev(dev)
    , m_bounds(bounds)
    , m_profile(profile)
{
}

KisFiltersThumbnailThread::~KisFiltersThumbnailThread()
{
    m_iconItem->resetThread();
}

void KisFiltersThumbnailThread::run()
{
    if (m_canceled) return;

    KisPaintDeviceSP thumbPreview = KisPaintDeviceSP(new KisPaintDevice(*m_dev));
    m_filter->disableProgress();
    m_filter->process(thumbPreview, thumbPreview, m_config, m_bounds);

    if (!m_canceled) {
        m_image = thumbPreview->convertToQImage(m_profile);

        qApp->postEvent(m_parent, new KisThumbnailDoneEvent (m_iconItem, m_image));

    }
}

QPixmap KisFiltersThumbnailThread::pixmap()
{
    return QPixmap::fromImage(m_image);
}

void KisFiltersThumbnailThread::cancel()
{
    m_canceled = true;
    m_filter->cancel();

}


// ------------------------------------------------

KisFiltersIconViewItem::KisFiltersIconViewItem(Q3IconView * parent, const QString & text, const QPixmap & icon,
                                               KoID id, KisFilter* filter, KisFilterConfiguration* filterConfig,
                                               KisPaintDeviceSP thumb, const QRect & bounds, KoColorProfile * profile)
    : Q3IconViewItem(parent, text, icon)
    , m_id(id)
    , m_filter(filter)
    , m_filterconfig(filterConfig)
{
    m_thread = new KisFiltersThumbnailThread(parent, this, filterConfig, filter, thumb, bounds, profile);
}

KisFiltersIconViewItem::~KisFiltersIconViewItem()
{
    if (m_thread) m_thread->cancel();
}


// ------------------------------------------------

KisFiltersListView::KisFiltersListView(QWidget* parent, const char* name)
    : K3IconView(parent, name)
    , m_original(0)
    , m_profile(0)
{
    init();
}

KisFiltersListView::KisFiltersListView(KisLayerSP layer, QWidget* parent, const char * name)
    : K3IconView(parent, name)
    , m_original(0)
    , m_profile(0)
{
    KisPaintLayer* pl = dynamic_cast<KisPaintLayer*>(layer.data());
    if(pl != 0)
    {
        m_original = pl->paintDevice();
        buildPreview();
    }
    init();
}

KisFiltersListView::KisFiltersListView(KisPaintDeviceSP device, QWidget* parent, const char * name)
    : K3IconView(parent, name)
    , m_original(device)
    , m_profile(0)
{
    buildPreview();
    init();
}

void KisFiltersListView::init()
{
    setWindowTitle(i18n("Filters List"));
    setItemsMovable(false);
    setSelectionMode(Q3IconView::Single);
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding ));
    setMinimumWidth(160);

}

void KisFiltersListView::setLayer(KisLayerSP layer) {
    KisPaintLayer* pl = dynamic_cast<KisPaintLayer*>(layer.data());
    if(pl == 0)
        return;
    KisPaintDeviceSP npd = pl->paintDevice();
    if(npd!= m_original)
    {
        m_original = npd;
        buildPreview();
    }
}

void KisFiltersListView::setCurrentFilter(KoID filter)
{
    setCurrentItem(findItem(filter.name()));
}

void KisFiltersListView::buildPreview()
{
    QTime t;
    if(m_original.isNull())
        return;

    QApplication::setOverrideCursor(KisCursor::waitCursor());
    t.start();
    m_thumb = m_original->createThumbnailDevice(150, 150);

    t.start();
    QRect bounds = m_thumb->exactBounds();
    QPixmap pm(bounds.width(), bounds.height());
    QPainter gc(&pm);
    gc.fillRect(0, 0, bounds.width(), bounds.height(), palette().background());
    gc.end();

    t.start();
    KoIDList l = KisFilterRegistry::instance()->listKeys();
    KoIDList::iterator it;
    it = l.begin();
    // Iterate over the list of filters
    for (it = l.begin(); it !=  l.end(); ++it) {
        KisFilterSP f = KisFilterRegistry::instance()->get(*it);
        // Check if filter support the preview and work with the current colorspace
        if (f->supportsPreview() && f->workWith( m_original->colorSpace() ) ) {
            std::list<KisFilterConfiguration*> configlist = f->listOfExamplesConfiguration(m_thumb);
            // apply the filter for each of example of configuration
            for(std::list<KisFilterConfiguration*>::iterator itc = configlist.begin();
                itc != configlist.end();
                itc++)
            {
                KisFiltersIconViewItem * icon = new KisFiltersIconViewItem( this, (*it).name(), pm, *it, f.data(), *itc, m_thumb, bounds, m_profile );
                //KisThreadPool::instance()->enqueue(icon->thread());
                icon->thread()->runDirectly();
            }
        }
    }
    QApplication::restoreOverrideCursor();
}


void KisFiltersListView::customEvent(QCustomEvent * e)
{
    KisThumbnailDoneEvent * ev = dynamic_cast<KisThumbnailDoneEvent *>(e);
    if (ev) {
        QPixmap * p = ev->m_iconItem->pixmap();
        QImage img = ev->m_image;
        int x, y;

        if (p->width() > img.width())
            x = (p->width() - img.width()) / 2;
        else
            x = 0;
        if (p->height() > img.height())
            y = (p->height() - img.height()) / 2;
        else
            y = 0;

        QPainter gc(p);
        gc.drawImage(QPoint(x,y), img);
        gc.end();

        //ev->m_iconItem->setPixmap(QPixmap(*p));
        arrangeItemsInGrid();
    }
}
