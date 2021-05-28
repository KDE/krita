/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "VectorLayer.h"
#include <kis_shape_layer.h>
#include <kis_image.h>
#include <SvgWriter.h>
#include <SvgParser.h>
#include <QBuffer>
#include <commands/KoShapeCreateCommand.h>
#include <KoShapeGroup.h>
#include <KisDocument.h>
#include <kis_processing_applicator.h>

#include "Krita.h"
#include "GroupShape.h"

VectorLayer::VectorLayer(KoShapeControllerBase* shapeController, KisImageSP image, QString name, QObject *parent) :
    Node(image, new KisShapeLayer(shapeController, image, name, OPACITY_OPAQUE_U8), parent)
{

}

VectorLayer::VectorLayer(KisShapeLayerSP layer, QObject *parent):
    Node(layer->image(), layer, parent)
{

}

VectorLayer::~VectorLayer()
{

}

QString VectorLayer::type() const
{
    return "vectorlayer";
}

QList<Shape *> VectorLayer::shapes() const
{
    QList<Shape*> shapes;
    KisShapeLayerSP vector = KisShapeLayerSP(dynamic_cast<KisShapeLayer*>(this->node().data()));
    if (vector) {
        QList<KoShape*> originalShapes = vector->shapes();
        std::sort(originalShapes.begin(), originalShapes.end(), KoShape::compareShapeZIndex);
        for (int i=0; i<vector->shapeCount(); i++) {
            if (dynamic_cast<KoShapeGroup*>(originalShapes.at(i))) {
                shapes << new GroupShape(dynamic_cast<KoShapeGroup*>(originalShapes.at(i)));
            } else {
                shapes << new Shape(originalShapes.at(i));
            }
        }
    }
    return shapes;
}

QString VectorLayer::toSvg()
{
    QString svgData;
    KisShapeLayerSP vector = KisShapeLayerSP(dynamic_cast<KisShapeLayer*>(this->node().data()));

    if (vector) {
        QBuffer buffer;
        QList<KoShape*> originalShapes = vector->shapes();

        std::sort(originalShapes.begin(), originalShapes.end(), KoShape::compareShapeZIndex);

        const QSizeF sizeInPx = this->node()->image()->bounds().size();
        const QSizeF pageSize(sizeInPx.width() / this->node()->image()->xRes(),
                          sizeInPx.height() / this->node()->image()->yRes());

        buffer.open(QIODevice::WriteOnly);

        SvgWriter writer(originalShapes);

        writer.save(buffer, pageSize);
        buffer.close();

        svgData = QString::fromUtf8(buffer.data());
    }

    return svgData;

}

QList<Shape *> VectorLayer::addShapesFromSvg(const QString &svgData)
{
    QList<Shape*> shapes;
    QList<KoShape*> originalShapes;

    if (svgData.isEmpty() || !svgData.contains("<svg") ) {
        return shapes;
    }

    KoShapeContainer *container = dynamic_cast<KoShapeContainer*>(this->node().data());

    if (container) {
        QSizeF fragmentSize;
        QString errorMsg;
        int errorLine = 0;
        int errorColumn = 0;

        QDomDocument dom = SvgParser::createDocumentFromSvg(svgData, &errorMsg, &errorLine, &errorColumn);

        if (dom.isNull()) {
            qWarning() << "Failed to process an SVG string at"
                       << errorLine << ":" << errorColumn << "->" << errorMsg;
            return shapes;
        }

        Document *document = Krita::instance()->activeDocument();
        SvgParser parser(document->document()->shapeController()->resourceManager());

        parser.setResolution(this->node()->image()->bounds(), this->node()->image()->xRes() * 72.0);

        originalShapes = parser.parseSvg(dom.documentElement(), &fragmentSize);

        KUndo2Command *cmd = new KoShapeCreateCommand(document->document()->shapeController(), originalShapes, container);

        KisProcessingApplicator::runSingleCommandStroke(this->node()->image(), cmd);
        this->node()->image()->waitForDone();
        delete document;

        std::sort(originalShapes.begin(), originalShapes.end(), KoShape::compareShapeZIndex);
        for (int i=0; i<originalShapes.size(); i++) {
            if (dynamic_cast<KoShapeGroup*>(originalShapes.at(i))) {
                shapes << new GroupShape(dynamic_cast<KoShapeGroup*>(originalShapes.at(i)));
            } else {
                shapes << new Shape(originalShapes.at(i));
            }
        }

    }

    return shapes;
}
