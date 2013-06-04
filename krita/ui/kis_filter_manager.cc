/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_filter_manager.h"

#include "kis_filter_manager.moc"

#include <QHash>
#include <QSignalMapper>

#include <kmessagebox.h>
#include <kactionmenu.h>
#include <kactioncollection.h>

#include <KoID.h>

// krita/image
#include <filter/kis_filter.h>
#include <filter/kis_filter_registry.h>
#include <filter/kis_filter_configuration.h>
#include <kis_paint_device.h>

// krita/ui
#include "kis_view2.h"
#include "kis_canvas2.h"
#include <kis_bookmarked_configuration_manager.h>
#include <KoColorSpaceRegistry.h>

#include "dialogs/kis_dlg_filter.h"
#include "strokes/kis_filter_stroke_strategy.h"
#include "krita_utils.h"


struct KisFilterManager::Private {
    Private() : reapplyAction(0), actionCollection(0) {

    }
    KAction* reapplyAction;
    QHash<QString, KActionMenu*> filterActionMenus;
    QHash<KisFilter*, KAction*> filters2Action;
    KActionCollection *actionCollection;
    KisView2 *view;




    KisSafeFilterConfigurationSP lastConfiguration;
    KisSafeFilterConfigurationSP currentlyAppliedCofiguration;
    KisStrokeId currentStrokeId;

    QSignalMapper actionsMapper;
};

KisFilterManager::KisFilterManager(KisView2 * view, KisDoc2 * doc) : d(new Private)
{
    Q_UNUSED(doc);
    d->view = view;
}

KisFilterManager::~KisFilterManager()
{
    delete d;
}

void KisFilterManager::setup(KActionCollection * ac)
{
    d->actionCollection = ac;

    // Setup reapply action
    d->reapplyAction = new KAction(i18n("Apply Filter Again"), this);
    d->actionCollection->addAction("filter_apply_again", d->reapplyAction);

    d->reapplyAction->setEnabled(false);
    connect(d->reapplyAction, SIGNAL(triggered()), SLOT(reapplyLastFilter()));

    connect(&d->actionsMapper, SIGNAL(mapped(const QString&)), SLOT(showFilterDialog(const QString&)));

    // Setup list of filters
    foreach (const QString &filterName, KisFilterRegistry::instance()->keys()) {
        insertFilter(filterName);
    }

    connect(KisFilterRegistry::instance(), SIGNAL(filterAdded(QString)), SLOT(insertFilter(const QString &)));
}

void KisFilterManager::insertFilter(const QString & filterName)
{
    Q_ASSERT(d->actionCollection);

    KisFilterSP filter = KisFilterRegistry::instance()->value(filterName);
    Q_ASSERT(filter);

    if (d->filters2Action.keys().contains(filter.data())) {
        warnKrita << "Filter" << filterName << " has already been inserted";
        return;
    }

    KoID category = filter->menuCategory();
    KActionMenu* actionMenu = d->filterActionMenus[ category.id()];
    if (!actionMenu) {
        actionMenu = new KActionMenu(category.name(), this);
        d->actionCollection->addAction(category.id(), actionMenu);
        d->filterActionMenus[category.id()] = actionMenu;
    }

    KAction *action = new KAction(filter->menuEntry(), this);
    action->setShortcut(filter->shortcut(), KAction::DefaultShortcut);
    d->actionCollection->addAction(QString("krita_filter_%1").arg(filterName), action);
    d->filters2Action[filter.data()] = action;

    actionMenu->addAction(action);

    d->actionsMapper.setMapping(action, filterName);
    connect(action, SIGNAL(triggered()), &d->actionsMapper, SLOT(map()));
}

void KisFilterManager::updateGUI()
{
    if (!d->view) return;

    bool enable = false;

    KisNodeSP activeNode = d->view->activeNode();
    enable = activeNode && activeNode->paintDevice() && activeNode->isEditable();

    d->reapplyAction->setEnabled(enable);

    for (QHash<KisFilter*, KAction*>::iterator it = d->filters2Action.begin();
            it != d->filters2Action.end(); ++it) {

        bool localEnable = enable &&
            it.key()->workWith(activeNode->paintDevice()->compositionSourceColorSpace());

        it.value()->setEnabled(localEnable);
    }
}

void KisFilterManager::reapplyLastFilter()
{
    if (!d->lastConfiguration) return;

    apply(d->lastConfiguration);
    finish();
}

