/*
 * SPDX-FileCopyrightText: 2023 Rasyuqa A. H. <qampidh@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef LAYEROPACITYCHECK_H
#define LAYEROPACITYCHECK_H

#include "KisExportCheckRegistry.h"
#include <KoID.h>
#include <klocalizedstring.h>
#include <kis_image.h>
#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>
#include <kis_layer.h>
#include <kis_node_visitor.h>
#include "kis_node.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_external_layer_iface.h"
#include "kis_clone_layer.h"
#include "generator/kis_generator_layer.h"

class KisLayerOpacityCheckVisitor : public KisNodeVisitor
{
public:

    using KisNodeVisitor::visit;

    KisLayerOpacityCheckVisitor()
        : m_count(0)
    {
    }

    quint32 count() {
        return m_count;
    }

    bool visit(KisNode* node) override {
        return check(node);
    }

    bool visit(KisPaintLayer *layer) override {
        return check(layer);
    }

    bool visit(KisGroupLayer *layer) override {
        return check(layer);
    }


    bool visit(KisAdjustmentLayer *layer) override {
        return check(layer);
    }

    bool visit(KisExternalLayer *layer) override {
        return check(layer);
    }

    bool visit(KisCloneLayer *layer) override {
        return check(layer);
    }

    bool visit(KisGeneratorLayer * layer) override {
        return check(layer);
    }

    bool visit(KisFilterMask *) override {return true;}

    bool visit(KisTransformMask *) override {return true;}

    bool visit(KisTransparencyMask *) override {return true;}

    bool visit(KisSelectionMask *) override {return true;}

    bool visit(KisColorizeMask *) override {return true;}


private:
    bool check(KisNode * node)
    {
        KisLayer *layer = dynamic_cast<KisLayer*>(node);
        if (layer) {
            // XXX: This need to be rewritten if true float opacity is implemented
            if (layer->opacity() < OPACITY_OPAQUE_U8) {
                m_count++;
            }
        }
        visitAll(node);
        return true;
    }

    quint32 m_count;

};

class LayerOpacityCheck : public KisExportCheckBase
{
public:

    LayerOpacityCheck(const QString &id, Level level, const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning, true)
    {
        if (customWarning.isEmpty()) {
            m_warning = i18nc(
                "image conversion warning",
                "Your image contains layers with a partial opacity. The layers will be exported at full opacity.");
        }
    }

    bool checkNeeded(KisImageSP image) const override
    {
        KisLayerOpacityCheckVisitor v;
        image->rootLayer()->accept(v);
        return (v.count() > 0);
    }

    Level check(KisImageSP /*image*/) const override
    {
        return m_level;
    }

};

class LayerOpacityCheckFactory : public KisExportCheckFactory
{
public:

    LayerOpacityCheckFactory()
    {
    }

    ~LayerOpacityCheckFactory() override {}

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning) override
    {
        return new LayerOpacityCheck(id(), level, customWarning);
    }

    QString id() const override {
        return "LayerOpacityCheck";
    }

};


#endif // LAYEROPACITYCHECK_H
