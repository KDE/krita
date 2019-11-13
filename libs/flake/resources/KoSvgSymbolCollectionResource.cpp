/* This file is part of the KDE project

   Copyright (c) 2017 L. E. Segovia <leo.segovia@siggraph.org>


   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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

#include <KoStore.h>
#include <KoDocumentResourceManager.h>
#include "kis_debug.h"

#include <KoShape.h>
#include <KoShapeGroup.h>
#include <KoShapeManager.h>
#include <KoViewConverter.h>
#include <KoShapePaintingContext.h>
#include <SvgParser.h>
#include <KoHashGenerator.h>
#include <KoHashGeneratorProvider.h>

#include <FlakeDebug.h>

void paintGroup(KoShapeGroup *group, QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintContext)
{
    QList<KoShape*> shapes = group->shapes();
    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    Q_FOREACH (KoShape *child, shapes) {
        // we paint recursively here, so we do not have to check recursively for visibility
        if (!child->isVisible(false))
            continue;
        KoShapeGroup *childGroup = dynamic_cast<KoShapeGroup*>(child);
        if (childGroup) {
            paintGroup(childGroup, painter, converter, paintContext);
        } else {
            painter.save();
            KoShapeManager::renderSingleShape(child, painter, converter, paintContext);
            painter.restore();
        }
    }

}

QImage KoSvgSymbol::icon()
{
    KoShapeGroup *group = dynamic_cast<KoShapeGroup*>(shape);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(group, QImage());

    QRectF rc = group->boundingRect().normalized();

    QImage image(rc.width(), rc.height(), QImage::Format_ARGB32_Premultiplied);
    QPainter gc(&image);
    image.fill(Qt::gray);

    KoViewConverter vc;
    KoShapePaintingContext ctx;

//        debugFlake << "Going to render. Original bounding rect:" << group->boundingRect()
//                 << "Normalized: " << rc
//                 << "Scale W" << 256 / rc.width() << "Scale H" << 256 / rc.height();

    gc.translate(-rc.x(), -rc.y());
    paintGroup(group, gc, vc, ctx);
    gc.end();
    image = image.scaled(128, 128, Qt::KeepAspectRatio);
    return image;
}



struct KoSvgSymbolCollectionResource::Private {
    QVector<KoSvgSymbol*> symbols;
    QString title;
    QString description;
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
    , d(new Private())
{
    *this = rhs;
}

KoSvgSymbolCollectionResource &KoSvgSymbolCollectionResource::operator=(const KoSvgSymbolCollectionResource &rhs)
{
    if (*this != rhs) {
        d->symbols = rhs.d->symbols;
        d->title = rhs.d->title;
        d->description = rhs.d->description;
    }
    return *this;
}

KoResourceSP KoSvgSymbolCollectionResource::clone() const
{
    return KoResourceSP(new KoSvgSymbolCollectionResource(*this));
}

KoSvgSymbolCollectionResource::~KoSvgSymbolCollectionResource()
{
}

bool KoSvgSymbolCollectionResource::load()
{
    QFile file(filename());
    if (file.size() == 0) return false;
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    bool res =  loadFromDevice(&file);
    file.close();
    return res;
}



bool KoSvgSymbolCollectionResource::loadFromDevice(QIODevice *dev)
{
    if (!dev->isOpen()) {
        dev->open(QIODevice::ReadOnly);
    }

    QByteArray ba = dev->readAll();
    KoHashGenerator *hashGenerator = KoHashGeneratorProvider::instance()->getGenerator("MD5");
    setMD5(hashGenerator->generateHash(ba));

    dev->seek(0);

    QString errorMsg;
    int errorLine = 0;
    int errorColumn;

    KoXmlDocument doc = SvgParser::createDocumentFromSvg(dev, &errorMsg, &errorLine, &errorColumn);
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

bool KoSvgSymbolCollectionResource::save()
{
    QFile file(filename());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    saveToDevice(&file);
    file.close();
    return true;
}

bool KoSvgSymbolCollectionResource::saveToDevice(QIODevice *dev) const
{
    bool res = false;
    // XXX
    if (res) {
        KoResource::saveToDevice(dev);
    }
    return res;
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
