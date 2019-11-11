/*
 *  Copyright (c) 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KoGamutMask.h"

#include <cstring>

#include <QVector>
#include <QString>
#include <QFile>
#include <QList>
#include <QDomDocument>
#include <QDomElement>
#include <QByteArray>
#include <QBuffer>
#include <QScopedPointer>

#include <FlakeDebug.h>

#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoDocumentResourceManager.h>
#include <SvgParser.h>
#include <SvgWriter.h>
#include <KoShape.h>
#include <KisGamutMaskViewConverter.h>
#include <kis_assert.h>

KoGamutMaskShape::KoGamutMaskShape(KoShape* shape)
    : m_maskShape(shape)
    , m_shapePaintingContext(KoShapePaintingContext())
{
}

KoGamutMaskShape::KoGamutMaskShape() {};
KoGamutMaskShape::~KoGamutMaskShape() {};

KoShape* KoGamutMaskShape::koShape()
{
    return m_maskShape;
}

bool KoGamutMaskShape::coordIsClear(const QPointF& coord, const KoViewConverter& viewConverter, int maskRotation) const
{
    // apply mask rotation to coord
    const KisGamutMaskViewConverter& converter = dynamic_cast<const KisGamutMaskViewConverter&>(viewConverter);
    QPointF centerPoint(converter.viewSize().width()*0.5, converter.viewSize().height()*0.5);

    QTransform rotationTransform;
    rotationTransform.translate(centerPoint.x(), centerPoint.y());
    rotationTransform.rotate(-maskRotation);
    rotationTransform.translate(-centerPoint.x(), -centerPoint.y());

    QPointF rotatedCoord = rotationTransform.map(coord);
    QPointF translatedPoint = viewConverter.viewToDocument(rotatedCoord);

    bool isClear = m_maskShape->hitTest(translatedPoint);

    return isClear;
}

void KoGamutMaskShape::paint(QPainter &painter, const KoViewConverter& viewConverter, int maskRotation)
{
    painter.save();

    // apply mask rotation before drawing
    QPointF centerPoint(painter.viewport().width()*0.5, painter.viewport().height()*0.5);
    painter.translate(centerPoint);
    painter.rotate(maskRotation);
    painter.translate(-centerPoint);
    painter.setTransform(m_maskShape->absoluteTransformation(&viewConverter) * painter.transform());
    m_maskShape->paint(painter, viewConverter, m_shapePaintingContext);
    painter.restore();
}

void KoGamutMaskShape::paintStroke(QPainter &painter, const KoViewConverter &viewConverter, int maskRotation)
{
    painter.save();

    // apply mask rotation before drawing
    QPointF centerPoint(painter.viewport().width()*0.5, painter.viewport().height()*0.5);
    painter.translate(centerPoint);
    painter.rotate(maskRotation);
    painter.translate(-centerPoint);

    painter.setTransform(m_maskShape->absoluteTransformation(&viewConverter) * painter.transform());
    m_maskShape->paintStroke(painter, viewConverter, m_shapePaintingContext);
    painter.restore();

}

struct KoGamutMask::Private {
    QString name;
    QString title;
    QString description;
    QByteArray data;
    QVector<KoGamutMaskShape*> maskShapes;
    QVector<KoGamutMaskShape*> previewShapes;
    QSizeF maskSize;
    int rotation {0};
};

KoGamutMask::KoGamutMask(const QString& filename)
    : KoResource(filename)
    , d(new Private)
{
    d->maskSize = QSizeF(144.0,144.0);
    setRotation(0);
}

KoGamutMask::KoGamutMask()
    : KoResource(QString())
    , d(new Private)
{
    d->maskSize = QSizeF(144.0,144.0);
    setRotation(0);
}

KoGamutMask::KoGamutMask(KoGamutMask* rhs)
    : QObject(0)
    , KoResource(QString())
    , d(new Private)
{
    *this = *rhs;
}

KoGamutMask::KoGamutMask(const KoGamutMask &rhs)
    : QObject(0)
    , KoResource(rhs)
    , d(new Private)
{
    *this = rhs;
}

KoGamutMask &KoGamutMask::operator=(const KoGamutMask &rhs)
{
    if (*this != rhs) {
        setTitle(rhs.title());
        setDescription(rhs.description());
        d->maskSize = rhs.d->maskSize;

        QList<KoShape*> newShapes;
        for(KoShape* sh: rhs.koShapes()) {
            newShapes.append(sh->cloneShape());
        }
        setMaskShapes(newShapes);
    }
    return *this;
}

KoResourceSP KoGamutMask::clone() const
{
    return KoResourceSP(new KoGamutMask(*this));
}

KoGamutMask::~KoGamutMask()
{
    delete d;
}

bool KoGamutMask::coordIsClear(const QPointF& coord, KoViewConverter &viewConverter, bool preview)
{
    QVector<KoGamutMaskShape*>* shapeVector;

    if (preview && !d->previewShapes.isEmpty()) {
        shapeVector = &d->previewShapes;
    } else {
        shapeVector = &d->maskShapes;
    }

    for(KoGamutMaskShape* shape: *shapeVector) {
        if (shape->coordIsClear(coord, viewConverter, rotation()) == true) {
            return true;
        }
    }

    return false;
}

void KoGamutMask::paint(QPainter &painter, KoViewConverter& viewConverter, bool preview)
{
    QVector<KoGamutMaskShape*>* shapeVector;

    if (preview && !d->previewShapes.isEmpty()) {
        shapeVector = &d->previewShapes;
    } else {
        shapeVector = &d->maskShapes;
    }

    for(KoGamutMaskShape* shape: *shapeVector) {
        shape->paint(painter, viewConverter, rotation());
    }
}

void KoGamutMask::paintStroke(QPainter &painter, KoViewConverter &viewConverter, bool preview)
{
    QVector<KoGamutMaskShape*>* shapeVector;

    if (preview && !d->previewShapes.isEmpty()) {
        shapeVector = &d->previewShapes;
    } else {
        shapeVector = &d->maskShapes;
    }

    for(KoGamutMaskShape* shape: *shapeVector) {
        shape->paintStroke(painter, viewConverter, rotation());
    }
}

bool KoGamutMask::load()
{
    QFile file(filename());
    if (file.size() == 0) return false;
    if (!file.open(QIODevice::ReadOnly)) {
        warnFlake << "Can't open file " << filename();
        return false;
    }
    bool res = loadFromDevice(&file);
    file.close();
    return res;
}

bool KoGamutMask::loadFromDevice(QIODevice *dev)
{
    if (!dev->isOpen()) dev->open(QIODevice::ReadOnly);

    d->data = dev->readAll();

    // TODO: test
    KIS_ASSERT_RECOVER_RETURN_VALUE(d->data.size() != 0, false);

    if (filename().isNull()) {
        warnFlake << "Cannot load gamut mask" << name() << "there is no filename set";
        return false;
    }

    if (d->data.isNull()) {
        QFile file(filename());
        if (file.size() == 0) {
            warnFlake << "Cannot load gamut mask" << name() << "there is no data available";
            return false;
        }

        file.open(QIODevice::ReadOnly);
        d->data = file.readAll();
        file.close();
    }

    QBuffer buf(&d->data);
    buf.open(QBuffer::ReadOnly);

    QScopedPointer<KoStore> store(KoStore::createStore(&buf, KoStore::Read, "application/x-krita-gamutmask", KoStore::Zip));
    if (!store || store->bad()) return false;

    bool storeOpened = store->open("gamutmask.svg");
    if (!storeOpened) { return false; }

    QByteArray data;
    data.resize(store->size());
    QByteArray ba = store->read(store->size());
    store->close();

    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    KoXmlDocument xmlDocument = SvgParser::createDocumentFromSvg(ba, &errorMsg, &errorLine, &errorColumn);
    if (xmlDocument.isNull()) {

        errorFlake << "Parsing error in " << filename() << "! Aborting!" << endl
        << " In line: " << errorLine << ", column: " << errorColumn << endl
        << " Error message: " << errorMsg << endl;
        errorFlake << "Parsing error in the main document at line" << errorLine
                   << ", column" << errorColumn << endl
                   << "Error message: " << errorMsg;

        return false;
    }

    KoDocumentResourceManager manager;
    SvgParser parser(&manager);
    parser.setResolution(QRectF(0,0,100,100), 72); // initialize with default values
    QSizeF fragmentSize;

    QList<KoShape*> shapes = parser.parseSvg(xmlDocument.documentElement(), &fragmentSize);

    d->maskSize = fragmentSize;

    d->title = parser.documentTitle();
    setName(d->title);
    d->description = parser.documentDescription();

    setMaskShapes(shapes);

    if (store->open("preview.png")) {
        KoStoreDevice previewDev(store.data());
        previewDev.open(QIODevice::ReadOnly);

        QImage preview = QImage();
        preview.load(&previewDev, "PNG");
        setImage(preview);

        (void)store->close();
    }

    buf.close();

    setValid(true);

    return true;
}

void KoGamutMask::setMaskShapes(QList<KoShape*> shapes)
{
    setMaskShapesToVector(shapes, d->maskShapes);
}

bool KoGamutMask::save()
{
    QFile file(filename());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    saveToDevice(&file);
    file.close();

    return true;
}

QList<KoShape*> KoGamutMask::koShapes() const
{
    QList<KoShape*> shapes;
    for(KoGamutMaskShape* maskShape: d->maskShapes) {
        shapes.append(maskShape->koShape());
    }

    return shapes;
}

bool KoGamutMask::saveToDevice(QIODevice *dev) const
{
    KoStore* store(KoStore::createStore(dev, KoStore::Write, "application/x-krita-gamutmask", KoStore::Zip));
    if (!store || store->bad()) return false;

    QList<KoShape*> shapes = koShapes();

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);

    if (!store->open("gamutmask.svg")) {
        return false;
    }

    KoStoreDevice storeDev(store);
    storeDev.open(QIODevice::WriteOnly);

    SvgWriter writer(shapes);
    writer.setDocumentTitle(d->title);
    writer.setDocumentDescription(d->description);

    writer.save(storeDev, d->maskSize);

    if (!store->close()) { return false; }


    if (!store->open("preview.png")) {
        return false;
    }

    KoStoreDevice previewDev(store);
    previewDev.open(QIODevice::WriteOnly);

    image().save(&previewDev, "PNG");
    if (!store->close()) { return false; }

    return store->finalize();
}

QString KoGamutMask::title() const
{
    return d->title;
}

void KoGamutMask::setTitle(QString title)
{
    d->title = title;
    setName(title);
}

QString KoGamutMask::description() const
{
    return d->description;
}

void KoGamutMask::setDescription(QString description)
{
    d->description = description;
}

QString KoGamutMask::defaultFileExtension() const
{
    return ".kgm";
}

int KoGamutMask::rotation()
{
    return d->rotation;
}

void KoGamutMask::setRotation(int rotation)
{
    d->rotation = rotation;
}

QSizeF KoGamutMask::maskSize()
{
    return d->maskSize;
}

void KoGamutMask::setPreviewMaskShapes(QList<KoShape*> shapes)
{
    setMaskShapesToVector(shapes, d->previewShapes);
}

void KoGamutMask::setMaskShapesToVector(QList<KoShape *> shapes, QVector<KoGamutMaskShape *> &targetVector)
{
    targetVector.clear();

    for(KoShape* sh: shapes) {
        KoGamutMaskShape* maskShape = new KoGamutMaskShape(sh);
        targetVector.append(maskShape);
    }
}

// clean up when ending mask preview
void KoGamutMask::clearPreview()
{
    d->previewShapes.clear();
}
