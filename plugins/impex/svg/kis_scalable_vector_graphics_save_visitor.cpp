#include "kis_scalable_vector_graphics_save_visitor.h"
#include <math.h>

#include <QDomElement>
#include <QImage>
#include <QBuffer>
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
#include <QTextStream>
#include <KoXmlNS.h>
#include <KoShapeLayer.h>
#include <KoShapeGroup.h>
#include <KoShapeLayer.h>
#include <KoPathShape.h>
#include <SvgShape.h>
#include "KoXmlWriter.h"
#include <SvgUtil.h>
#include "SvgStyleWriter.h"
#include "SvgSavingContext.h"
#include <KoShapePainter.h>
#include <QSvgGenerator>

struct KisScalableVectorGraphicsSaveVisitor::Private {
    Private() {}
    SvgSavingContext* saveContext {nullptr};
    QDomDocument layerStack;
    QDomElement currentElement;
    vKisNodeSP activeNodes;
    QIODevice* saveDevice ;

};


KisScalableVectorGraphicsSaveVisitor::KisScalableVectorGraphicsSaveVisitor(QIODevice* saveDevice, vKisNodeSP activeNodes, QSizeF pageSize, SvgSavingContext* savingContext)
    : d(new Private)
{
    d->saveDevice = saveDevice;
    d->activeNodes = activeNodes;
    d->saveContext = savingContext;

    QTextStream svgStream(saveDevice);
    svgStream.setCodec("UTF-8");

    // standard header:
    svgStream << "<?xml version=\"1.0\" standalone=\"no\"?>" << endl;
    svgStream << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\" ";
    svgStream << "\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">" << endl;

    // add some PR.  one line is more than enough.
    svgStream << "<!-- Created using Krita: https://krita.org -->" << endl;

//    svgStream << "<svg xmlns=\"http://www.w3.org/2000/svg\" \n";
//    svgStream << "    xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n";
//    svgStream << QString("    xmlns:krita=\"%1\"\n").arg(KoXmlNS::krita);
//    svgStream << "    xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"\n";
//    svgStream << "    width=\"" << pageSize.width() << "pt\"\n";
//    svgStream << "    height=\"" << pageSize.height() << "pt\"\n";
//    svgStream << "    viewBox=\"0 0 "
//              << pageSize.width() << " " << pageSize.height()
//              << "\"" ;
//    svgStream << ">" << endl;


    d->saveContext->shapeWriter().startElement("svg");
    d->saveContext->shapeWriter().addAttribute("xmlns", "http://www.w3.org/2000/svg");
    d->saveContext->shapeWriter().addAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    d->saveContext->shapeWriter().addAttribute("xmlns:krita", "http://krita.org/namespaces/svg/krita");
    d->saveContext->shapeWriter().addAttribute("xmlns:sodipodi", "http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd");

    d->saveContext->shapeWriter().addAttribute("width", QString::number(pageSize.width()) + "pt");
    d->saveContext->shapeWriter().addAttribute("height", QString::number(pageSize.height()) + "pt" );
    d->saveContext->shapeWriter().addAttribute("viewBox", "0 0 " + QString::number(pageSize.width()) + " " + QString::number(pageSize.height()));



}

KisScalableVectorGraphicsSaveVisitor::~KisScalableVectorGraphicsSaveVisitor()
{
    QTextStream svgStream(d->saveDevice);
//    svgStream.setCodec("UTF-8");
//    svgStream << endl << "</svg>" << endl;
    d->saveContext->shapeWriter().endElement();

    delete d;
}

