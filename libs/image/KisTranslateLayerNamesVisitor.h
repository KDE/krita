/*
 *  Copyright (c) 2020 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KISTRANSLATELAYERNAMESVISITOR_H
#define KISTRANSLATELAYERNAMESVISITOR_H

#include <QMap>
#include "kis_node_visitor.h"

#include <kritaimage_export.h>


/**
 * @brief KisTranslateLayerNamesVisitor::KisTranslateLayerNamesVisitor translates layer names
 * from templates.
 */
class KRITAIMAGE_EXPORT KisTranslateLayerNamesVisitor : public KisNodeVisitor
{
public:
    KisTranslateLayerNamesVisitor(QMap<QString, QString> dictionary);

    using KisNodeVisitor::visit;

    bool visit(KisNode* node) override;

    bool visit(KisPaintLayer *layer) override;

    bool visit(KisGroupLayer *layer) override;

    bool visit(KisAdjustmentLayer *layer) override;

    bool visit(KisExternalLayer *layer) override;

    bool visit(KisCloneLayer *layer) override;

    bool visit(KisFilterMask *mask) override;

    bool visit(KisTransformMask *mask) override;

    bool visit(KisTransparencyMask *mask) override;

    bool visit(KisGeneratorLayer * layer) override;

    bool visit(KisSelectionMask* mask) override;

    bool visit(KisColorizeMask* mask) override;

    QMap<QString, QString> defaultDictionary();

private:

    bool translate(KisNode *node);

    QMap<QString, QString> m_dictionary;
};

#endif // KISTRANSLATELAYERNAMESVISITOR_H
