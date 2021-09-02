/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "LibKisUtils.h"

#include <kis_node.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_file_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_generator_layer.h>
#include <kis_clone_layer.h>
#include <kis_shape_layer.h>
#include <kis_transparency_mask.h>
#include <kis_filter_mask.h>
#include <kis_transform_mask.h>
#include <kis_selection_mask.h>
#include <lazybrush/kis_colorize_mask.h>
#include <kis_layer.h>

#include "Node.h"
#include "GroupLayer.h"
#include "CloneLayer.h"
#include "FilterLayer.h"
#include "FillLayer.h"
#include "FileLayer.h"
#include "VectorLayer.h"
#include "FilterMask.h"
#include "SelectionMask.h"
#include "TransformMask.h"

QList<Node *> LibKisUtils::createNodeList(KisNodeList kisnodes, KisImageWSP image)
{
    QList <Node*> nodes;
    Q_FOREACH(KisNodeSP node, kisnodes) {
        nodes << Node::createNode(image, node);
    }
    return nodes;
}
