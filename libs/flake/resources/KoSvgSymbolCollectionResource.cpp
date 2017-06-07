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
#include <KoViewConverter.h>
#include <KoShapePaintingContext.h>
#include <SvgParser.h>

struct KoSvgSymbolCollectionResource::Private {
    QVector<KoSvgSymbol> symbols;
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
    : QObject(0)
    , KoResource(QString())
    , d(new Private())
{
    setFilename(rhs.filename());
    d->symbols = rhs.d->symbols;
    setValid(true);
}

KoSvgSymbolCollectionResource::~KoSvgSymbolCollectionResource()
{
}

bool KoSvgSymbolCollectionResource::load()
{
    qDebug() << "Going to load" << filename();

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
    if (!dev->isOpen()) dev->open(QIODevice::ReadOnly);

    KoXmlDocument doc;
    QString errorMsg;
    int errorLine = 0;
    int errorColumn;

    bool ok = doc.setContent(dev->readAll(), false, &errorMsg, &errorLine, &errorColumn);
    if (!ok) {

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
    QList<KoShape*> shapes = parser.parseSvg(doc.documentElement(), &fragmentSize);

    KoViewConverter converter;
    KoShapePaintingContext context;

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
    bool res;
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

