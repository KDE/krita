/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoFontFamily.h"
#include <KoMD5Generator.h>
#include <KoSvgTextShape.h>
#include <KoColorBackground.h>
#include <SvgWriter.h>
#include <QPainter>
#include <QBuffer>
#include <KoShapePainter.h>

const QString TYPOGRAPHIC_NAME = "typographic_name";
const QString LOCALIZED_TYPOGRAPHIC_NAME = "localized_typographic_name";
const QString LOCALIZED_TYPOGRAPHIC_STYLE = "localized_typographic_style";
const QString LOCALIZED_FONT_FAMILY = "localized_font_family";
const QString AXES = "axes";
const QString STYLES = "styles";
const QString FONT_TYPE = "font_type";
const QString IS_VARIABLE = "is_variable";
const QString COLOR_BITMAP = "color_bitmap";
const QString COLOR_CLRV0 = "color_clrv0";
const QString COLOR_CLRV1 = "color_clrv1";
const QString COLOR_SVG = "color_svg";
const QString SAMPLE_STRING = "sample_string";
const QString SAMPLE_SVG = "sample_svg";
const QString SAMPLE_BBOX = "sample_bbox";
const QString LAST_MODIFIED = "last_modified";
const QString SUPPORTED_LANGUAGES = "supported_languages";

struct KoFontFamily::Private {
};

QImage generateImage(const QString &sample, const QString &fontFamily, bool isColor) {
    QSharedPointer<KoSvgTextShape> shape(new KoSvgTextShape);
    shape->setResolution(300, 300);
    shape->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(Qt::black)));
    KoSvgTextProperties props = shape->textProperties();
    KoSvgText::CssLengthPercentage fontSize(12.0);
    props.setProperty(KoSvgTextProperties::FontSizeId, QVariant::fromValue(fontSize));
    props.setProperty(KoSvgTextProperties::FontFamiliesId, {fontFamily});
    shape->setPropertiesAtPos(-1, props);
    shape->insertText(0, sample);

    QImage img(256,
               256,
               isColor? QImage::Format_ARGB32: QImage::Format_Grayscale8);
    img.fill(Qt::white);

    KoShapePainter painter;
    painter.setShapes({shape.data()});
    painter.paint(img);

    return img;
}

QString generateSVG(const QString &sample, const QString &fontFamily, QRectF &layoutBox, const QString &lang) {
    QSharedPointer<KoSvgTextShape> shape(new KoSvgTextShape);
    shape->setResolution(300, 300);
    shape->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(Qt::black)));
    KoSvgTextProperties props = shape->textProperties();
    props.setProperty(KoSvgTextProperties::FontFamiliesId, {fontFamily});
    KoSvgText::CssLengthPercentage fontSize(12.0);
    props.setProperty(KoSvgTextProperties::FontSizeId, QVariant::fromValue(fontSize));
    if (!lang.isEmpty()) {
        props.setProperty(KoSvgTextProperties::TextLanguage, lang);
    }
    shape->setPropertiesAtPos(-1, props);
    shape->insertText(0, sample);

    layoutBox = shape->selectionBoxes(0, shape->posDown(0)).boundingRect();

    SvgWriter writer({shape->textOutline()});
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    writer.save(buffer, shape->boundingRect().size());
    buffer.close();

    return QString::fromUtf8(buffer.data());
}

QMap<QString, QVariant> localeHashtoVariantMap(QHash<QLocale, QString> names) {
    // Needs to be a map to be accessible in qml (see https://bugreports.qt.io/browse/QTBUG-62014)
    QMap<QString, QVariant> newNames;
    Q_FOREACH(const QLocale key, names.keys()) {
        newNames.insert(key.bcp47Name(), QVariant::fromValue(names.value(key)));
    }
    return newNames;
}

KoFontFamily::KoFontFamily(KoFontFamilyWWSRepresentation representation)
    : KoResource(representation.fontFamilyName)
    , d(new Private())

{
    setName(representation.fontFamilyName);
    addMetaData(TYPOGRAPHIC_NAME, representation.typographicFamilyName);
    addMetaData(LOCALIZED_FONT_FAMILY, localeHashtoVariantMap(representation.localizedFontFamilyNames));
    addMetaData(LOCALIZED_TYPOGRAPHIC_NAME, localeHashtoVariantMap(representation.localizedTypographicFamily));
    addMetaData(LOCALIZED_TYPOGRAPHIC_STYLE, localeHashtoVariantMap(representation.localizedTypographicStyles));

    addMetaData(LAST_MODIFIED, representation.lastModified);

    QMap<QString, QVariant> samples;
    Q_FOREACH(const QString key, representation.sampleStrings.keys()) {
        samples.insert(key, QVariant::fromValue(representation.sampleStrings.value(key)));
    }
    addMetaData(SAMPLE_STRING, samples);
    QList<QVariant> supportedLanguages;
    Q_FOREACH(const QLocale l, representation.supportedLanguages) {
        QString val = l.bcp47Name();
        val.replace("_", "-");
        supportedLanguages.append(QVariant::fromValue(val));
    }
    addMetaData(SUPPORTED_LANGUAGES, supportedLanguages);

    addMetaData(FONT_TYPE, representation.type);
    addMetaData(IS_VARIABLE, representation.isVariable);
    addMetaData(COLOR_BITMAP, representation.colorBitMap);
    addMetaData(COLOR_CLRV0, representation.colorClrV0);
    addMetaData(COLOR_CLRV1, representation.colorClrV1);
    addMetaData(COLOR_SVG, representation.colorSVG);
    QVariantHash axes;
    Q_FOREACH(const QString key, representation.axes.keys()) {
        axes.insert(key, QVariant::fromValue(representation.axes.value(key)));
    }
    addMetaData(AXES, axes);
    QVariantList styles;
    Q_FOREACH(const KoSvgText::FontFamilyStyleInfo style, representation.styles) {
        styles.append(QVariant::fromValue(style));
    }
    addMetaData(STYLES, styles);
    setMD5Sum(KoMD5Generator::generateHash(representation.fontFamilyName.toUtf8()));
    setValid(true);
}

