/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoCssStylePreset.h"

#include <KoShapePainter.h>
#include <KoSvgTextShape.h>
#include <KoDocumentResourceManager.h>

#include <SvgWriter.h>
#include <SvgParser.h>

#include <QDomDocument>
#include <QBuffer>
#include <QFileInfo>

#include <FlakeDebug.h>

const QString TITLE = "title";
const QString DESCRIPTION = "description";
const QString DESC = "desc";
const QString SAMPLE_SVG = "sample_svg";
const QString STYLE_TYPE = "style_type";

struct KoCssStylePreset::Private {

    Private()
        : shape(new KoSvgTextShape())
    {
    }
    ~Private() {}

    QScopedPointer<KoSvgTextShape> shape;
};

KoCssStylePreset::KoCssStylePreset(const QString &filename)
    : KoResource(filename)
    , d(new Private())
{
    setName(name().replace("_", " "));
    if (name().endsWith(defaultFileExtension())) {
        const QFileInfo f(name());
        setName(f.completeBaseName());
    }
}

KoCssStylePreset::KoCssStylePreset(const KoCssStylePreset &rhs)
    : KoResource(rhs)
    , d(new Private())
{
    d->shape.reset(dynamic_cast<KoSvgTextShape*>(rhs.d->shape.data()->cloneShape()));
    setTitle(rhs.title());
    setDescription(rhs.description());
    setValid(true);
}

KoCssStylePreset::~KoCssStylePreset()
{

}

KoSvgTextProperties KoCssStylePreset::properties()
{
    const QVector<int> treeIndex = d->shape->findTreeIndexForPropertyId(KoSvgTextProperties::KraTextStyleType);
    return d->shape->propertiesForTreeIndex(treeIndex);
}

void KoCssStylePreset::setProperties(const KoSvgTextProperties &properties)
{
    const QVector<int> treeIndex = d->shape->findTreeIndexForPropertyId(KoSvgTextProperties::KraTextStyleType);
    if (d->shape->propertiesForTreeIndex(treeIndex) == properties) return;
    d->shape->setPropertiesAtTreeIndex(treeIndex, properties);
}

QString KoCssStylePreset::title() const
{
    return d->shape->additionalAttribute(TITLE);
}

void KoCssStylePreset::setTitle(QString title)
{
    d->shape->setAdditionalAttribute(TITLE, title);
}

QString KoCssStylePreset::description() const
{
    QMap<QString, QVariant> m = metadata();
    return m[DESCRIPTION].toString();
}

void KoCssStylePreset::setDescription(QString description)
{
    d->shape->setAdditionalAttribute(DESC, description);
    addMetaData(DESCRIPTION, description);
}

KoResourceSP KoCssStylePreset::clone() const
{
    return KoResourceSP(new KoCssStylePreset(*this));
}

bool KoCssStylePreset::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    if (!dev->isOpen()) dev->open(QIODevice::ReadOnly);
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;
    QDomDocument xmlDocument = SvgParser::createDocumentFromSvg(dev, &errorMsg, &errorLine, &errorColumn);
    if (xmlDocument.isNull()) {

        errorFlake << "Parsing error in " << filename() << "! Aborting!" << Qt::endl
        << " In line: " << errorLine << ", column: " << errorColumn << Qt::endl
        << " Error message: " << errorMsg << Qt::endl;
        errorFlake << "Parsing error in the main document at line" << errorLine
                   << ", column" << errorColumn << Qt::endl
                   << "Error message: " << errorMsg;

        return false;
    }

    KoDocumentResourceManager manager;
    SvgParser parser(&manager);
    parser.setResolution(QRectF(0,0,100,100), 72); // initialize with default values
    QSizeF fragmentSize;

    QList<KoShape*> shapes = parser.parseSvg(xmlDocument.documentElement(), &fragmentSize);

    Q_FOREACH(KoShape *shape, shapes) {
        KoSvgTextShape *textShape = dynamic_cast<KoSvgTextShape*>(shape);
        if (textShape) {
            d->shape.reset(textShape);
            setName(d->shape->additionalAttribute(TITLE));
            addMetaData(DESCRIPTION, d->shape->additionalAttribute(DESC));
            QVector<int> treeIndex = d->shape->findTreeIndexForPropertyId(KoSvgTextProperties::KraTextStyleType);
            qDebug() << "style found at..." << treeIndex;
            QString styleType = "paragraph";
            if (!treeIndex.isEmpty()) {
                KoSvgTextProperties props = d->shape->propertiesForTreeIndex(treeIndex);
                styleType = props.propertyOrDefault(KoSvgTextProperties::KraTextStyleType).toString();
            }

            addMetaData(STYLE_TYPE, styleType);

            qDebug() << "type..." << styleType;
            updateThumbnail();
            setValid(true);
            return true;
        }
    }


    return false;
}

bool KoCssStylePreset::saveToDevice(QIODevice *dev) const
{
    if (!d->shape) return false;
    SvgWriter writer({d->shape.data()});
    return writer.save(dev, d->shape->boundingRect().size());
}

QString KoCssStylePreset::defaultFileExtension() const
{
    return ".svg";
}

QString generateSVG(const KoSvgTextShape *shape) {

    SvgWriter writer({shape->textOutline()});
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    writer.save(buffer, shape->boundingRect().size());
    buffer.close();

    return QString::fromUtf8(buffer.data());
}

void KoCssStylePreset::updateThumbnail()
{
    QImage img(256,
               256,
               QImage::Format_ARGB32);
    img.fill(Qt::white);

    KoShapePainter painter;
    painter.setShapes({d->shape.data()});
    painter.paint(img);

    /// generate SVG sample.
    addMetaData(SAMPLE_SVG, generateSVG(d->shape.data()));

    setImage(img);
}

QPair<QString, QString> KoCssStylePreset::resourceType() const
{
    return QPair<QString, QString>(ResourceType::CssStyles, "");
}