bool KisScalableVectorGraphicsSaveVisitor::visit(KisPaintLayer *layer)
{
//    qDebug() << "paintlayer: " << layer->name();

//    return saveLayer(layer);
    QRect rc = layer->projection()->exactBounds();
    //QImage resulteImage = layer->projection()->convertToQImage(0, layer->projection()->x(), layer->projection()->y(), rc.width(), rc.height() );
    QImage resulteImage = layer->projection()->convertToQImage(0, layer->exactBounds());
    d->saveContext->shapeWriter().startElement("image");
    d->saveContext->shapeWriter().addAttribute("id", layer->name());
    d->saveContext->shapeWriter().addAttribute("x", layer->projection()->x());
    d->saveContext->shapeWriter().addAttribute("y", layer->projection()->y());
    d->saveContext->shapeWriter().addAttribute("width", rc.width());
    d->saveContext->shapeWriter().addAttribute("height", rc.height() );
    d->saveContext->shapeWriter().addAttribute("xlink:href", d->saveContext->saveImage(resulteImage));
    d->saveContext->shapeWriter().endElement(); // image


    return true;
}

bool KisScalableVectorGraphicsSaveVisitor::visit(KisGroupLayer *layer)
{
//    qDebug() << "grouplayer: " << layer->name();

    visitAll(layer);
    return true;
}

bool KisScalableVectorGraphicsSaveVisitor::visit(KisAdjustmentLayer *layer)
{

    return saveLayer(layer);
}

bool KisScalableVectorGraphicsSaveVisitor::visit(KisExternalLayer *layer)
{
    qDebug() << "external layer: " << layer->name();
//    return true;
    return saveLayer(layer);
}

bool KisScalableVectorGraphicsSaveVisitor::visit(KisGeneratorLayer *layer)
{
    return saveLayer(layer);
}

bool KisScalableVectorGraphicsSaveVisitor::visit(KisCloneLayer *layer)
{
    return saveLayer(layer);
}

bool KisScalableVectorGraphicsSaveVisitor::saveLayer(KisLayer *layer)
{
    QTextStream svgStream(d->saveDevice);
    svgStream.setCodec("UTF-8");

    if (layer->isFakeNode()) {
        // don't save grids, reference images layers etc.
        return true;
    }

    // here we adjust the bounds to encompass the entire area of the layer, including transforms
    QRect adjustedBounds = layer->exactBounds();

    if (adjustedBounds.isEmpty()) {
        // in case of an empty layer, artificially increase the size of the saved rectangle
        // to just save an empty layer file
        adjustedBounds.adjust(0, 0, 1, 1);
    }
    qDebug() << "savelayer: " << layer->name();

    QDomElement elt = d->layerStack.createElement("layer");
    saveLayerInfo(elt, layer);
    d->currentElement.insertBefore(elt, QDomNode());
    d->saveContext->shapeWriter().startElement("g");

    KoShapeLayer *kolayer = dynamic_cast<KoShapeLayer*>(layer);

    d->saveContext->shapeWriter().addAttribute("id", d->saveContext->getID(kolayer));
    d->saveContext->shapeWriter().addAttribute("krita:label", layer->name() );
    QList<KoShape*> sortedShapes = kolayer->shapes();
    std::sort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);

    Q_FOREACH (KoShape * shape, sortedShapes) {
        KoShapeGroup * group = dynamic_cast<KoShapeGroup*>(shape);
        if (group)
            saveGroup(group);
        else
            saveShape(shape);
    }


    KisLayerSP layerSP = qobject_cast<KisLayer*>(layer);



    QList<KoShape*> toplevelShapes;

    d->saveContext->shapeWriter().endElement();

    return true;
}

void KisScalableVectorGraphicsSaveVisitor::saveShape(KoShape *shape)
{
    KoPathShape * path = dynamic_cast<KoPathShape*>(shape);
    if (path) {
        savePath(path);
    } else {
        // generic saving of shape via a switch element
        qDebug() << "saving generic";
        saveGeneric(shape);
    }
}

