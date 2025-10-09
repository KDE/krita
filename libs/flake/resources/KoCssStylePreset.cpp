/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoCssStylePreset.h"

#include <KoShapePainter.h>
#include <KoSvgTextShape.h>
#include <KoDocumentResourceManager.h>
#include <KLocalizedString>

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
const QString STORED_PPI = "stored_ppi";
const QString PRIMARY_FONT_FAMILY = "primary_font_family";

const QString STYLE_TYPE_PARAGRAPH = "paragraph";
const QString STYLE_TYPE_CHARACTER = "character";
const KLocalizedString SAMPLE_PLACEHOLDER = ki18nc("info:placeholder", "Style Sample");

struct KoCssStylePreset::Private {

    Private()
    {
    }
    ~Private() {}

    KoSvgTextProperties properties;
    QString beforeText;
    QString sample = SAMPLE_PLACEHOLDER.toString();
    QString afterText;
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
    d->properties = rhs.d->properties;
    d->sample = rhs.d->sample;
    d->afterText = rhs.d->afterText;
    d->beforeText = rhs.d->beforeText;
    setDescription(rhs.description());
    updateThumbnail();
    setValid(true);
}

KoCssStylePreset::~KoCssStylePreset()
{

}

KoSvgTextProperties KoCssStylePreset::properties(int ppi, bool removeKraProps) const
{
    KoSvgTextProperties props = d->properties;
    const int storedPPI = storedPPIResolution();
    if (storedPPI > 0 && ppi > 0) {
        const double scale = double(storedPPI)/double(ppi);
        props.scaleAbsoluteValues(scale, scale);
    }
    if (removeKraProps) {
        props.removeProperty(KoSvgTextProperties::KraTextStyleType);
        props.removeProperty(KoSvgTextProperties::KraTextStyleResolution);
        props.removeProperty(KoSvgTextProperties::KraTextVersionId);
    }
    // remove fill and stroke for now.
    props.removeProperty(KoSvgTextProperties::FillId);
    props.removeProperty(KoSvgTextProperties::StrokeId);
    props.removeProperty(KoSvgTextProperties::Opacity);
    props.removeProperty(KoSvgTextProperties::Visiblity);
    props.removeProperty(KoSvgTextProperties::TextOrientationId);
    props.removeProperty(KoSvgTextProperties::PaintOrder);
    return props;
}

void KoCssStylePreset::setProperties(const KoSvgTextProperties &properties)
{
    if (d->properties == properties)
        return;
    d->properties = properties;
    QStringList fonts = d->properties.property(KoSvgTextProperties::FontFamiliesId).toStringList();
    //TODO: Apparantly we cannot remove metadata, only set it to nothing...
    addMetaData(PRIMARY_FONT_FAMILY, fonts.value(0));
    setValid(true);
    setDirty(true);
}

QString KoCssStylePreset::description() const
{
    QMap<QString, QVariant> m = metadata();
    return m[DESCRIPTION].toString();
}

void KoCssStylePreset::setDescription(const QString &desc)
{
    QMap<QString, QVariant> m = metadata();
    if (m[DESCRIPTION].toString() == desc) return;
    addMetaData(DESCRIPTION, desc);
    setDirty(true);
}

QString KoCssStylePreset::styleType() const
{
    QMap<QString, QVariant> m = metadata();
    return m.value(STYLE_TYPE, STYLE_TYPE_PARAGRAPH).toString();
}

void KoCssStylePreset::setStyleType(const QString &type)
{
    QMap<QString, QVariant> m = metadata();
    if (m[STYLE_TYPE].toString() == type) return;
    addMetaData(STYLE_TYPE, type);
    setDirty(true);
}

QString KoCssStylePreset::sampleText() const
{
    return d->sample;
}

void KoCssStylePreset::setSampleText(const QString &text)
{
    if (d->sample == text) return;
    d->sample = text;
    setDirty(true);
}

KoShape* KoCssStylePreset::generateSampleShape() const
{
    KoSvgTextProperties modifiedProps = d->properties;
    const QString sample = d->sample;
    const QString after = d->afterText;
    const QString before = d->beforeText;

    QScopedPointer<KoSvgTextShape> sampleText(new KoSvgTextShape());
    sampleText->insertText(0, sample.isEmpty()? name().isEmpty()? SAMPLE_PLACEHOLDER.toString(): name(): sample);
    const QString type = styleType().isEmpty()? STYLE_TYPE_CHARACTER: styleType();

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
    if (storedPPIResolution() > 0) {
        modifiedProps.setProperty(KoSvgTextProperties::KraTextStyleResolution, storedPPIResolution());
    } else {
        modifiedProps.removeProperty(KoSvgTextProperties::KraTextStyleResolution);
    }
    // Always remove inline size, it is shape-specific.
    modifiedProps.removeProperty(KoSvgTextProperties::InlineSizeId);
    // Remove fill and stroke for now as we have no widgets for them.
    modifiedProps.removeProperty(KoSvgTextProperties::FillId);
    modifiedProps.removeProperty(KoSvgTextProperties::StrokeId);
    modifiedProps.removeProperty(KoSvgTextProperties::PaintOrder);
    modifiedProps.removeProperty(KoSvgTextProperties::Opacity);
    modifiedProps.removeProperty(KoSvgTextProperties::Visiblity);
    modifiedProps.removeProperty(KoSvgTextProperties::TextOrientationId);

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

        return sampleText.take();
    } else {
        /// For character the sample always needs to be set to be a child to
        /// ensure that the character property doesn't include the default props.
        QScopedPointer<KoSvgTextShape> newShape(new KoSvgTextShape());
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

        newShape->insertRichText(newShape->posForIndex(before.size()), sampleText.data());
        return newShape.take();
    }

    return nullptr;
}

