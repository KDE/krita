/*
    SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "LayerThumbProvider.h"
#include "LayerModel.h"
#include <KisViewManager.h>
#include <kis_image.h>

class LayerThumbProvider::Private {
public:
    Private()
        : layerModel(0)
        , id(0)
    {};
    LayerModel* layerModel;
    int id;
};

LayerThumbProvider::LayerThumbProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
    , d(new Private)
{
}

LayerThumbProvider::~LayerThumbProvider()
{
    delete d;
}

QImage LayerThumbProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    Q_UNUSED(size);
    Q_UNUSED(requestedSize);
    if (id.split("/").first() == QLatin1String("fullimage")) {
        KisViewManager* view = qobject_cast<KisViewManager*>(d->layerModel->view());
        int width = 300 * ((float)view->image()->bounds().width() / (float)view->image()->bounds().height());
        return view->image()->convertToQImage(QSize(width, 300), view->image()->profile());
    }
    return d->layerModel->layerThumbnail(id);
}

void LayerThumbProvider::setLayerModel(LayerModel* model)
{
    d->layerModel = model;
}

void LayerThumbProvider::setLayerID(int id)
{
    d->id = id;
}

int LayerThumbProvider::layerID() const
{
    return d->id;
}
