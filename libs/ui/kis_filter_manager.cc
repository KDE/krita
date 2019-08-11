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


#include <QHash>
#include <QSignalMapper>

#include <QMessageBox>
#include <kactionmenu.h>
#include <kactioncollection.h>

#include <KoID.h>
#include <KisMainWindow.h>

// krita/image
#include <filter/kis_filter.h>
#include <filter/kis_filter_registry.h>
#include <filter/kis_filter_configuration.h>
#include <kis_paint_device.h>

// krita/ui
#include "KisViewManager.h"
#include "kis_canvas2.h"
#include <kis_bookmarked_configuration_manager.h>

#include "kis_action.h"
#include "kis_action_manager.h"
#include "kis_canvas_resource_provider.h"
#include "dialogs/kis_dlg_filter.h"
#include "strokes/kis_filter_stroke_strategy.h"
#include "krita_utils.h"
#include "kis_icon_utils.h"


struct KisFilterManager::Private {
    Private()
        : reapplyAction(0)
        , actionCollection(0)
        , actionManager(0)
        , view(0)
    {
    }
    KisAction* reapplyAction;
    QHash<QString, KActionMenu*> filterActionMenus;
    QHash<KisFilter*, QAction *> filters2Action;
    KActionCollection *actionCollection;
    KisActionManager *actionManager;
    KisViewManager *view;

    KisFilterConfigurationSP lastConfiguration;
    KisFilterConfigurationSP currentlyAppliedConfiguration;
    KisStrokeId currentStrokeId;
    QRect initialApplyRect;

    QSignalMapper actionsMapper;

    QPointer<KisDlgFilter> filterDialog;
};

KisFilterManager::KisFilterManager(KisViewManager * view)
    : d(new Private)
{
    d->view = view;
}

KisFilterManager::~KisFilterManager()
{
    delete d;
}

void KisFilterManager::setView(QPointer<KisView>imageView)
{
    Q_UNUSED(imageView);
}


void KisFilterManager::setup(KActionCollection * ac, KisActionManager *actionManager)
{
    d->actionCollection = ac;
    d->actionManager = actionManager;

    // Setup reapply action
    d->reapplyAction = d->actionManager->createAction("filter_apply_again");
    d->reapplyAction->setActivationFlags(KisAction::ACTIVE_DEVICE);
    d->reapplyAction->setEnabled(false);
    connect(d->reapplyAction, SIGNAL(triggered()), SLOT(reapplyLastFilter()));

    connect(&d->actionsMapper, SIGNAL(mapped(QString)), SLOT(showFilterDialog(QString)));

    // Setup list of filters
    QStringList keys = KisFilterRegistry::instance()->keys();
    keys.sort();
    Q_FOREACH (const QString &filterName, keys) {
        insertFilter(filterName);
    }

    connect(KisFilterRegistry::instance(), SIGNAL(filterAdded(QString)), SLOT(insertFilter(QString)));
}

