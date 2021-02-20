/*
 * rotateimage.h -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Michael Thaler (michael.thaler@physik.tu-muenchen.de)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ROTATEIMAGE_H
#define ROTATEIMAGE_H

#include <QVariant>

#include <KisActionPlugin.h>
#include "kis_types.h"

class RotateImage : public KisActionPlugin
{
    Q_OBJECT
public:
    RotateImage(QObject *parent, const QVariantList &);
    ~RotateImage() override;

private:
    void rotateLayerCustomImpl(KisNodeSP rootNode);
    void rotateLayerImpl(KisNodeSP rootNode, qreal radians);

private Q_SLOTS:

    void slotRotateImage();
    void slotRotateImage90();
    void slotRotateImage180();
    void slotRotateImage270();
    void slotMirrorImageVertical();
    void slotMirrorImageHorizontal();
    void slotRotateLayer();
    void slotRotateLayerCW90();
    void slotRotateLayerCCW90();
    void slotRotateLayer180();
    void slotRotateAllLayers();
    void slotRotateAllLayersCW90();
    void slotRotateAllLayersCCW90();
    void slotRotateAllLayers180();
};

#endif // ROTATEIMAGE_H
