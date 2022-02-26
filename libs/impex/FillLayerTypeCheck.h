/*
 * SPDX-FileCopyrightText: 2022 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef FILLLAYERTYPECHECK_H
#define FILLLAYERTYPECHECK_H


#include "KisExportCheckRegistry.h"
#include <KoID.h>
#include <klocalizedstring.h>
#include <kis_assert.h>
#include <kis_image.h>
#include <kis_generator_registry.h>
#include <kis_generator_layer.h>
#include <kis_node_visitor.h>
#include <kis_group_layer.h>


class FillLayerTypeCheckVisitor : public KisNodeVisitor
{
public:

    using KisNodeVisitor::visit;

    FillLayerTypeCheckVisitor (QString fillLayerID)
        : m_count(0)
        , m_fillLayerID(fillLayerID)
    {
    }

    quint32 count() {
        return m_count;
    }

    bool visit(KisNode*) override {return true;}

    bool visit(KisPaintLayer *) override {return true;}

    bool visit(KisGroupLayer *layer) override {
        return visitAll(layer);
    }


    bool visit(KisAdjustmentLayer *) override {return true;}

    bool visit(KisExternalLayer *) override {return true;}

    bool visit(KisCloneLayer *) override {return true;}

    bool visit(KisGeneratorLayer * layer) override {
        return check(layer);
    }

    bool visit(KisFilterMask *) override {return true;}

    bool visit(KisTransformMask *) override {return true;}

    bool visit(KisTransparencyMask *) override {return true;}

    bool visit(KisSelectionMask *) override {return true;}

    bool visit(KisColorizeMask *) override {return true;}


private:
    bool check(KisGeneratorLayer * node) {
        if (node->filter()->name() == m_fillLayerID) {
            m_count++;
        }
        return true;
    }

    quint32 m_count;
    const QString m_fillLayerID;

};


class FillLayerTypeCheck : public KisExportCheckBase
{
public:

    FillLayerTypeCheck(const QString &generatorID, const QString &id, Level level, const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning),
          m_fillLayerID(generatorID)
    {
        if (customWarning.isEmpty()) {
            QString name = KisGeneratorRegistry::instance()->get(generatorID)->name();
            m_warning = i18nc("image conversion warning", "The image contains a Fill Layer of type <b>%1</b>, which is not supported, the layer will be converted to a paintlayer", name);
        }
    }

    bool checkNeeded(KisImageSP image) const override
    {
        FillLayerTypeCheckVisitor v(m_fillLayerID);
        image->rootLayer()->accept(v);
        return (v.count() > 0);
    }

    Level check(KisImageSP /*image*/) const override
    {
        return m_level;
    }

    const QString m_fillLayerID;
};

class FillLayerTypeCheckFactory : public KisExportCheckFactory
{
public:

    FillLayerTypeCheckFactory(const QString &generatorID)
        : m_fillLayerID(generatorID)
    {
    }

    ~FillLayerTypeCheckFactory() override {}

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning) override
    {
        return new FillLayerTypeCheck(m_fillLayerID, id(), level, customWarning);
    }

    QString id() const override {
        return "FillLayerTypeCheck/" + m_fillLayerID;
    }

    const QString m_fillLayerID;
};
#endif // FILLLAYERTYPECHECK_H
