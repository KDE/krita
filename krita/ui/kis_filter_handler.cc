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
#include <recorder/kis_node_query_path.h>
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
#include "kis_filter_job.h"
#include "filter/kis_filter_registry.h"
#include "kis_system_locker.h"
#include "kis_progress_widget.h"

#include "strokes/kis_filter_stroke_strategy.h"
#include "krita_utils.h"


struct KisFilterHandler::Private {

    Private(KisFilterHandler *_q)
            : view(0)
            , manager(0)
            , q(_q)
    {
    }

    ~Private() {
    }

    KisFilterSP filter;

    KisView2* view;
    KisFilterManager* manager;

    KisSafeFilterConfigurationSP lastConfiguration;
    KisFilterHandler *q;

    void saveConfiguration(KisFilterSP filter, KisSafeFilterConfigurationSP filterConfig);
};

void KisFilterHandler::Private::saveConfiguration(KisFilterSP filter, KisSafeFilterConfigurationSP filterConfig)
{
    if (filter->bookmarkManager()) {
        filter->bookmarkManager()->save(KisBookmarkedConfigurationManager::ConfigLastUsed.id(),
                                        filterConfig.data());
    }

    lastConfiguration = filterConfig;
    manager->setLastFilterHandler(q);
}


KisFilterHandler::KisFilterHandler(KisFilterManager* parent, KisFilterSP f, KisView2* view)
        : QObject(parent)
        , m_d(new Private(this))
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
    /**
     * HACK ALERT:
     * Until filters are ported to strokes, there should be a barrier
     * to finish all the running strokes in the system
     */
    m_d->view->image()->barrierLock();
    m_d->view->image()->unlock();

    KisPaintDeviceSP dev = m_d->view->activeNode()->paintDevice();
    if (!dev) {
        qWarning() << "KisFilterHandler::showDialog(): Filtering was requested for illegal active layer!" << m_d->view->activeNode();
        return;
    }

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

    if (m_d->filter->showConfigurationWidget()) {
        KisFilterDialog* dialog = new KisFilterDialog(m_d->view , m_d->view->activeNode(), m_d->view->image(), m_d->view->selection());
        dialog->setFilter(m_d->filter);
        connect(dialog, SIGNAL(sigPleaseApplyFilter(KisSafeFilterConfigurationSP)),
                SLOT(apply(KisSafeFilterConfigurationSP)));
        dialog->setVisible(true);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
    } else {
        apply(KisSafeFilterConfigurationSP(m_d->filter->defaultConfiguration(m_d->view->activeNode()->original())));
    }
}

void KisFilterHandler::reapply()
{
    apply(m_d->lastConfiguration);
}

void KisFilterHandler::apply(KisSafeFilterConfigurationSP config)
{
    KisSafeFilterConfigurationSP filterConfig(config);

    KisImageWSP image = m_d->view->image();
    KisPostExecutionUndoAdapter *undoAdapter =
        image->postExecutionUndoAdapter();
    KoCanvasResourceManager *resourceManager =
        m_d->view->canvasBase()->resourceManager();

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image,
                                 undoAdapter,
                                 resourceManager);

    KisStrokeId strokeId =
        image->startStroke(new KisFilterStrokeStrategy(m_d->filter,
                                                       KisSafeFilterConfigurationSP(filterConfig),
                                                       resources));

    if (m_d->filter->supportsThreading()) {
        QSize size = KritaUtils::optimalPatchSize();
        QVector<QRect> rects = KritaUtils::splitRectIntoPatches(image->bounds(), size);

        foreach(const QRect &rc, rects) {
            image->addJob(strokeId,
                          new KisFilterStrokeStrategy::Data(rc, true));
        }
    } else {
        image->addJob(strokeId,
                      new KisFilterStrokeStrategy::Data(image->bounds(), false));
    }

    image->endStroke(strokeId);

    m_d->saveConfiguration(m_d->filter, filterConfig);
}

const QString KisFilterHandler::filterName() const
{
    return m_d->filter->name();
}

