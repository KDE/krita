/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "TransformMask.h"
#include <kis_transform_mask.h>
#include <kis_image.h>
#include <kis_transform_mask_params_interface.h>

#include "Node.h"


TransformMask::TransformMask(KisImageSP image, QString name, QObject *parent) :
    Node(image, new KisTransformMask(image, name), parent)
{

}

TransformMask::TransformMask(KisImageSP image, KisTransformMaskSP mask, QObject *parent):
    Node(image, mask, parent)
{

}

TransformMask::~TransformMask()
{

}

QString TransformMask::type() const
{
    return "transformmask";
}

QTransform TransformMask::finalAffineTransform() const
{
    QTransform affineTransformation;
    KisTransformMask *mask = dynamic_cast<KisTransformMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mask, QTransform());

    affineTransformation = mask->transformParams()->finalAffineTransform();

    return affineTransformation;
}
