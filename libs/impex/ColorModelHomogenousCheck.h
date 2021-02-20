/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef COLORMODELHOMOGENOUSCHECK_H
#define COLORMODELHOMOGENOUSCHECK_H

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

class KisColorModelHomogenousCheckVisitor : public KisNodeVisitor
{
public:

    using KisNodeVisitor::visit;

    KisColorModelHomogenousCheckVisitor(KoID colorModelID, KoID colorDepthID)
        : m_count(0)
        , m_colorModelID(colorModelID)
        , m_colorDepthID(colorDepthID)
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

            const KoColorSpace * cs = layer->colorSpace();
            if (cs->colorModelId() != m_colorModelID || cs->colorDepthId() != m_colorDepthID) {
                m_count++;
            }
        }
        visitAll(node);
        return true;
    }

    quint32 m_count;
    const KoID m_colorModelID;
    const KoID m_colorDepthID;

};

class ColorModelHomogenousCheck : public KisExportCheckBase
{
public:

    ColorModelHomogenousCheck(const QString &id, Level level, const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning, true)
    {
        if (customWarning.isEmpty()) {
            m_warning = i18nc("image conversion warning", "Your image contains layers with a color model that is different from the image. The layers will be converted.");
        }
    }

    bool checkNeeded(KisImageSP image) const override
    {
        const KoColorSpace *cs = image->colorSpace();
        KisColorModelHomogenousCheckVisitor v(cs->colorModelId(), cs->colorDepthId());
        image->rootLayer()->accept(v);
        return (v.count() > 0);
    }

    Level check(KisImageSP /*image*/) const override
    {
        return m_level;
    }

};

class ColorModelHomogenousCheckFactory : public KisExportCheckFactory
{
public:

    ColorModelHomogenousCheckFactory()
    {
    }

    ~ColorModelHomogenousCheckFactory() override {}

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning) override
    {
        return new ColorModelHomogenousCheck(id(), level, customWarning);
    }

    QString id() const override {
        return "ColorModelHomogenousCheck";
    }

};


#endif // COLORMODELHOMOGENOUSCHECK_H
