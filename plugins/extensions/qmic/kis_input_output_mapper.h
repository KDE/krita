/*
 * SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    void allLayers(KisNodeListSP result, bool visible);

private:
    KisImageWSP m_image;
    KisNodeSP m_activeNode;

};

#endif
