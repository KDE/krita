#include "kis_scalable_vector_graphics_save_visitor.h"
#include <math.h>

#include <QDomElement>
#include <QImage>

#include <KoCompositeOpRegistry.h>

#include "kis_adjustment_layer.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include <generator/kis_generator_layer.h>
#include <kis_scalable_vector_graphics_save_context.h>
#include <kis_clone_layer.h>
#include <kis_external_layer_iface.h>

struct KisScalableVectorGraphicsSaveVisitor::Private {
    Private() {}
    KisScalableVectorGraphicsSaveContext* saveContext {nullptr};
    QDomDocument layerStack;
    QDomElement currentElement;
    vKisNodeSP activeNodes;
};


KisScalableVectorGraphicsSaveVisitor::KisScalableVectorGraphicsSaveVisitor(KisScalableVectorGraphicsSaveContext* saveContext, vKisNodeSP activeNodes)
    : d(new Private)
{
    d->saveContext = saveContext;
    d->activeNodes = activeNodes;
}

KisScalableVectorGraphicsSaveVisitor::~KisScalableVectorGraphicsSaveVisitor()
{
    delete d;
}

bool KisScalableVectorGraphicsSaveVisitor::visit(KisPaintLayer *layer)
{
    return true;
}

bool KisScalableVectorGraphicsSaveVisitor::visit(KisGroupLayer *layer)
{
    return true;
}

bool KisScalableVectorGraphicsSaveVisitor::visit(KisAdjustmentLayer *layer)
{
    return true;
}

bool KisScalableVectorGraphicsSaveVisitor::visit(KisExternalLayer *layer)
{
    return true;
}

bool KisScalableVectorGraphicsSaveVisitor::visit(KisGeneratorLayer *layer)
{
    return true;
}

bool KisScalableVectorGraphicsSaveVisitor::visit(KisCloneLayer *layer)
{
    return true;
}
