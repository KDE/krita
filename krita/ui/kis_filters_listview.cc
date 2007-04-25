/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#include <QPainter>
#include <QPixmap>
#include <QListWidgetItem>
#include <QListWidget>

#include <threadweaver/Job.h>

#include <kdebug.h>
#include <kglobalsettings.h>

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_filter.h"
#include "kis_filter_strategy.h"

using namespace ThreadWeaver;

// ------------------------------------------------

class ThumbnailJob : public Job
{

public:

    ThumbnailJob(QObject * parent, KisPaintDeviceSP dev, KisFiltersIconViewItem * item, KoColorProfile * profile, const QRect & bounds)
        : Job( parent )
        , m_dev( dev )
        , m_item( item )
        , m_canceled( false )
        , m_profile( profile )
        , m_bounds( bounds )
        {
        }

    void requestAbort()
        {
            m_item->filter()->cancel();
            m_canceled = true;
        }

    void run()
        {
            if (m_item->filter()->name() == "Auto Contrast") {
                setFinished(true);
                return;
            }
            m_item->filter()->disableProgress();
            m_item->filter()->process(m_dev, m_bounds, m_item->filterConfiguration());

            if (!m_canceled) {
                kDebug() << "Converting to qimage " << endl;
                m_image = m_dev->convertToQImage(m_profile);

            }

            setFinished( true );
            kDebug() << "done " << m_item->filter()->name() << endl;
        }

    KisFiltersIconViewItem * item()
        {
            kDebug() << "getting item " << m_item->filter()->name() << endl;
            m_item->setIcon( QPixmap::fromImage( m_image ) );
            return m_item;
        }

private:
    KisPaintDeviceSP m_dev;
    KisFiltersIconViewItem * m_item;
    bool m_canceled;
    QImage m_image;
    KoColorProfile * m_profile;
    const QRect m_bounds;
};

// ------------------------------------------------

KisFiltersListView::KisFiltersListView(QWidget* parent, bool filterForAdjustmentLayers)
    : QListWidget(parent)
    , m_original(0)
    , m_profile(0)
    , m_filterForAdjustmentLayers(filterForAdjustmentLayers)
{
    init();
}

KisFiltersListView::KisFiltersListView(KisLayerSP layer, QWidget* parent, bool filterForAdjustmentLayers)
    : QListWidget(parent)
    , m_original(0)
    , m_profile(0)
    , m_filterForAdjustmentLayers(filterForAdjustmentLayers)
{
    init();

    KisPaintLayer* pl = dynamic_cast<KisPaintLayer*>(layer.data());
    if(pl != 0)
    {
        m_original = pl->paintDevice();
        buildPreviews();
    }

}

KisFiltersListView::KisFiltersListView(KisPaintDeviceSP device, QWidget* parent, bool filterForAdjustmentLayers)
    : QListWidget(parent)
    , m_original(device)
    , m_profile(0)
    , m_filterForAdjustmentLayers(filterForAdjustmentLayers)
{
    init();

    buildPreviews();

}

KisFiltersListView::~KisFiltersListView()
{
    m_weaver->requestAbort();
}


void KisFiltersListView::init()
{
    setWindowTitle(i18n("Filters List"));
    setViewMode( QListView::IconMode );
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding ));
    setMinimumWidth(160);

    m_weaver = new Weaver();
    KConfigGroup cfg = KGlobal::config()->group("");
    m_weaver->setMaximumNumberOfThreads( cfg.readEntry("maxthreads",  1) );
    connect( m_weaver, SIGNAL( jobDone(Job*) ), this, SLOT( itemDone( Job* ) ) );
}

void KisFiltersListView::setLayer(KisLayerSP layer) {

    KisPaintLayer* pl = dynamic_cast<KisPaintLayer*>(layer.data());
    if(pl == 0)
        return;
    KisPaintDeviceSP npd = pl->paintDevice();
    if(npd!= m_original)
    {
        m_original = npd;
        buildPreviews();
    }
}

void KisFiltersListView::setCurrentFilter(KoID filter)
{
    // XXX!
//    setCurrentItem(findItem(filter.name()));
}

void KisFiltersListView::buildPreviews()
{
    m_weaver->requestAbort();

    if(m_original.isNull())
        return;

    QApplication::setOverrideCursor(KisCursor::waitCursor());
    m_thumb = m_original->createThumbnailDevice(150, 150);

    QRect bounds = m_thumb->exactBounds();

    foreach(QString id, KisFilterRegistry::instance()->keys()) {
        KisFilterSP filter = KisFilterRegistry::instance()->value(id);
        // Check if filter support the preview and work with the current colorspace
        if (filter->supportsPreview() && filter->workWith( m_original->colorSpace() ) ) {

            if (m_filterForAdjustmentLayers && !filter->supportsAdjustmentLayers()) continue;

            const QHash<QString, KisFilterConfiguration*>& configlist = filter->bookmarkedConfigurations(m_thumb);

            // apply the filter for each of example of configuration
            for ( QHash<QString, KisFilterConfiguration*>::const_iterator itc = configlist.begin();
                itc != configlist.end();
                itc++)
            {
                KisFiltersIconViewItem * item = new KisFiltersIconViewItem(filter.data(), itc.value());
                // XXX: deep copy the thumb?
                item->setText( filter->name() );
                KisPaintDeviceSP thumbPreview = new KisPaintDevice(*m_thumb);
                ThumbnailJob * job = new ThumbnailJob( this, thumbPreview, item, m_profile, bounds );
                m_weaver->enqueue( job );
            }
        }
    }
    m_weaver->finish();
    QApplication::restoreOverrideCursor();
}



void KisFiltersListView::setPaintDevice(KisPaintDeviceSP pd)
{
    if( pd != m_original)
    {
        m_original = pd;
        buildPreviews();
    }
}

void KisFiltersListView::itemDone( Job * job)
{
    ThumbnailJob * thumbnailJob = dynamic_cast<ThumbnailJob*>( job );
    if ( thumbnailJob ) {
        addItem( thumbnailJob->item() );
    }
    delete job;
}

#include "kis_filters_listview.moc"
