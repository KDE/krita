/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "SelectionMask.h"
#include <kis_selection_mask.h>
#include <kis_image.h>
#include "Selection.h"
#include <kis_selection.h>

SelectionMask::SelectionMask(KisImageSP image, QString name, QObject *parent) :
    Node(image, new KisSelectionMask(image), parent)
{
    this->node()->setName(name);
}

SelectionMask::SelectionMask(KisImageSP image, KisSelectionMaskSP mask, QObject *parent):
    Node(image, mask, parent)
{

}

SelectionMask::~SelectionMask()
{

}

Selection *SelectionMask::selection() const
{
    const KisSelectionMask *mask = qobject_cast<const KisSelectionMask*>(this->node());
    return new Selection(mask->selection());
}

void SelectionMask::setSelection(Selection *selection)
{
    KisSelectionMask *mask = dynamic_cast<KisSelectionMask*>(this->node().data());
    mask->setSelection(selection->selection());
}

QString SelectionMask::type() const
{
    return "selectionmask";
}
