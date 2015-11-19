/*
    Copyright (C) 2012  Dan Leinir Turthra Jensen <admin@leinir.dk>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
