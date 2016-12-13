/*
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef PSDLayerStyleCHECK_H
#define PSDLayerStyleCHECK_H

#include "KisExportCheckRegistry.h"
#include <KoID.h>
#include <klocalizedstring.h>
#include <kis_image.h>
#include <KoColorSpace.h>
#include <kis_node_visitor.h>
#include "kis_node.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_external_layer_iface.h"
#include "kis_clone_layer.h"
#include "generator/kis_generator_layer.h"

class KisLayerStyleVisitor : public KisNodeVisitor
{
public:

    using KisNodeVisitor::visit;

    KisLayerStyleVisitor()
        : m_count(0)
    {
    }

    quint32 count() {
        return m_count;
    }

    bool visit(KisNode* node) {
        return check(node);
    }

    bool visit(KisPaintLayer *layer) {
        return check(layer);
    }

    bool visit(KisGroupLayer *layer) {
        return check(layer);
    }


    bool visit(KisAdjustmentLayer *layer) {
        return check(layer);
    }

    bool visit(KisExternalLayer *layer) {
        return check(layer);
    }

    bool visit(KisCloneLayer *layer) {
        return check(layer);
    }

    bool visit(KisGeneratorLayer * layer) {
        return check(layer);
    }

    virtual bool visit(KisFilterMask *) {return true;}

    virtual bool visit(KisTransformMask *) {return true;}

    virtual bool visit(KisTransparencyMask *) {return true;}

    virtual bool visit(KisSelectionMask *) {return true;}

    virtual bool visit(KisColorizeMask *) {return true;}


private:
    bool check(KisNode * node)
    {
        KisLayer *layer = dynamic_cast<KisLayer*>(node);
        if (layer) {
            if (layer->layerStyle()) {
                m_count++;
            }
        }
        visitAll(node);
        return true;
    }

    quint32 m_count;
};


class PSDLayerStyleCheck : public KisExportCheckBase
{
public:

    PSDLayerStyleCheck(const QString &id, Level level, const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning, true)
    {
        if (customWarning.isEmpty()) {
            m_warning = i18nc("image conversion warning", "The image contains <b>layer styles</b>. The layer styles will not be saved.");
        }
    }

    bool checkNeeded(KisImageSP image) const
    {
        KisLayerStyleVisitor  v;
        image->rootLayer()->accept(v);
        return (v.count() > 0);
    }

    Level check(KisImageSP /*image*/) const
    {
        return m_level;
    }
};

class PSDLayerStyleCheckFactory : public KisExportCheckFactory
{
public:

    PSDLayerStyleCheckFactory() {}

    virtual ~PSDLayerStyleCheckFactory() {}

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning)
    {
        return new PSDLayerStyleCheck(id(), level, customWarning);
    }

    QString id() const {
        return "PSDLayerStyleCheck";
    }
};

#endif // PSDLayerStyleCHECK_H
