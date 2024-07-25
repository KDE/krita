/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "TransparencyMask.h"
#include <kis_transparency_mask.h>
#include <kis_image.h>
#include "Selection.h"
#include <kis_selection.h>
#include <kis_group_layer.h>
#include "kis_layer.h"

TransparencyMask::TransparencyMask(KisImageSP image, QString name, QObject *parent) :
    Node(image, new KisTransparencyMask(image, name), parent)
{
    KisTransparencyMask *mask = qobject_cast<KisTransparencyMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(mask);

    KisSelectionSP selection = new KisSelection();
    KisLayerSP layer = qobject_cast<KisLayer*>(image->rootLayer().data());

    mask->initSelection(selection, layer);
}

TransparencyMask::TransparencyMask(KisImageSP image, KisTransparencyMaskSP mask, QObject *parent):
    Node(image, mask, parent)
{

}

TransparencyMask::~TransparencyMask()
{

}

Selection *TransparencyMask::selection() const
{
    const KisTransparencyMask *mask = qobject_cast<const KisTransparencyMask*>(this->node());
    return new Selection(mask->selection());
}

void TransparencyMask::setSelection(Selection *selection)
{
    KisTransparencyMask *mask = qobject_cast<KisTransparencyMask*>(this->node().data());
    mask->setSelection(selection->selection());
}

QString TransparencyMask::type() const
{
    return "transparencymask";
}