KoFontFamily::KoFontFamily(const QString &filename)
    :KoResource(filename)
{
    setMD5Sum(KoMD5Generator::generateHash(ResourceType::FontFamilies.toUtf8()));
    setValid(false);
}

KoFontFamily::~KoFontFamily()
{
}

KoFontFamily::KoFontFamily(const KoFontFamily &rhs)
    : KoResource(QString())
    , d(new Private(*rhs.d))
{
    setFilename(rhs.filename());
    QMap<QString, QVariant> meta = metadata();
    Q_FOREACH(const QString key, meta.keys()) {
        addMetaData(key, meta.value(key));
    }
    setValid(true);
}

KoResourceSP KoFontFamily::clone() const
{
    return KoResourceSP(new KoFontFamily(*this));
}

bool KoFontFamily::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(dev)
    Q_UNUSED(resourcesInterface);
    return false;
}

bool KoFontFamily::isSerializable() const
{
    return false;
}

QPair<QString, QString> KoFontFamily::resourceType() const
{
    return QPair<QString, QString>(ResourceType::FontFamilies, "");
}

void KoFontFamily::updateThumbnail()
{
    QHash<QString, QVariant> samples = metadata().value(SAMPLE_STRING).toHash();
    QMap<QString, QVariant> sampleSVG;
    QMap<QString, QVariant> sampleSVGBbox;

    Q_FOREACH (const QString key, samples.keys()) {
        QString sample = samples.value(key).toString();
        QRectF sampleBBox;
        QString lang = key.startsWith("l_")? key.right(key.size() - 2): "";
        sampleSVG.insert(key, QVariant::fromValue(generateSVG(sample, filename(), sampleBBox, lang)));
        sampleSVGBbox.insert(key, QVariant::fromValue(sampleBBox));
    }

    addMetaData(SAMPLE_SVG, sampleSVG);
    addMetaData(SAMPLE_BBOX, sampleSVGBbox);
    const QString sample = samples.isEmpty()? "AaBbGg"
                                            : samples.value("s_Latn", samples.values().first()).toString();
    bool isColor = (metadata().value(COLOR_BITMAP).toBool() || metadata().value(COLOR_CLRV0).toBool());
    setImage(generateImage(sample, filename(), isColor));
}

QString KoFontFamily::typographicFamily() const
{
    return metadata().value(TYPOGRAPHIC_NAME).toString();
}

QString KoFontFamily::translatedFontName(QStringList locales) const
{
    QMap<QString, QVariant> names = metadata().value(LOCALIZED_FONT_FAMILY).toMap();
    QString name = filename();
    Q_FOREACH(const QString locale, locales) {
        if (names.keys().contains(locale)) {
            name = names.value(locale).toString();
            break;
        }
    }
    return name;
}

bool KoFontFamily::isVariable() const
{
    return metadata().value(IS_VARIABLE).toBool();
}

bool KoFontFamily::colorBitmap() const
{
    return metadata().value(COLOR_BITMAP).toBool();
}

bool KoFontFamily::colorClrV0() const
{
    return metadata().value(COLOR_CLRV0).toBool();
}

bool KoFontFamily::colorClrV1() const
{
    return metadata().value(COLOR_CLRV1).toBool();
}

bool KoFontFamily::colorSVG() const
{
    return metadata().value(COLOR_SVG).toBool();
}

QList<KoSvgText::FontFamilyAxis> KoFontFamily::axes() const
{
    QVariantHash axes = metadata().value(AXES).toHash();
    QList<KoSvgText::FontFamilyAxis> converted;
    Q_FOREACH(const QString key, axes.keys()) {
        converted.append(axes.value(key).value<KoSvgText::FontFamilyAxis>());
    }
    return converted;
}

QList<KoSvgText::FontFamilyStyleInfo> KoFontFamily::styles() const
{
    QVariantList styles = metadata().value(STYLES).toList();
    QList<KoSvgText::FontFamilyStyleInfo> converted;
    Q_FOREACH(const QVariant val, styles) {
        converted.append(val.value<KoSvgText::FontFamilyStyleInfo>());
    }
    return converted;
}

QDateTime KoFontFamily::lastModified() const
{
    return metadata().value(LAST_MODIFIED).toDateTime();
}
