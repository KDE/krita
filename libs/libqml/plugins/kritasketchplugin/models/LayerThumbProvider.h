/*
    SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef LAYERTHUMBPROVIDER_H
#define LAYERTHUMBPROVIDER_H

#include <QQuickImageProvider>

class LayerModel;

class LayerThumbProvider : public QQuickImageProvider
{

public:
    LayerThumbProvider();
    virtual ~LayerThumbProvider();
    virtual QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize);

    void setLayerModel(LayerModel* model);
    void setLayerID(int id);
    int layerID() const;
private:
    class Private;
    Private* d;
};

#endif // LAYERTHUMBPROVIDER_H
