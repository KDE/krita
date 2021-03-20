/*
 *  SPDX-FileCopyrightText: 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <kis_assert.h>
#include <QTransform>

//#include <kis_debug.h>

KoGamutMaskShape::KoGamutMaskShape(KoShape* shape)
    : m_maskShape(shape)
    , m_shapePaintingContext(KoShapePaintingContext())
{
}

KoGamutMaskShape::KoGamutMaskShape()
{
};

KoGamutMaskShape::~KoGamutMaskShape()
{
    delete m_maskShape;
};

KoShape* KoGamutMaskShape::koShape()
{
    return m_maskShape;
}

bool KoGamutMaskShape::coordIsClear(const QPointF& coord) const
{
    bool isClear = m_maskShape->hitTest(coord);

    return isClear;
}

void KoGamutMaskShape::paint(QPainter &painter)
{
    painter.save();
    painter.setTransform(m_maskShape->absoluteTransformation(), true);
    m_maskShape->paint(painter, m_shapePaintingContext);
    painter.restore();
}

void KoGamutMaskShape::paintStroke(QPainter &painter)
{
    painter.save();
    painter.setTransform(m_maskShape->absoluteTransformation(), true);
    m_maskShape->paintStroke(painter, m_shapePaintingContext);
    painter.restore();
}

struct KoGamutMask::Private {
    QString name;
    QString title;
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
    : KoGamutMask(*rhs)
{
}

KoGamutMask::KoGamutMask(const KoGamutMask &rhs)
    : QObject(0)
    , KoResource(rhs)
    , d(new Private)
{
    setTitle(rhs.title());
    setDescription(rhs.description());
    d->maskSize = rhs.d->maskSize;

    QList<KoShape*> newShapes;
    for(KoShape* sh: rhs.koShapes()) {
        newShapes.append(sh->cloneShape());
    }
    setMaskShapes(newShapes);
}

KoResourceSP KoGamutMask::clone() const
{
    return KoResourceSP(new KoGamutMask(*this));
}

KoGamutMask::~KoGamutMask()
{
    qDeleteAll(d->maskShapes);
    qDeleteAll(d->previewShapes);
    delete d;
}

bool KoGamutMask::coordIsClear(const QPointF& coord, bool preview)
{
    QVector<KoGamutMaskShape*>* shapeVector;

    if (preview && !d->previewShapes.isEmpty()) {
        shapeVector = &d->previewShapes;
    } else {
        shapeVector = &d->maskShapes;
    }

    for(KoGamutMaskShape* shape: *shapeVector) {
        if (shape->coordIsClear(coord) == true) {
            return true;
        }
    }

    return false;
}

void KoGamutMask::paint(QPainter &painter, bool preview)
{
    QVector<KoGamutMaskShape*>* shapeVector;

    if (preview && !d->previewShapes.isEmpty()) {
        shapeVector = &d->previewShapes;
    } else {
        shapeVector = &d->maskShapes;
    }

    for(KoGamutMaskShape* shape: *shapeVector) {
        shape->paint(painter);
    }
}

void KoGamutMask::paintStroke(QPainter &painter, bool preview)
{
    QVector<KoGamutMaskShape*>* shapeVector;

    if (preview && !d->previewShapes.isEmpty()) {
        shapeVector = &d->previewShapes;
    } else {
        shapeVector = &d->maskShapes;
    }

    for(KoGamutMaskShape* shape: *shapeVector) {
        shape->paintStroke(painter);
    }
}

QTransform KoGamutMask::maskToViewTransform(qreal viewSize)
{
    // apply mask rotation before drawing
    QPointF centerPoint(viewSize*0.5, viewSize*0.5);

    QTransform transform;
    transform.translate(centerPoint.x(), centerPoint.y());
    transform.rotate(rotation());
    transform.translate(-centerPoint.x(), -centerPoint.y());

    qreal scale = viewSize/(maskSize().width());
    transform.scale(scale, scale);

    return transform;
}

QTransform KoGamutMask::viewToMaskTransform(qreal viewSize)
{
    QPointF centerPoint(viewSize*0.5, viewSize*0.5);

    QTransform transform;
    qreal scale = viewSize/(maskSize().width());
    transform.scale(1/scale, 1/scale);

    transform.translate(centerPoint.x(), centerPoint.y());
    transform.rotate(-rotation());
    transform.translate(-centerPoint.x(), -centerPoint.y());

    return transform;
}

bool KoGamutMask::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(resourcesInterface);

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
    setDescription(parser.documentDescription());

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
    writer.setDocumentDescription(description());

    writer.save(storeDev, d->maskSize);

    if (!store->close()) { return false; }


    if (!store->open("preview.png")) {
        return false;
    }

    KoStoreDevice previewDev(store);
    previewDev.open(QIODevice::WriteOnly);

    image().save(&previewDev, "PNG");
    if (!store->close()) { return false; }

    return store->finalize() && KoResource::saveToDevice(dev);
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
    QMap<QString, QVariant> m = metadata();
    return m["description"].toString();
}

void KoGamutMask::setDescription(QString description)
{
    addMetaData("description", description);
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
