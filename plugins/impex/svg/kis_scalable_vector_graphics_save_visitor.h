#ifndef KIS_SCALABLE_VECTOR_GRAPHICS_SAVE_VISITOR_H
#define KIS_SCALABLE_VECTOR_GRAPHICS_SAVE_VISITOR_H

#include <QSet>

#include "kis_global.h"
#include "kis_types.h"

#include "kis_node_visitor.h"
#include "kis_layer.h"
#include "KoShape.h"
#include "KoPathShape.h"
#include "SvgSavingContext.h"

class KisScalableVectorGraphicsSaveContext;

class KisAdjustmentLayer;
class KisGroupLayer;
class KisPaintLayer;
class KisGeneratorLayer;
class KoShapeGroup;

class QDomElement;

class KisScalableVectorGraphicsSaveVisitor : public KisNodeVisitor
{
public:
    KisScalableVectorGraphicsSaveVisitor(QIODevice* saveDevice, vKisNodeSP activeNodes, QSizeF, SvgSavingContext* savingContext);
    ~KisScalableVectorGraphicsSaveVisitor() override;

    using KisNodeVisitor::visit;

public:
    bool visit(KisPaintLayer *layer) override;
    bool visit(KisGroupLayer *layer) override;
    bool visit(KisAdjustmentLayer *layer) override;
    bool visit(KisGeneratorLayer * layer) override;

    bool visit(KisNode*) override {
        return true;
    }

    bool visit(KisCloneLayer*) override;

    bool visit(KisFilterMask*) override {
        return true;
    }
    bool visit(KisTransformMask*) override {
        return true;
    }
    bool visit(KisTransparencyMask*) override {
        return true;
    }
    bool visit(KisSelectionMask*) override {
        return true;
    }
    bool visit(KisColorizeMask*) override {
        return true;
    }

    bool visit(KisExternalLayer*) override;

private:
    bool saveLayer(KisLayer *layer);
    void saveLayerInfo(QDomElement& elt, KisLayer* layer);
    void saveShape(KoShape *shape);
    void savePath(KoPathShape *path);
    void saveGeneric(KoShape *shape);
    void saveGroup(KoShapeGroup *group);



    struct Private;
    Private* const d;
};

#endif // KIS_SCALABLE_VECTOR_GRAPHICS_SAVE_VISITOR_H