Qt::Alignment KoCssStylePreset::alignSample() const
{
    QMap<QString, QVariant> m = metadata();
    QVariant v = m.value(SAMPLE_ALIGN, static_cast<Qt::Alignment::Int>(Qt::AlignHCenter | Qt::AlignVCenter));
    return static_cast<Qt::Alignment>(v.value<Qt::Alignment::Int>());
}

QString KoCssStylePreset::primaryFontFamily() const
{
    QMap<QString, QVariant> m = metadata();
    return m.value(PRIMARY_FONT_FAMILY).toString();
}

void KoCssStylePreset::updateAlignSample()
{
    Qt::AlignmentFlag hComponent = Qt::AlignHCenter;
    Qt::AlignmentFlag vComponent = Qt::AlignVCenter;

    const KoSvgTextProperties props = d->properties;
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
    return d->beforeText;
}

void KoCssStylePreset::setBeforeText(const QString &text)
{
    if (d->beforeText == text) return;
    d->beforeText = text;
    setDirty(true);
}

QString KoCssStylePreset::afterText() const
{
    return d->afterText;
}

void KoCssStylePreset::setAfterText(const QString &text)
{
    if (d->afterText == text) return;
    d->afterText = text;
    setDirty(true);
}

QString KoCssStylePreset::sampleSvg() const
{
    QMap<QString, QVariant> m = metadata();
    return m[SAMPLE_SVG].toString();
}

int KoCssStylePreset::storedPPIResolution() const
{
    QMap<QString, QVariant> m = metadata();
    return m.value(STORED_PPI, 0).toInt();
}

void KoCssStylePreset::setStoredPPIResolution(const int ppi)
{
    addMetaData(STORED_PPI, ppi);
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
            setName(textShape->additionalAttribute(TITLE));
            addMetaData(DESCRIPTION, textShape->additionalAttribute(DESC));
            KoSvgTextNodeIndex node = textShape->findNodeIndexForPropertyId(KoSvgTextProperties::KraTextStyleType);
            QString styleType = STYLE_TYPE_PARAGRAPH;
            if (node.properties()) {
                KoSvgTextProperties props = *(node.properties());
                styleType = props.property(KoSvgTextProperties::KraTextStyleType).toString();
                if (props.hasProperty(KoSvgTextProperties::KraTextStyleResolution)) {
                    addMetaData(STORED_PPI, props.property(KoSvgTextProperties::KraTextStyleResolution));
                }
                if (props.hasProperty(KoSvgTextProperties::FontFamiliesId)) {
                    QStringList fonts = props.property(KoSvgTextProperties::FontFamiliesId).toStringList();
                    addMetaData(PRIMARY_FONT_FAMILY, fonts.value(0));
                }
                setProperties(props);

                QPair<int, int> pos = textShape->findRangeForNodeIndex(node);
                pos.first = textShape->indexForPos(pos.first);
                pos.second = textShape->indexForPos(pos.second);

                d->sample = textShape->plainText().mid(pos.first, pos.second-pos.first);
                d->beforeText = textShape->plainText().mid(0, pos.first);
                d->afterText = textShape->plainText().mid(pos.second);
            }

            addMetaData(STYLE_TYPE, styleType);

            updateThumbnail();
            setValid(true);
            return true;
        }
    }


    return false;
}

bool KoCssStylePreset::saveToDevice(QIODevice *dev) const
{
    QScopedPointer<KoShape> shape(generateSampleShape());
    if (!shape) return false;

    QMap<QString, QVariant> m = metadata();
    shape->setAdditionalAttribute(DESC, m[DESCRIPTION].toString());
    shape->setAdditionalAttribute(TITLE, name());

    const QRectF boundingRect = shape->boundingRect();
    SvgWriter writer({shape.take()});
    return writer.save(*dev, boundingRect.size());
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
    QScopedPointer<KoSvgTextShape> shape (dynamic_cast<KoSvgTextShape*>(generateSampleShape()));
    if (!shape) return;
    QImage img(256,
               256,
               QImage::Format_ARGB32);
    img.fill(Qt::white);

    KoShapePainter painter;
    painter.setShapes({shape.data()});
    painter.paint(img);

    /// generate SVG sample.
    addMetaData(SAMPLE_SVG, generateSVG(shape.data()));
    updateAlignSample();

    setImage(img);
}

QPair<QString, QString> KoCssStylePreset::resourceType() const
{
    return QPair<QString, QString>(ResourceType::CssStyles, "");
}