void KisFilterManager::showFilterDialog(const QString &filterId)
{
    /**
     * The UI should show only after every running stroke is finished,
     * so the barrier is added here.
     */
    d->view->image()->barrierLock();
    d->view->image()->unlock();

    KisPaintDeviceSP dev = d->view->activeNode()->paintDevice();
    if (!dev) {
        qWarning() << "KisFilterManager::showFilterDialog(): Filtering was requested for illegal active layer!" << d->view->activeNode();
        return;
    }

    KisFilterSP filter = KisFilterRegistry::instance()->value(filterId);

    if (dev->colorSpace()->willDegrade(filter->colorSpaceIndependence())) {
        // Warning bells!
        if (filter->colorSpaceIndependence() == TO_LAB16) {
            if (KMessageBox::warningContinueCancel(d->view,
                                                   i18n("The %1 filter will convert your %2 data to 16-bit L*a*b* and vice versa. ",
                                                        filter->name(),
                                                        dev->colorSpace()->name()),
                                                   i18n("Filter Will Convert Your Layer Data"),
                                                   KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                                   "lab16degradation") != KMessageBox::Continue) return;

        } else if (filter->colorSpaceIndependence() == TO_RGBA16) {
            if (KMessageBox::warningContinueCancel(d->view,
                                                   i18n("The %1 filter will convert your %2 data to 16-bit RGBA and vice versa. ",
                                                        filter->name() , dev->colorSpace()->name()),
                                                   i18n("Filter Will Convert Your Layer Data"),
                                                   KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                                   "rgba16degradation") != KMessageBox::Continue) return;
        }
    }

    if (filter->showConfigurationWidget()) {
        KisFilterDialog* dialog = new KisFilterDialog(d->view , d->view->activeNode(), this);
        dialog->setFilter(filter);
        dialog->setVisible(true);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
    } else {
        apply(KisSafeFilterConfigurationSP(filter->defaultConfiguration(d->view->activeNode()->original())));
        finish();
    }
}

void KisFilterManager::apply(KisSafeFilterConfigurationSP filterConfig)
{
    KisFilterSP filter = KisFilterRegistry::instance()->value(filterConfig->name());
    KisImageWSP image = d->view->image();

    if (d->currentStrokeId) {
        image->cancelStroke(d->currentStrokeId);
    }

    KisPostExecutionUndoAdapter *undoAdapter =
        image->postExecutionUndoAdapter();
    KoCanvasResourceManager *resourceManager =
        d->view->canvasBase()->resourceManager();

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image,
                                 undoAdapter,
                                 resourceManager);

    d->currentStrokeId =
        image->startStroke(new KisFilterStrokeStrategy(filter,
                                                       KisSafeFilterConfigurationSP(filterConfig),
                                                       resources));

    if (filter->supportsThreading()) {
        QSize size = KritaUtils::optimalPatchSize();
        QVector<QRect> rects = KritaUtils::splitRectIntoPatches(image->bounds(), size);

        foreach(const QRect &rc, rects) {
            image->addJob(d->currentStrokeId,
                          new KisFilterStrokeStrategy::Data(rc, true));
        }
    } else {
        image->addJob(d->currentStrokeId,
                      new KisFilterStrokeStrategy::Data(image->bounds(), false));
    }

    d->currentlyAppliedCofiguration = filterConfig;
}

void KisFilterManager::finish()
{
    Q_ASSERT(d->currentStrokeId);

    d->view->image()->endStroke(d->currentStrokeId);

    KisFilterSP filter = KisFilterRegistry::instance()->value(d->currentlyAppliedCofiguration->name());
    if (filter->bookmarkManager()) {
        filter->bookmarkManager()->save(KisBookmarkedConfigurationManager::ConfigLastUsed.id(),
                                       d->currentlyAppliedCofiguration.data());
    }

    d->lastConfiguration = d->currentlyAppliedCofiguration;
    d->reapplyAction->setEnabled(true);
    d->reapplyAction->setText(i18n("Apply Filter Again: %1", filter->name()));

    d->currentStrokeId.clear();
    d->currentlyAppliedCofiguration.clear();
}

void KisFilterManager::cancel()
{
    Q_ASSERT(d->currentStrokeId);

    d->view->image()->cancelStroke(d->currentStrokeId);

    d->currentStrokeId.clear();
    d->currentlyAppliedCofiguration.clear();
}

bool KisFilterManager::isStrokeRunning() const
{
    return d->currentStrokeId;
}
