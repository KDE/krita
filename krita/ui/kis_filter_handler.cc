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
#include <QRect>
#include <kmessagebox.h>
#include <kguiitem.h>

#include <KoColorSpace.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <recorder/kis_action_recorder.h>
#include <kis_bookmarked_configuration_manager.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <kis_layer.h>
#include <recorder/kis_recorded_filter_action.h>
#include <kis_selection.h>
#include <kis_image.h>

#include "kis_statusbar.h"
#include "kis_doc2.h"
#include "dialogs/kis_dlg_filter.h"
#include "kis_filter_manager.h"
#include "kis_transaction.h"
#include "kis_view2.h"
#include "kis_painter.h"
#include "kis_threaded_applicator.h"
#include "filter/kis_filter_job.h"
#include "filter/kis_filter_registry.h"
#include "kis_system_locker.h"
#include <recorder/kis_node_query_path.h>

struct KisFilterHandler::Private {

    Private()
        : view(0)
        , manager(0)
        , lastConfiguration(0)
        , updater( 0 )
    {
    }

    ~Private() {
    }

    KisFilterSP filter;
    KisView2* view;
    KisFilterManager* manager;
    KisFilterConfiguration* lastConfiguration;
    KisPaintDeviceSP dev;
    KoProgressUpdater* updater;
};

KisFilterHandler::KisFilterHandler(KisFilterManager* parent, KisFilterSP f, KisView2* view)
        : QObject(parent)
        , m_d(new Private)
{
    m_d->filter = f;
    m_d->view = view;
    m_d->manager = parent;
}

KisFilterHandler::~KisFilterHandler()
{
    delete m_d;
}

void KisFilterHandler::showDialog()
{
    KisPaintDeviceSP dev = m_d->view->activeDevice();
    if (dev->colorSpace()->willDegrade(m_d->filter->colorSpaceIndependence())) {
        // Warning bells!
        if (m_d->filter->colorSpaceIndependence() == TO_LAB16) {
            if (KMessageBox::warningContinueCancel(m_d->view,
                                                   i18n("The %1 filter will convert your %2 data to 16-bit L*a*b* and vice versa. ",
                                                        m_d->filter->name(),
                                                        dev->colorSpace()->name()),
                                                   i18n("Filter Will Convert Your Layer Data"),
                                                   KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                                   "lab16degradation") != KMessageBox::Continue) return;

        } else if (m_d->filter->colorSpaceIndependence() == TO_RGBA16) {
            if (KMessageBox::warningContinueCancel(m_d->view,
                                                   i18n("The %1 filter will convert your %2 data to 16-bit RGBA and vice versa. ",
                                                        m_d->filter->name() , dev->colorSpace()->name()),
                                                   i18n("Filter Will Convert Your Layer Data"),
                                                   KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                                   "rgba16degradation") != KMessageBox::Continue) return;
        }
    }


    KisFilterDialog* dialog = new KisFilterDialog(m_d->view , m_d->view->activeNode(), m_d->view->image());
    dialog->setFilter(m_d->filter);
    connect(dialog, SIGNAL(sigPleaseApplyFilter(KisNodeSP, KisFilterConfiguration*)),
            SLOT(apply(KisNodeSP, KisFilterConfiguration*)));
    dialog->setVisible(true);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
}

void KisFilterHandler::reapply()
{
    apply(m_d->view->activeLayer(), m_d->lastConfiguration);
}

void KisFilterHandler::apply(KisNodeSP layer, KisFilterConfiguration* config)
{
    // XXX: if the layer only had a preview mask and is a paint layer, then use flatten instead of applying the filter again
    dbgUI << "Applying a filter";
    if (!layer) return;

    KisSystemLocker l( layer );

    KisFilterSP filter = KisFilterRegistry::instance()->value(config->name());

    m_d->dev = layer->paintDevice();

    QRect r1 = m_d->dev->extent();
    QRect r2 = m_d->view->image()->bounds();

    // Filters should work only on the visible part of an image.
    QRect rect = r1.intersect(r2);

    KisSelectionSP selection = m_d->view->selection();

    if (selection) {
        QRect r3 = selection->selectedExactRect();
        rect = rect.intersect(r3);
    }

    KisTransaction * cmd = 0;

    if ( m_d->updater == 0 ) {
        m_d->updater = new KoProgressUpdater(m_d->view->statusBar()->progress());
    }
    // also deletes all old updaters
    m_d->updater->start( 100, filter->name() );

    if (m_d->view->image()->undo()) cmd = new KisTransaction(filter->name(), m_d->dev);

    KisProcessingInformation src(m_d->dev, rect.topLeft(), selection);
    KisProcessingInformation dst(m_d->dev, rect.topLeft(), selection);

    if (!filter->supportsThreading()) {
        KoUpdater* up = m_d->updater->startSubtask();
        filter->process(src, dst, rect.size(), config, up);
        areaDone(rect);
    } else {
        // Chop up in rects.
        KisFilterJobFactory factory(filter, config, selection);
        KisThreadedApplicator applicator(m_d->dev, rect, &factory, m_d->updater, filter->overlapMarginNeeded(config));
        applicator.connect(&applicator, SIGNAL(areaDone(const QRect&)), this, SLOT(areaDone(const QRect &)));
        applicator.execute();
    }

    /*    if (filter->cancelRequested()) { // TODO: port to the progress display reporter
            delete config;
            if (cmd) {
                cmm_d->undo();
                delete cmd;
            }
        } else */
    {
        if (cmd) m_d->view->document()->addCommand(cmd);
        if (filter->bookmarkManager()) {
            filter->bookmarkManager()->save(KisBookmarkedConfigurationManager::ConfigLastUsed.id(), config); // TODO ugly const_cast
        }
        if (m_d->lastConfiguration != config) {
            delete m_d->lastConfiguration;
        }
        m_d->lastConfiguration = config;
        m_d->manager->setLastFilterHandler(this);

        m_d->view->image()->actionRecorder()->addAction(KisRecordedFilterAction(filter->name(), KisNodeQueryPath::absolutePath(layer), filter, config));
    }
    QApplication::restoreOverrideCursor();
}

const KisFilterSP KisFilterHandler::filter() const
{
    return m_d->filter;
}

void KisFilterHandler::areaDone(const QRect & rc)
{
    m_d->dev->setDirty(rc); // Starts computing the projection for the area we've done.
    m_d->view->document()->setModified(true);
}

#include "kis_filter_handler.moc"
