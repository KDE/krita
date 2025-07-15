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
const QString SAMPLE_ALIGN = "sample_align";
const QString STYLE_TYPE = "style_type";

const QString STYLE_TYPE_PARAGRAPH = "paragraph";
const QString STYLE_TYPE_CHARACTER = "character";
const QString SAMPLE_PLACEHOLDER = i18nc("info:placeholder", "Style Sample");

struct KoCssStylePreset::Private {

    Private()
        : shape(new KoSvgTextShape())
    {
        shape->insertText(0, SAMPLE_PLACEHOLDER);
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
    setDescription(rhs.description());
    setValid(true);
}

KoCssStylePreset::~KoCssStylePreset()
{

}

KoSvgTextProperties KoCssStylePreset::properties() const
{
    const QVector<int> treeIndex = d->shape->findTreeIndexForPropertyId(KoSvgTextProperties::KraTextStyleType);
    qDebug() << "searching properties" << treeIndex;
    if (treeIndex.isEmpty()) return KoSvgTextProperties();
    return d->shape->propertiesForTreeIndex(treeIndex);
}

void KoCssStylePreset::setProperties(const KoSvgTextProperties &properties)
{
    const QVector<int> treeIndex = d->shape->findTreeIndexForPropertyId(KoSvgTextProperties::KraTextStyleType);
    if (!treeIndex.isEmpty() && d->shape->propertiesForTreeIndex(treeIndex) == properties) return;
    if (treeIndex.isEmpty()) {
        KoSvgTextProperties modifiedProps = properties;
        modifiedProps.setProperty(KoSvgTextProperties::KraTextStyleType, STYLE_TYPE_PARAGRAPH);
        d->shape->setPropertiesAtPos(-1, modifiedProps);
    } else {
        d->shape->setPropertiesAtTreeIndex(treeIndex, properties);
    }
    updateThumbnail();
    setValid(true);
}

QString KoCssStylePreset::description() const
{
    QMap<QString, QVariant> m = metadata();
    return m[DESCRIPTION].toString();
}

void KoCssStylePreset::setDescription(QString description)
{
    d->shape->setAdditionalAttribute(DESC, description);
    QMap<QString, QVariant> m = metadata();
    if (m[DESCRIPTION].toString() == description) return;
    addMetaData(DESCRIPTION, description);
    setDirty(true);
}

QString KoCssStylePreset::styleType() const
{
    QMap<QString, QVariant> m = metadata();
    return m.value(STYLE_TYPE, STYLE_TYPE_PARAGRAPH).toString();
}

void KoCssStylePreset::setStyleType(const QString &type)
{
    addMetaData(STYLE_TYPE, type);
}

QString KoCssStylePreset::sampleText() const
{
    const QVector<int> treeIndex = d->shape->findTreeIndexForPropertyId(KoSvgTextProperties::KraTextStyleType);
    if (treeIndex.isEmpty()) return QString();
    QPair<int, int> pos = d->shape->findRangeForTreeIndex(treeIndex);
    pos.first = d->shape->indexForPos(pos.first);
    pos.second = d->shape->indexForPos(pos.second);
    return d->shape->plainText().mid(pos.first, pos.second-pos.first);
}

void KoCssStylePreset::setSampleText(const QString &sample, const KoSvgTextProperties &props, const QString &before, const QString &after)
{
    KoSvgTextProperties modifiedProps = props;

    KoSvgTextShape *sampleText = new KoSvgTextShape();
    sampleText->insertText(0, sample.isEmpty()? name().isEmpty()? SAMPLE_PLACEHOLDER: name(): sample);
    const QString type = styleType().isEmpty()? STYLE_TYPE_CHARACTER: styleType();
    setStyleType(type);

    bool removeParagraph = type == STYLE_TYPE_CHARACTER;

    // Remove properties that cannot be edited.
    Q_FOREACH(KoSvgTextProperties::PropertyId p, modifiedProps.properties()) {
        if (KoSvgTextProperties::propertyIsBlockOnly(p)) {
            if (removeParagraph) {
                modifiedProps.removeProperty(p);
            }
        } else {
            if (!removeParagraph) {
                modifiedProps.removeProperty(p);
            }
        }

    }
    // This one is added after removing, because otherwise, the type is removed...
    modifiedProps.setProperty(KoSvgTextProperties::KraTextStyleType, type);
    // Always remove inline size, it is shape-specific.
    modifiedProps.removeProperty(KoSvgTextProperties::InlineSizeId);
    // Remove fill and stroke for now as we have no widgets for them.
    modifiedProps.removeProperty(KoSvgTextProperties::FillId);
    modifiedProps.removeProperty(KoSvgTextProperties::StrokeId);

    sampleText->setPropertiesAtPos(-1, modifiedProps);

    if (type == STYLE_TYPE_PARAGRAPH) {
        // For paragraph we'll add a shape, as those will allow wrapping,
        // without being part of the properties like inline-size is.
        KoPathShape *inlineShape = new KoPathShape();
        inlineShape->moveTo(QPointF(0, 0));
        inlineShape->lineTo(QPointF(120, 0));
        inlineShape->lineTo(QPointF(120, 120));
        inlineShape->lineTo(QPointF(0, 120));
        inlineShape->lineTo(QPointF(0, 0));
        inlineShape->close();
        sampleText->setShapesInside({inlineShape});
        sampleText->relayout();

        d->shape.reset(sampleText);
    } else {
        /// For character the sample always needs to be set to be a child to
        /// ensure that the character property doesn't include the default props.
        KoSvgTextShape *newShape = new KoSvgTextShape();
        KoSvgTextProperties paraProps = KoSvgTextProperties::defaultProperties();
        // Set whitespace rule to pre-wrap.
        paraProps.setProperty(KoSvgTextProperties::TextCollapseId, KoSvgText::Preserve);
        paraProps.setProperty(KoSvgTextProperties::TextWrapId, KoSvgText::Wrap);
        newShape->setPropertiesAtPos(-1, paraProps);
        if (!after.isEmpty()) {
            newShape->insertText(0, after);
        }
        if (!before.isEmpty()) {
            newShape->insertText(0, before);
        }

        newShape->insertRichText(newShape->posForIndex(before.size()), sampleText);

        d->shape.reset(newShape);

    }

    d->shape->cleanUp();
    updateThumbnail();
    setValid(true);
    setDirty(true);
}

Qt::Alignment KoCssStylePreset::alignSample() const
{
    QMap<QString, QVariant> m = metadata();
    QVariant v = m.value(SAMPLE_ALIGN, static_cast<Qt::Alignment::Int>(Qt::AlignHCenter | Qt::AlignVCenter));
    return static_cast<Qt::Alignment>(v.value<Qt::Alignment::Int>());
}

void KoCssStylePreset::updateAlignSample()
{
    Qt::AlignmentFlag hComponent = Qt::AlignHCenter;
    Qt::AlignmentFlag vComponent = Qt::AlignVCenter;

    const KoSvgTextProperties props = properties();
    const QString type = styleType().isEmpty()? props.property(KoSvgTextProperties::KraTextStyleType).toString(): styleType();
    if (type == STYLE_TYPE_PARAGRAPH) {
        const KoSvgText::WritingMode mode = KoSvgText::WritingMode(props.propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
        const KoSvgText::Direction dir = KoSvgText::Direction(props.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
        const bool textAlignLast = props.hasProperty(KoSvgTextProperties::TextAlignLastId);
        if (props.hasProperty(KoSvgTextProperties::TextAlignAllId) || textAlignLast) {
            const KoSvgText::TextAlign align = textAlignLast? KoSvgText::TextAlign(props.property(KoSvgTextProperties::TextAlignLastId).toInt())
                                                            : KoSvgText::TextAlign(props.property(KoSvgTextProperties::TextAlignAllId).toInt());

            if (mode == KoSvgText::HorizontalTB) {
                vComponent = Qt::AlignTop;
                if (align == KoSvgText::AlignStart || align == KoSvgText::AlignLastAuto) {
                    if (dir == KoSvgText::DirectionLeftToRight) {
                        hComponent = Qt::AlignLeft;
                    } else {
                        hComponent = Qt::AlignRight;
                    }
                } else if (align == KoSvgText::AlignEnd) {
                    if (dir == KoSvgText::DirectionLeftToRight) {
                        hComponent = Qt::AlignRight;
                    } else {
                        hComponent = Qt::AlignLeft;
                    }
                } else if (align == KoSvgText::AlignLeft) {
                    hComponent = Qt::AlignLeft;
                } else if (align == KoSvgText::AlignRight) {
                    hComponent =  Qt::AlignRight;
                }
            } else {
                hComponent = mode == KoSvgText::VerticalRL? Qt::AlignRight: Qt::AlignLeft;
                if (align == KoSvgText::AlignStart || align == KoSvgText::AlignLastAuto) {
                    if (dir == KoSvgText::DirectionLeftToRight) {
                        vComponent = Qt::AlignTop;
                    } else {
                        vComponent =  Qt::AlignBottom;
                    }
                } else if (align == KoSvgText::AlignEnd) {
                    if (dir == KoSvgText::DirectionLeftToRight) {
                        vComponent =  Qt::AlignBottom;
                    } else {
                        vComponent =  Qt::AlignTop;
                    }
                } else if (align == KoSvgText::AlignLeft) {
                    vComponent =  Qt::AlignTop;
                } else if (align == KoSvgText::AlignRight) {
                    vComponent =  Qt::AlignBottom;
                }
            }
        } else {
            const KoSvgText::TextAnchor anchor = KoSvgText::TextAnchor(props.propertyOrDefault(KoSvgTextProperties::TextAnchorId).toInt());

            if (mode == KoSvgText::HorizontalTB) {
                vComponent = Qt::AlignTop;
                if (anchor == KoSvgText::AnchorStart) {
                    if (dir == KoSvgText::DirectionLeftToRight) {
                        hComponent = Qt::AlignLeft;
                    } else {
                        hComponent = Qt::AlignRight;
                    }
                } else if (anchor == KoSvgText::AnchorEnd) {
                    if (dir == KoSvgText::DirectionLeftToRight) {
                        hComponent = Qt::AlignRight;
                    } else {
                        hComponent = Qt::AlignLeft;
                    }
                } else {
                    hComponent = Qt::AlignHCenter;
                }
            } else {
                hComponent = mode == KoSvgText::VerticalRL? Qt::AlignRight: Qt::AlignLeft;
                if (anchor == KoSvgText::AnchorStart) {
                    if (dir == KoSvgText::DirectionLeftToRight) {
                        vComponent = Qt::AlignTop;
                    } else {
                        vComponent = Qt::AlignBottom;
                    }
                } else if (anchor == KoSvgText::AnchorEnd) {
                    if (dir == KoSvgText::DirectionLeftToRight) {
                        vComponent = Qt::AlignBottom;
                    } else {
                        vComponent = Qt::AlignTop;
                    }
                } else {
                    vComponent = Qt::AlignVCenter;
                }
            }
        }
    }

    addMetaData(SAMPLE_ALIGN, static_cast<Qt::Alignment::Int>(hComponent | vComponent));
}

QString KoCssStylePreset::beforeText() const
{
    // handle paragraph.
    const QVector<int> treeIndex = d->shape->findTreeIndexForPropertyId(KoSvgTextProperties::KraTextStyleType);
    QPair<int, int> pos = d->shape->findRangeForTreeIndex(treeIndex);
    if (treeIndex.isEmpty()) return QString();
    pos.first = d->shape->indexForPos(pos.first);
    return d->shape->plainText().mid(0, pos.first);
}

QString KoCssStylePreset::afterText() const
{
    // handle paragraph
    const QVector<int> treeIndex = d->shape->findTreeIndexForPropertyId(KoSvgTextProperties::KraTextStyleType);
    QPair<int, int> pos = d->shape->findRangeForTreeIndex(treeIndex);
    if (treeIndex.isEmpty()) return QString();
    pos.second = d->shape->indexForPos(pos.second);
    return d->shape->plainText().mid(pos.second);
}

QString KoCssStylePreset::sampleSvg() const
{
    QMap<QString, QVariant> m = metadata();
    return m[SAMPLE_SVG].toString();
}

KoResourceSP KoCssStylePreset::clone() const
{
    return KoResourceSP(new KoCssStylePreset(*this));
}

bool KoCssStylePreset::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(resourcesInterface)
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
    parser.setResolveTextPropertiesForTopLevel(false);
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
            QString styleType = STYLE_TYPE_PARAGRAPH;
            if (!treeIndex.isEmpty()) {
                KoSvgTextProperties props = d->shape->propertiesForTreeIndex(treeIndex);
                styleType = props.property(KoSvgTextProperties::KraTextStyleType).toString();
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

    QMap<QString, QVariant> m = metadata();
    d->shape->setAdditionalAttribute(DESC, m[DESCRIPTION].toString());
    d->shape->setAdditionalAttribute(TITLE, name());

    SvgWriter writer({d->shape.data()});
    return writer.save(*dev, d->shape->boundingRect().size());
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
    updateAlignSample();

    setImage(img);
}

QPair<QString, QString> KoCssStylePreset::resourceType() const
{
    return QPair<QString, QString>(ResourceType::CssStyles, "");
}