void KisScalableVectorGraphicsSaveVisitor::savePath(KoPathShape *path)
{
    d->saveContext->shapeWriter().startElement("path");
    d->saveContext->shapeWriter().addAttribute("id", d->saveContext->getID(path));

    SvgUtil::writeTransformAttributeLazy("transform", path->transformation(), d->saveContext->shapeWriter());

    SvgStyleWriter::saveSvgStyle(path, *d->saveContext);

    d->saveContext->shapeWriter().addAttribute("d", path->toString(d->saveContext->userSpaceTransform()));
    d->saveContext->shapeWriter().addAttribute("sodipodi:nodetypes", path->nodeTypes());
    d->saveContext->shapeWriter().endElement();
}

void KisScalableVectorGraphicsSaveVisitor::saveGeneric(KoShape *shape)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(shape);

    const QRectF bbox = shape->boundingRect();

    // paint shape to the image
    KoShapePainter painter;
    painter.setShapes(QList<KoShape*>()<< shape);

    // generate svg from shape
    QBuffer svgBuffer;
    QSvgGenerator svgGenerator;
    svgGenerator.setOutputDevice(&svgBuffer);

    /**
     * HACK ALERT: Qt (and Krita 3.x) has a weird bug, it assumes that all font sizes are
     *             defined in 96 ppi resolution, even though your the resolution in QSvgGenerator
     *             is manually set to 72 ppi. So here we do a tricky thing: we set a fake resolution
     *             to (72 * 72 / 96) = 54 ppi, which guarantees that the text, when painted in 96 ppi,
     *             will be actually painted in 72 ppi.
     *
     * BUG: 389802
     */
    if (shape->shapeId() == "TextShapeID") {
        svgGenerator.setResolution(54);
    }

    QPainter svgPainter;
    svgPainter.begin(&svgGenerator);
    painter.paint(svgPainter, SvgUtil::toUserSpace(bbox).toRect(), bbox);
    svgPainter.end();

    // remove anything before the start of the svg element from the buffer
    int startOfContent = svgBuffer.buffer().indexOf("<svg");
    if(startOfContent>0) {
        svgBuffer.buffer().remove(0, startOfContent);
    }

    // check if painting to svg produced any output
    if (svgBuffer.buffer().isEmpty()) {
        // prepare a transparent image, make it twice as big as the original size
        QImage image(2*bbox.size().toSize(), QImage::Format_ARGB32);
        image.fill(0);
        painter.paint(image);

        d->saveContext->shapeWriter().startElement("image");
        d->saveContext->shapeWriter().addAttribute("id", d->saveContext->getID(shape));
        d->saveContext->shapeWriter().addAttribute("x", bbox.x());
        d->saveContext->shapeWriter().addAttribute("y", bbox.y());
        d->saveContext->shapeWriter().addAttribute("width", bbox.width());
        d->saveContext->shapeWriter().addAttribute("height", bbox.height());
        d->saveContext->shapeWriter().addAttribute("xlink:href", d->saveContext->saveImage(image));
        d->saveContext->shapeWriter().endElement(); // image

    } else {
        d->saveContext->shapeWriter().addCompleteElement(&svgBuffer);
    }

    // TODO: once we support saving single (flat) odf files
    // we can embed these here to have full support for generic shapes

}

void KisScalableVectorGraphicsSaveVisitor::saveGroup(KoShapeGroup * group)
{
    d->saveContext->shapeWriter().startElement("g");
    d->saveContext->shapeWriter().addAttribute("id", d->saveContext->getID(group));

    SvgUtil::writeTransformAttributeLazy("transform", group->transformation(), d->saveContext->shapeWriter());

    SvgStyleWriter::saveSvgStyle(group, *d->saveContext);

    QList<KoShape*> sortedShapes = group->shapes();
    std::sort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);

    Q_FOREACH (KoShape * shape, sortedShapes) {
        KoShapeGroup * childGroup = dynamic_cast<KoShapeGroup*>(shape);
        if (childGroup)
            saveGroup(childGroup);
        else
            saveShape(shape);
    }

    d->saveContext->shapeWriter().endElement();
}

void KisScalableVectorGraphicsSaveVisitor::saveLayerInfo(QDomElement& elt, KisLayer* layer)
{
}
