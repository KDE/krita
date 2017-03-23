/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com
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

#ifndef _KIS_INPUT_OUTPUT_MAPPER
#define _KIS_INPUT_OUTPUT_MAPPER

#include <kis_types.h>
#include "gmic.h"

class KisInputOutputMapper
{
public:
    KisInputOutputMapper(KisImageWSP image, KisNodeSP activeNode);
    KisNodeListSP inputNodes(InputLayerMode inputMode);

private:
    void allLayers(KisNodeListSP result);
    void allInverseOrderedLayers(KisNodeListSP result);

private:
    KisImageWSP m_image;
    KisNodeSP m_activeNode;

};

#endif
