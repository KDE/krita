/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SHAPELAYERTYPECHECK_H
#define SHAPELAYERTYPECHECK_H


#include "KisExportCheckRegistry.h"
#include <KoID.h>
#include <klocalizedstring.h>
#include <kis_assert.h>
#include <kis_image.h>
#include <kis_generator_registry.h>
#include <kis_node_visitor.h>
#include <kis_group_layer.h>
#include <KoShapeLayer.h>
#include <KoShapeGroup.h>
#include <KoPathShape.h>

class ShapeLayerTypeCheckVisitor : public KisNodeVisitor
{
public:

    using KisNodeVisitor::visit;

    ShapeLayerTypeCheckVisitor (QString shapeId, QString pathId)
        : m_count(0)
        , m_shapeId(shapeId)
        , m_pathShapeId(pathId)
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

    bool visit(KisExternalLayer * layer) override {
        if (KoShapeLayer *shapeLayer = dynamic_cast<KoShapeLayer*>(layer)) {
            check(shapeLayer->shapes());
        }
        return true;
    }

    bool visit(KisCloneLayer *) override {return true;}

    bool visit(KisGeneratorLayer *) override {return true;}

    bool visit(KisFilterMask *) override {return true;}

    bool visit(KisTransformMask *) override {return true;}

    bool visit(KisTransparencyMask *) override {return true;}

    bool visit(KisSelectionMask *) override {return true;}

    bool visit(KisColorizeMask *) override {return true;}



private:
    bool check(QList<KoShape*> shapes) {
        Q_FOREACH(KoShape* shape, shapes) {
            if (KoShapeGroup *group = dynamic_cast<KoShapeGroup*>(shape)) {
                // ShapeGroups have no id...
                if (m_shapeId == "KoShapeGroup") {
                    m_count ++;
                }
                check(group->shapes());
            } else if (KoPathShape *path = dynamic_cast<KoPathShape*>(shape)) {
                if (!m_pathShapeId.isEmpty()) {
                    if (path->pathShapeId() == m_pathShapeId
                            && m_shapeId == path->shapeId()) {
                        m_count ++;
                    }
                } else if (m_shapeId == path->shapeId()) {
                    m_count ++;
                }
            } else if (m_shapeId == shape->shapeId()) {
                m_count ++;
            }
        }
        return true;
    }

    quint32 m_count;
    const QString m_shapeId;
    const QString m_pathShapeId;

};


class ShapeLayerTypeCheck : public KisExportCheckBase
{
public:

    ShapeLayerTypeCheck(const QString &ShapeId, const QString &PathShapeId, const QString &id, Level level, const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning)
        , m_shapeId(ShapeId)
        , m_pathShapeId(PathShapeId)
    {
        if (customWarning.isEmpty()) {
            if (m_pathShapeId.isEmpty()) {
                QString name = m_shapeId;
                m_warning = i18nc("image conversion warning", "The image contains a Vector Layer with Shapes of type <b>%1</b>, which is not supported, these will not be saved.", name);
            } else {
                m_warning = i18nc("image conversion warning", "The image contains a Vector Layer with Shapes of type <b>%1</b>, which is not supported, these will be saved as paths", m_pathShapeId);
            }
        }
    }

    bool checkNeeded(KisImageSP image) const override
    {
        ShapeLayerTypeCheckVisitor v(m_shapeId, m_pathShapeId);
        image->rootLayer()->accept(v);
        return (v.count() > 0);
    }

    Level check(KisImageSP /*image*/) const override
    {
        return m_level;
    }

    QString m_shapeId;
    QString m_pathShapeId;
};

class ShapeLayerTypeCheckFactory : public KisExportCheckFactory
{
public:

    ShapeLayerTypeCheckFactory(const QString &ShapeId, const QString &PathShapeId = "")
        : m_shapeId(ShapeId)
        , m_pathShapeId(PathShapeId)
    {
    }

    ~ShapeLayerTypeCheckFactory() override {}

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning) override
    {
        return new ShapeLayerTypeCheck(m_shapeId, m_pathShapeId, id(), level, customWarning);
    }

    QString id() const override {
        if (m_pathShapeId.isEmpty()) {
            return "ShapeLayerTypeCheck/" + m_shapeId;
        }
        return "ShapeLayerTypeCheck/" + m_shapeId + "/" + m_pathShapeId;
    }

    const QString m_shapeId;
    const QString m_pathShapeId;
};
#endif // SHAPELAYERTYPECHECK_H
