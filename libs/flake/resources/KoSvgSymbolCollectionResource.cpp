/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2017 L. E. Segovia <amy@amyspark.me>


   SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <resources/KoSvgSymbolCollectionResource.h>

#include <QDebug>
#include <QVector>
#include <QFile>
#include <QFileInfo>
#include <QBuffer>
#include <QByteArray>
#include <QImage>
#include <QPainter>

#include <klocalizedstring.h>
#include <KoMarker.h>
#include <KoStore.h>
#include <KoDocumentResourceManager.h>
#include "kis_debug.h"

#include <KoShape.h>
#include <KoShapeGroup.h>
#include <KoShapeManager.h>
#include <KoShapePaintingContext.h>
#include <SvgParser.h>
#include <KoMD5Generator.h>

#include <FlakeDebug.h>

QImage KoSvgSymbol::icon()
{
    KoShapeGroup *group = dynamic_cast<KoShapeGroup*>(shape);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(group, QImage());

    QRectF rc = group->boundingRect().normalized();

    QImage image(rc.width(), rc.height(), QImage::Format_ARGB32_Premultiplied);
    QPainter gc(&image);
    image.fill(Qt::gray);

    KoShapePaintingContext ctx;

//        debugFlake << "Going to render. Original bounding rect:" << group->boundingRect()
//                 << "Normalized: " << rc
//                 << "Scale W" << 256 / rc.width() << "Scale H" << 256 / rc.height();

    gc.translate(-rc.x(), -rc.y());
    KoShapeManager::renderSingleShape(group, gc, ctx);
    gc.end();
    image = image.scaled(128, 128, Qt::KeepAspectRatio);
    return image;
}



struct KoSvgSymbolCollectionResource::Private {
    QVector<KoSvgSymbol*> symbols;
    QString title;
    QString description;
    QByteArray data;
};


KoSvgSymbolCollectionResource::KoSvgSymbolCollectionResource(const QString& filename)
    : KoResource(filename)
    , d(new Private())
{
}

KoSvgSymbolCollectionResource::KoSvgSymbolCollectionResource()
    : KoResource(QString())
    , d(new Private())
{
}

KoSvgSymbolCollectionResource::KoSvgSymbolCollectionResource(const KoSvgSymbolCollectionResource& rhs)
    : KoResource(QString())
    , d(new Private(*rhs.d))
{
    setFilename(rhs.filename());

    Q_FOREACH(KoSvgSymbol *symbol, rhs.d->symbols) {
        d->symbols << new KoSvgSymbol(*symbol);
    }

    setValid(true);
}

KoResourceSP KoSvgSymbolCollectionResource::clone() const
{
    return KoResourceSP(new KoSvgSymbolCollectionResource(*this));
}

KoSvgSymbolCollectionResource::~KoSvgSymbolCollectionResource()
{
    qDeleteAll(d->symbols);
}

bool KoSvgSymbolCollectionResource::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(resourcesInterface);

    if (!dev->isOpen()) {
        dev->open(QIODevice::ReadOnly);
    }

    d->data = dev->readAll();
    setMD5Sum(KoMD5Generator::generateHash(d->data));

    dev->seek(0);

    QString errorMsg;
    int errorLine = 0;
    int errorColumn;

    QDomDocument doc = SvgParser::createDocumentFromSvg(dev, &errorMsg, &errorLine, &errorColumn);
    if (doc.isNull()) {

        errKrita << "Parsing error in " << filename() << "! Aborting!" << endl
        << " In line: " << errorLine << ", column: " << errorColumn << endl
        << " Error message: " << errorMsg << endl;
        errKrita << i18n("Parsing error in the main document at line %1, column %2\nError message: %3"
                         , errorLine , errorColumn , errorMsg);
        return false;
    }

    KoDocumentResourceManager manager;
    SvgParser parser(&manager);
    parser.setResolution(QRectF(0,0,100,100), 72); // initialize with default values
    QSizeF fragmentSize;
    // We're not interested in the shapes themselves
    qDeleteAll(parser.parseSvg(doc.documentElement(), &fragmentSize));
    d->symbols = parser.takeSymbols();
//    debugFlake << "Loaded" << filename() << "\n\t"
//             << "Title" << parser.documentTitle() << "\n\t"
//             << "Description" << parser.documentDescription()
//             << "\n\tgot" << d->symbols.size() << ResourceType::Symbols
//             << d->symbols[0]->shape->outlineRect()
//             << d->symbols[0]->shape->size();

    d->title = parser.documentTitle();
    setName(d->title);
    d->description = parser.documentDescription();

    if (d->symbols.size() < 1) {
        setValid(false);
        return false;
    }
    setValid(true);
    setImage(d->symbols[0]->icon());
    return true;
}

bool KoSvgSymbolCollectionResource::saveToDevice(QIODevice *dev) const
{
    dev->open(QIODevice::WriteOnly);
    dev->write(d->data);
    dev->close();
    return true;
}

QString KoSvgSymbolCollectionResource::defaultFileExtension() const
{
    return QString(".svg");
}

QString KoSvgSymbolCollectionResource::title() const
{
    return d->title;
}

QString KoSvgSymbolCollectionResource::description() const
{
    return d->description;
}

QString KoSvgSymbolCollectionResource::creator() const
{
    return "";
}

QString KoSvgSymbolCollectionResource::rights() const
{
    return "";
}

QString KoSvgSymbolCollectionResource::language() const
{
    return "";
}

QStringList KoSvgSymbolCollectionResource::subjects() const
{
    return QStringList();
}

QString KoSvgSymbolCollectionResource::license() const
{
    return "";
}

QStringList KoSvgSymbolCollectionResource::permits() const
{
    return QStringList();
}

QVector<KoSvgSymbol *> KoSvgSymbolCollectionResource::symbols() const
{
    return d->symbols;
}
