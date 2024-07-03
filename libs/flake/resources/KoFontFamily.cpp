/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoFontFamily.h"
#include <KoMD5Generator.h>

struct KoFontFamily::Private {
};

KoFontFamily::KoFontFamily(KoFontFamilyWWSRepresentation representation)
    : KoResource(representation.fontFamilyName)
    , d(new Private())

{
    setName(representation.fontFamilyName);

    QImage img(256, 256, QImage::Format_ARGB32);
    /*QPainter gc(&img);
    gc.fillRect(0, 0, 256, 256, Qt::white);
    gc.end();*/
    setImage(img);

    QVariantHash axes;
    Q_FOREACH(const QString key, representation.axes.keys()) {
        axes.insert(key, QVariant::fromValue(representation.axes.value(key)));
    }
    addMetaData("axes", axes);
    QVariantList styles;
    Q_FOREACH(const KoSvgText::FontFamilyStyleInfo style, representation.styles) {
        styles.append(QVariant::fromValue(style));
    }
    addMetaData("styles", styles);
    setMD5Sum(KoMD5Generator::generateHash(representation.fontFamilyName.toUtf8()));
    qDebug() << representation.fontFamilyName << md5Sum(false);
    setValid(true);
}

KoFontFamily::KoFontFamily(const QString &filename)
    :KoResource(filename)
{
    qDebug() << "empty";
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

/*bool KoFontFamily::isEphemeral() const
{
    return true;
}*/

QPair<QString, QString> KoFontFamily::resourceType() const
{
    return QPair<QString, QString>(ResourceType::FontFamilies, "");
}

QString KoFontFamily::translatedFontName(QList<QLocale> locales)
{
    return filename();
}

QString KoFontFamily::translatedTypographicName(QList<QLocale> locales)
{
    return filename();
}

QList<KoSvgText::FontFamilyAxis> KoFontFamily::axes() const
{
    QVariantHash axes = metadata().value("axes").toHash();
    QList<KoSvgText::FontFamilyAxis> converted;
    Q_FOREACH(const QString key, axes.keys()) {
        converted.append(axes.value(key).value<KoSvgText::FontFamilyAxis>());
    }
    return converted;
}

QList<KoSvgText::FontFamilyStyleInfo> KoFontFamily::styles() const
{
    QVariantList styles = metadata().value("styles").toList();
    QList<KoSvgText::FontFamilyStyleInfo> converted;
    Q_FOREACH(const QVariant val, styles) {
        converted.append(val.value<KoSvgText::FontFamilyStyleInfo>());
    }
    return converted;
}
