/*
 * shearimage.h -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Michael Thaler (michael.thaler@physik.tu-muenchen.de)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SHEARIMAGE_H
#define SHEARIMAGE_H

#include <QVariant>

#include <KisActionPlugin.h>
#include "kis_types.h"


class ShearImage : public KisActionPlugin
{
    Q_OBJECT
public:
    ShearImage(QObject *parent, const QVariantList &);
    ~ShearImage() override;

private:
    void shearLayerImpl(KisNodeSP rootNode);

private Q_SLOTS:

    void slotShearImage();
    void slotShearLayer();
    void slotShearAllLayers();
};

#endif // SHEARIMAGE_H
