/*
 *  Copyright (c) 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_MERGE_LABELED_LAYERS_H
#define __KIS_MERGE_LABELED_LAYERS_H


#include "kundo2command.h"
#include "kritaimage_export.h"
#include "kis_types.h"

class KisUpdatesFacade;


class KRITAIMAGE_EXPORT KisMergeLabeledLayersCommand : public KUndo2Command
{
public:
    KisMergeLabeledLayersCommand(KisImageSP refImage, KisPaintDeviceSP refPaintDevice, KisNodeSP currentRoot);
    ~KisMergeLabeledLayersCommand() override;

    void undo() override;
    void redo() override;

private:
    void mergeLabeledLayers();

private:
    KisImageSP m_refImage;
    KisPaintDeviceSP m_refPaintDevice;
    KisNodeSP m_currentRoot;
};

#endif /* __KIS_MERGE_LABELED_LAYERS_H */