void KisFilterManager::insertFilter(const QString & filterName)
{
    Q_ASSERT(d->actionCollection);

    KisFilterSP filter = KisFilterRegistry::instance()->value(filterName);
    Q_ASSERT(filter);

    if (d->filters2Action.contains(filter.data())) {
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

    KisAction *action = new KisAction(filter->menuEntry(), this);
    action->setDefaultShortcut(filter->shortcut());
    action->setActivationFlags(KisAction::ACTIVE_DEVICE);

    d->actionManager->addAction(QString("krita_filter_%1").arg(filterName), action);
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
    enable = activeNode && activeNode->hasEditablePaintDevice();

    d->reapplyAction->setEnabled(enable);

    for (QHash<KisFilter*, QAction *>::iterator it = d->filters2Action.begin();
            it != d->filters2Action.end(); ++it) {

        bool localEnable = enable;

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
    if (!d->view->activeNode()->isEditable()) {
        d->view->showFloatingMessage(i18n("Cannot apply filter to locked layer."),
                                      KisIconUtils::loadIcon("object-locked"));
        return;
    }

    if (d->filterDialog && d->filterDialog->isVisible()) {
        KisFilterSP filter = KisFilterRegistry::instance()->value(filterId);
        d->filterDialog->setFilter(filter);
        return;
    }

    connect(d->view->image(),
            SIGNAL(sigStrokeCancellationRequested()),
            SLOT(slotStrokeCancelRequested()),
            Qt::UniqueConnection);

    connect(d->view->image(),
            SIGNAL(sigStrokeEndRequested()),
            SLOT(slotStrokeEndRequested()),
            Qt::UniqueConnection);

    /**
     * The UI should show only after every running stroke is finished,
     * so a virtual barrier is added here.
     */
    if (!d->view->blockUntilOperationsFinished(d->view->image())) {
        return;
    }

    Q_ASSERT(d->view);
    Q_ASSERT(d->view->activeNode());

    KisPaintDeviceSP dev = d->view->activeNode()->paintDevice();
    if (!dev) {
        warnKrita << "KisFilterManager::showFilterDialog(): Filtering was requested for illegal active layer!" << d->view->activeNode();
        return;
    }

    KisFilterSP filter = KisFilterRegistry::instance()->value(filterId);

    if (dev->colorSpace()->willDegrade(filter->colorSpaceIndependence())) {
        // Warning bells!
        if (filter->colorSpaceIndependence() == TO_LAB16) {
            if (QMessageBox::warning(d->view->mainWindow(),
                                     i18nc("@title:window", "Krita"),
                                     i18n("The %1 filter will convert your %2 data to 16-bit L*a*b* and vice versa. ",
                                          filter->name(),
                                          dev->colorSpace()->name()),
                                     QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok)
                    != QMessageBox::Ok) return;

        } else if (filter->colorSpaceIndependence() == TO_RGBA16) {
            if (QMessageBox::warning(d->view->mainWindow(),
                                     i18nc("@title:window", "Krita"),
                                     i18n("The %1 filter will convert your %2 data to 16-bit RGBA and vice versa. ",
                                          filter->name() , dev->colorSpace()->name()),
                                     QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok)
                    != QMessageBox::Ok) return;
        }
    }

    if (filter->showConfigurationWidget()) {
        if (!d->filterDialog) {
            d->filterDialog = new KisDlgFilter(d->view , d->view->activeNode(), this, d->view->mainWindow());
            d->filterDialog->setAttribute(Qt::WA_DeleteOnClose);
        }
        d->filterDialog->setFilter(filter);
        d->filterDialog->setVisible(true);
    } else {
        apply(KisFilterConfigurationSP(filter->defaultConfiguration()));
        finish();
    }
}

void KisFilterManager::apply(KisFilterConfigurationSP filterConfig)
{
    KisFilterSP filter = KisFilterRegistry::instance()->value(filterConfig->name());
    KisImageWSP image = d->view->image();

    if (d->currentStrokeId) {
        image->addJob(d->currentStrokeId, new KisFilterStrokeStrategy::CancelSilentlyMarker);
        image->cancelStroke(d->currentStrokeId);
        d->currentStrokeId.clear();
    } else {
        image->waitForDone();
        d->initialApplyRect = d->view->activeNode()->exactBounds();
    }

    QRect applyRect = d->initialApplyRect;

    KisPaintDeviceSP paintDevice = d->view->activeNode()->paintDevice();
    if (paintDevice &&
        filter->needsTransparentPixels(filterConfig.data(), paintDevice->colorSpace())) {

        applyRect |= image->bounds();
    }

    KoCanvasResourceProvider *resourceManager =
        d->view->canvasResourceProvider()->resourceManager();

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image,
                                 d->view->activeNode(),
                                 resourceManager);

    d->currentStrokeId =
        image->startStroke(new KisFilterStrokeStrategy(filter,
                                                       KisFilterConfigurationSP(filterConfig),
                                                       resources));

    QRect processRect = filter->changedRect(applyRect, filterConfig.data(), 0);
    processRect &= image->bounds();

    if (filter->supportsThreading()) {
        QSize size = KritaUtils::optimalPatchSize();
        QVector<QRect> rects = KritaUtils::splitRectIntoPatches(processRect, size);

        Q_FOREACH (const QRect &rc, rects) {
            image->addJob(d->currentStrokeId,
                          new KisFilterStrokeStrategy::Data(rc, true));
        }
    } else {
        image->addJob(d->currentStrokeId,
                      new KisFilterStrokeStrategy::Data(processRect, false));
    }

    d->currentlyAppliedConfiguration = filterConfig;
}

void KisFilterManager::finish()
{
    Q_ASSERT(d->currentStrokeId);

    d->view->image()->endStroke(d->currentStrokeId);

    KisFilterSP filter = KisFilterRegistry::instance()->value(d->currentlyAppliedConfiguration->name());
    if (filter->bookmarkManager()) {
        filter->bookmarkManager()->save(KisBookmarkedConfigurationManager::ConfigLastUsed,
                                       d->currentlyAppliedConfiguration.data());
    }

    d->lastConfiguration = d->currentlyAppliedConfiguration;
    d->reapplyAction->setEnabled(true);
    d->reapplyAction->setText(i18n("Apply Filter Again: %1", filter->name()));

    d->currentStrokeId.clear();
    d->currentlyAppliedConfiguration.clear();
}

void KisFilterManager::cancel()
{
    Q_ASSERT(d->currentStrokeId);

    d->view->image()->cancelStroke(d->currentStrokeId);

    d->currentStrokeId.clear();
    d->currentlyAppliedConfiguration.clear();
}

bool KisFilterManager::isStrokeRunning() const
{
    return d->currentStrokeId;
}

void KisFilterManager::slotStrokeEndRequested()
{
    if (d->currentStrokeId && d->filterDialog) {
        d->filterDialog->accept();
    }
}

void KisFilterManager::slotStrokeCancelRequested()
{
    if (d->currentStrokeId && d->filterDialog) {
        d->filterDialog->reject();
    }
}
