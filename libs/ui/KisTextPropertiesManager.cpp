/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisTextPropertiesManager.h"
#include <KoSelectedShapesProxy.h>
#include <kis_canvas_resource_provider.h>
#include <KoSvgTextPropertyData.h>
#include <KoSelection.h>
#include <KoSvgTextShape.h>

#include <KisView.h>
#include <kis_canvas2.h>

#include <QPointer>

struct KisTextPropertiesManager::Private {
    KisCanvasResourceProvider *provider;
    QPointer<KisView> view {0};
};

KisTextPropertiesManager::KisTextPropertiesManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

KisTextPropertiesManager::~KisTextPropertiesManager()
{
}

void KisTextPropertiesManager::setView(QPointer<KisView> imageView)
{
    if (d->view) {
        disconnect(d->view->canvasBase()->selectedShapesProxy(), SIGNAL(selectionChanged()), this, SLOT(slotShapeSelectionChanged()));
    }

    d->view = imageView;
    if (d->view) {
        connect(d->view->canvasBase()->selectedShapesProxy(), SIGNAL(selectionChanged()), this, SLOT(slotShapeSelectionChanged()));
    }
}

void KisTextPropertiesManager::setCanvasResourceProvider(KisCanvasResourceProvider *provider)
{
    d->provider = provider;
}

void KisTextPropertiesManager::slotShapeSelectionChanged()
{
    if (!d->view
            || !d->view->canvasBase()
            || !d->view->canvasBase()->selectedShapesProxy()
            || !d->view->canvasBase()->selectedShapesProxy()->selection()
            || !d->provider) return;

    KoSvgTextPropertyData textData;

    const QList<KoShape*> shapes = d->view->canvasBase()->selectedShapesProxy()->selection()->selectedEditableShapes();
    QList<KoSvgTextProperties> props;
    QSet<KoSvgTextProperties::PropertyId> propIds;

    for (auto it = shapes.begin(); it != shapes.end(); it++) {
        KoSvgTextShape *textShape = dynamic_cast<KoSvgTextShape*>(*it);
        if (!textShape) continue;
        KoSvgTextProperties p = textShape->textProperties();
        props.append(p);
        for (int i = 0; i < p.properties().size(); i++) {
            propIds.insert(p.properties().at(i));
        }
    }

    if (props.isEmpty()) return;

    // Find common properties, and mark all non-common properties as tristate.

    textData.commonProperties = props.first();

    for (auto it = props.begin(); it != props.end(); it++) {
        for (int i = 0; i < propIds.size(); i++) {
            KoSvgTextProperties::PropertyId p = propIds.values().at(i);
            if (it->hasProperty(p)) {
                if (textData.commonProperties.property(p) != it->property(p)) {
                    textData.commonProperties.removeProperty(p);
                    textData.tristate.insert(p);
                }
            } else {
                textData.commonProperties.removeProperty(p);
                textData.tristate.insert(p);
            }
        }
    }

    qDebug() << "Setting text data on canvas resources";
    qDebug() << textData;

    d->provider->setTextPropertyData(textData);
}
