/*
 * imagesize.h -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef IMAGESIZE_H
#define IMAGESIZE_H

#include <QVariant>

#include <KisActionPlugin.h>
#include "kis_types.h"

class ImageSize : public KisActionPlugin
{
    Q_OBJECT
public:
    ImageSize(QObject *parent, const QVariantList &);
    ~ImageSize() override;

private:
    void scaleLayerImpl(KisNodeSP rootNode);

private Q_SLOTS:

    void slotImageSize();
    void slotCanvasSize();
    void slotLayerSize();
    void slotSelectionScale();

    void slotScaleAllLayers();
};

#endif // IMAGESIZE_H
