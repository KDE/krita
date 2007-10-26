/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_filter_handler.h"

#include <QApplication>

#include <kmessagebox.h>
#include <kguiitem.h>

#include <KoColorSpace.h>

#include <kis_action_recorder.h>
#include <kis_bookmarked_configuration_manager.h>
#include <kis_filter.h>
#include <kis_filter_configuration.h>
#include <kis_filter_processing_information.h>
#include <kis_layer.h>
#include <kis_recorded_filter_action.h>

#include "kis_doc2.h"
#include "kis_filter_dialog.h"
#include "kis_filter_manager.h"
#include "kis_transaction.h"
#include "kis_view2.h"
#include "kis_painter.h"
#include "kis_threaded_applicator.h"

class KisFilterJob : public KisJob {
public:

    KisFilterJob( const KisFilter* filter,
                  const KisFilterConfiguration * config,
                  QObject * parent, KisPaintDeviceSP dev, const QRect & rc, int margin )
        : KisJob( parent, dev, rc, margin )
        , m_filter( filter )
        , m_config( config )
        {
        }


    virtual void run()
        {
            KisPaintDeviceSP dst = new KisPaintDevice( m_dev->colorSpace() );
            QRect marginRect = m_rc.adjusted( -m_margin, -m_margin, m_margin, m_margin );

            // XXX: Make KisFilter::process const! The progress
            // updates should be separate things, because we may be
            // using the same filter instance from several threads.
            // Use Thomas' progress code here.
            const_cast<KisFilter*>( m_filter )->process( KisFilterConstantProcessingInformation( m_dev, marginRect.topLeft()), KisFilterProcessingInformation(
                               dst, marginRect.topLeft() ),
                               marginRect.size(),
                               m_config );
            KisPainter p( m_dev );
            p.setCompositeOp( m_dev->colorSpace()->compositeOp( COMPOSITE_COPY ) );
            p.bitBlt( m_rc.topLeft(), dst, m_rc );
            p.end();
        }

private:

    const KisFilter * m_filter;
    const KisFilterConfiguration * m_config;

};

class KisFilterJobFactory : public KisJobFactory {
public:

    KisFilterJobFactory( const KisFilter* filter, const KisFilterConfiguration * config )
        : m_filter( filter )
        , m_config( config )
        {
        }


    ThreadWeaver::Job * createJob(QObject * parent, KisPaintDeviceSP dev, const QRect & rc, int margin )
        {
            return new KisFilterJob( m_filter, m_config, parent, dev, rc, margin );
        }

private:

    const KisFilter * m_filter;
    const KisFilterConfiguration * m_config;
};

struct KisFilterHandler::Private {

    Private()
        : view(0)
        , manager(0)
        , lastConfiguration(0)
        {
        }

    ~Private()
        {
        }

    KisFilterSP filter;
    KisView2* view;
    KisFilterManager* manager;
    KisFilterConfiguration* lastConfiguration;
};

KisFilterHandler::KisFilterHandler(KisFilterManager* parent, KisFilterSP f, KisView2* view) : QObject(parent), d(new Private)
{
    d->filter = f;
    d->view = view;
    d->manager = parent;
}

KisFilterHandler::~KisFilterHandler()
{
}

void KisFilterHandler::showDialog()
{
    KisPaintDeviceSP dev = d->view->activeDevice();
    if (dev->colorSpace()->willDegrade(d->filter->colorSpaceIndependence())) {
        // Warning bells!
        if (d->filter->colorSpaceIndependence() == TO_LAB16) {
            if (KMessageBox::warningContinueCancel(d->view,
                                                   i18n("The %1 filter will convert your %2 data to 16-bit L*a*b* and vice versa. ",
                                                        d->filter->name(),
                                                        dev->colorSpace()->name()),
                                                   i18n("Filter Will Convert Your Layer Data"),
                                                   KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                                   "lab16degradation") != KMessageBox::Continue) return;

        }
        else if (d->filter->colorSpaceIndependence() == TO_RGBA16) {
            if (KMessageBox::warningContinueCancel(d->view,
                                                   i18n("The %1 filter will convert your %2 data to 16-bit RGBA and vice versa. ",
                                                        d->filter->name() , dev->colorSpace()->name()),
                                                   i18n("Filter Will Convert Your Layer Data"),
                                                   KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                                   "rgba16degradation") != KMessageBox::Continue) return;
        }
    }


    KisFilterDialog* dialog = new KisFilterDialog( d->view , d->view->activeLayer());
    dialog->setFilter( d->filter );
    connect(dialog, SIGNAL(sigPleaseApplyFilter(KisLayerSP, KisFilterConfiguration*)),
            SLOT(apply(KisLayerSP, KisFilterConfiguration*)));
    dialog->setVisible(true);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
}

void KisFilterHandler::reapply()
{
    apply(d->view->activeLayer(), d->lastConfiguration);
}

void KisFilterHandler::apply(KisLayerSP layer, KisFilterConfiguration* config)
{
    kDebug(41007) <<"Applying a filter";
    if( not layer ) return;

    KisPaintDeviceSP dev = layer->paintDevice();

    QRect r1 = dev->extent();
    QRect r2 = layer->image()->bounds();

    // Filters should work only on the visible part of an image.
    QRect rect = r1.intersect(r2);

    if (KisSelectionSP selection = layer->selection() ) {
        QRect r3 = selection->selectedExactRect();
        rect = rect.intersect(r3);
    }

    KisTransaction * cmd = 0;
    if (layer->image()->undo()) cmd = new KisTransaction(d->filter->name(), dev);

    if ( !d->filter->supportsThreading() ) {
        d->filter->process(dev, rect, config);
    }
    else {
        // Chop up in rects.
        KisFilterJobFactory factory( d->filter, config );
        KisThreadedApplicator applicator(dev, rect, &factory, d->filter->overlapMarginNeeded( config ));
        applicator.execute();
    }
/*    if (d->filter->cancelRequested()) { // TODO: port to the progress display reporter
        delete config;
        if (cmd) {
            cmd->undo();
            delete cmd;
        }
    } else */{
        dev->setDirty(rect);
        d->view->document()->setModified(true);
        if (cmd) d->view->document()->addCommand(cmd);
        if(d->filter->bookmarkManager())
        {
            d->filter->bookmarkManager()->save(KisBookmarkedConfigurationManager::ConfigLastUsed.id(), config);
        }
        if(d->lastConfiguration != config)
        {
            delete d->lastConfiguration;
        }
        d->lastConfiguration = config;
        d->manager->setLastFilterHandler(this);

        layer->image()->actionRecorder()->addAction( KisRecordedFilterAction(d->filter->name(), layer, d->filter, config));
    }

    QApplication::restoreOverrideCursor();
}

const KisFilterSP KisFilterHandler::filter() const
{
    return d->filter;
}


#include "kis_filter_handler.moc"
