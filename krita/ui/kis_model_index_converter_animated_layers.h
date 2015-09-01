/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef _KIS_MODEL_INDEX_CONVERTER_ANIMATER_LAYERS_H_
#define _KIS_MODEL_INDEX_CONVERTER_ANIMATER_LAYERS_H_

#include "kis_model_index_converter_base.h"

class KisDummiesFacadeBase;
class KisNodeModel;

class KRITAUI_EXPORT KisModelIndexConverterAnimatedLayers : public KisModelIndexConverterBase
{
public:
    KisModelIndexConverterAnimatedLayers(KisDummiesFacadeBase *dummiesFacade,
                                         KisNodeModel *model);

    KisNodeDummy *dummyFromRow(int row, QModelIndex parent);
    KisNodeDummy *dummyFromIndex(QModelIndex index);
    QModelIndex indexFromDummy(KisNodeDummy *dummy);
    bool indexFromAddedDummy(KisNodeDummy *parentDummy, int index, const QString &newNodeMetaObjectType, QModelIndex &parentIndex, int &row);
    int rowCount(QModelIndex parent);

private:
    KisDummiesFacadeBase *m_dummiesFacade;
    KisNodeModel *m_model;
};


#endif
