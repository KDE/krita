/*
 *  SPDX-FileCopyrightText: 2019 Tusooa Zhu <tusooa@vista.aero>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDlgChangeCloneSource.h"

#include <kis_clone_layer.h>
#include <kis_image.h>
#include <kis_undo_adapter.h>
#include <kis_processing_applicator.h>
#include <KisImageSignals.h>
#include <kis_signals_blocker.h>

#include "KisViewManager.h"
#include "KisChangeCloneLayersCommand.h"

struct KisDlgChangeCloneSource::Private
{
    Private(QList<KisCloneLayerSP> layers, KisViewManager *view)
        : cloneLayers(layers)
        , view(view)
        , image(view->image())
        , applicator(new KisProcessingApplicator(image, 0,
                                                 KisProcessingApplicator::NONE,
                                                 /* emitSignals = */ KisImageSignalVector(),
                                                 kundo2_i18n("Change Clone Layers")))
        , modified(false) {}

    QList<KisCloneLayerSP> cloneLayers;
    KisViewManager *view;
    KisImageSP image;
    QList<KisLayerSP> validTargets;
    Ui::WdgChangeCloneSource ui;
    QScopedPointer<KisProcessingApplicator> applicator;
    bool modified;

    void addToTargetListRecursively(KisNodeSP node, bool addSelf = true);
    void filterOutAncestorsAndClonesRecursively(KisLayerSP layer);
    KisLayerSP getSelectedTargetLayer();
    KUndo2Command *createCommand(KisLayerSP targetLayer);
};

void KisDlgChangeCloneSource::Private::addToTargetListRecursively(KisNodeSP node, bool addSelf)
{
    if (!node) {
        return;
    }
    if (addSelf) {
        KisLayerSP layer(qobject_cast<KisLayer *>(node.data()));
        if (layer) {
            validTargets << layer;
        }
    }
    for (KisNodeSP childNode = node->lastChild(); childNode; childNode = childNode->prevSibling()) {
        KisLayerSP childLayer(qobject_cast<KisLayer *>(childNode.data()));
        if (childLayer) {
            addToTargetListRecursively(childLayer);
        }
    }

}

void KisDlgChangeCloneSource::Private::filterOutAncestorsAndClonesRecursively(KisLayerSP layer)
{
    validTargets.removeOne(layer);
    // remove `layer` and its ancestors
    KisLayerSP parent = qobject_cast<KisLayer *>(layer->parent().data());
    if (parent) {
        filterOutAncestorsAndClonesRecursively(parent);
    }

    // remove all clones of `layer`, and their ancestors
    Q_FOREACH (KisCloneLayerSP clone, layer->registeredClones()) {
        filterOutAncestorsAndClonesRecursively(clone);
    }
}

KisLayerSP KisDlgChangeCloneSource::Private::getSelectedTargetLayer()
{
    int index = ui.cmbSourceLayer->currentIndex();
    if (index != -1) {
        return validTargets.at(index);
    } else {
        return 0;
    }
}

KUndo2Command *KisDlgChangeCloneSource::Private::createCommand(KisLayerSP targetLayer)
{
    return new KisChangeCloneLayersCommand(cloneLayers, targetLayer);
}

KisDlgChangeCloneSource::KisDlgChangeCloneSource(QList<KisCloneLayerSP> layers, KisViewManager *view, QWidget *parent)
    : KoDialog(parent)
    , d(new Private(layers, view))
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!layers.isEmpty());

    connect(d->image.data(), &KisImage::sigStrokeCancellationRequested,
            this, &KisDlgChangeCloneSource::reject);
    connect(d->image.data(), &KisImage::sigUndoDuringStrokeRequested,
            this, &KisDlgChangeCloneSource::reject);
    connect(d->image.data(), &KisImage::sigStrokeEndRequested,
            this, &KisDlgChangeCloneSource::accept);

    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    QWidget *widget = new QWidget(this);
    d->ui.setupUi(widget);
    setMainWidget(widget);

    connect(d->ui.cmbSourceLayer, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &KisDlgChangeCloneSource::slotCancelChangesAndSetNewTarget);

    updateTargetLayerList();
}

KisDlgChangeCloneSource::~KisDlgChangeCloneSource()
{
    dbgUI << "dialog destroyed";
    if (d->applicator) {
        if (result() == QDialog::Accepted && d->modified) {
            dbgUI << "Accepted";
            d->applicator->end();
        } else {
            dbgUI << "Rejected";
            d->applicator->cancel();
        }
    }
}

void KisDlgChangeCloneSource::updateTargetLayerList()
{
    KisSignalsBlocker b(d->ui.cmbSourceLayer);

    if (!d->image) {
        return;
    }
    KisNodeSP root = d->image->root();
    d->validTargets.clear();
    d->addToTargetListRecursively(root, /* addSelf = */ false);

    KisLayerSP commonCopyFrom(d->cloneLayers.first()->copyFrom());

    Q_FOREACH (KisCloneLayerSP clone, d->cloneLayers) {
        // filter out invalid targets:
        // selected clone layers, their ancestors;
        // the clone layers' registered clone, the clones' ancestors.
        d->filterOutAncestorsAndClonesRecursively(clone);

        // assume that clone->copyFrom() != 0
        if (clone->copyFrom() != commonCopyFrom) {
            commonCopyFrom = 0;
        }
    }

    d->ui.cmbSourceLayer->clear();
    Q_FOREACH (KisNodeSP node, d->validTargets) {
        d->ui.cmbSourceLayer->addItem(node->name());
    }

    if (commonCopyFrom) {
        d->ui.cmbSourceLayer->setCurrentIndex(d->validTargets.indexOf(commonCopyFrom));
    } else {
        d->ui.cmbSourceLayer->setCurrentIndex(-1);
    }
}

void KisDlgChangeCloneSource::slotCancelChangesAndSetNewTarget()
{
    KisLayerSP targetLayer = d->getSelectedTargetLayer();
    if (targetLayer) {
        d->applicator->applyCommand(d->createCommand(targetLayer));
        d->modified = true;
    }
}

