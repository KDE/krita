/* This file is part of the KDE project
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "SvgSavingContext.h"
#include "SvgUtil.h"

#include <KoXmlWriter.h>
#include <KoShape.h>
#include <KoShapeGroup.h>
#include <KoShapeLayer.h>
#include <KoImageData.h>

#include <QTemporaryFile>

#include <QImage>
#include <QTransform>
#include <QBuffer>
#include <QHash>
#include <QFile>
#include <QFileInfo>
#include <KisMimeDatabase.h>

class Q_DECL_HIDDEN SvgSavingContext::Private
{
public:
    Private(QIODevice *_mainDevice, QIODevice *_styleDevice)
        : mainDevice(_mainDevice)
        , styleDevice(_styleDevice)
        , styleWriter(0)
        , shapeWriter(0)
        , saveInlineImages(true)
    {
        styleWriter.reset(new KoXmlWriter(&styleBuffer, 1));
        styleWriter->startElement("defs");
        shapeWriter.reset(new KoXmlWriter(&shapeBuffer, 1));

        const qreal scaleToUserSpace = SvgUtil::toUserSpace(1.0);
        userSpaceMatrix.scale(scaleToUserSpace, scaleToUserSpace);
    }

    ~Private()
    {
    }

    QIODevice *mainDevice;
    QIODevice *styleDevice;
    QBuffer styleBuffer;
    QBuffer shapeBuffer;
    QScopedPointer<KoXmlWriter> styleWriter;
    QScopedPointer<KoXmlWriter> shapeWriter;

    QHash<QString, int> uniqueNames;
    QHash<const KoShape*, QString> shapeIds;
    QTransform userSpaceMatrix;
    bool saveInlineImages;
    bool strippedTextMode = false;
};

SvgSavingContext::SvgSavingContext(QIODevice &outputDevice, bool saveInlineImages)
    : d(new Private(&outputDevice, 0))
{
    d->saveInlineImages = saveInlineImages;
}

SvgSavingContext::SvgSavingContext(QIODevice &shapesDevice, QIODevice &styleDevice, bool saveInlineImages)
    : d(new Private(&shapesDevice, &styleDevice))
{
    d->saveInlineImages = saveInlineImages;
}

SvgSavingContext::~SvgSavingContext()
{
    d->styleWriter->endElement();

    if (d->styleDevice) {
        d->styleDevice->write(d->styleBuffer.data());
    } else {
        d->mainDevice->write(d->styleBuffer.data());
        d->mainDevice->write("\n");
    }

    d->mainDevice->write(d->shapeBuffer.data());

    delete d;
}

KoXmlWriter &SvgSavingContext::styleWriter()
{
    return *d->styleWriter;
}

KoXmlWriter &SvgSavingContext::shapeWriter()
{
    return *d->shapeWriter;
}

QString SvgSavingContext::createUID(const QString &base)
{
    QString idBase = base.isEmpty() ? "defitem" : base;
    int counter = d->uniqueNames.value(idBase);
    d->uniqueNames.insert(idBase, counter+1);

    return idBase + QString("%1").arg(counter);
}

QString SvgSavingContext::getID(const KoShape *obj)
{
    QString id;
    // do we have already an id for this object ?
    if (d->shapeIds.contains(obj)) {
        // use existing id
        id = d->shapeIds[obj];
    } else {
        // initialize from object name
        id = obj->name();
        // if object name is not empty and was not used already
        // we can use it as is
        if (!id.isEmpty() && !d->uniqueNames.contains(id)) {
            // add to unique names so it does not get reused
            d->uniqueNames.insert(id, 1);
        } else {
            if (id.isEmpty()) {
                // differentiate a little between shape types
                if (dynamic_cast<const KoShapeGroup*>(obj))
                    id = "group";
                else if (dynamic_cast<const KoShapeLayer*>(obj))
                    id = "layer";
                else
                    id = "shape";
            }
            // create a completely new id based on object name
            // or a generic name
            id = createUID(id);
        }
        // record id for this shape
        d->shapeIds.insert(obj, id);
    }
    return id;
}

QTransform SvgSavingContext::userSpaceTransform() const
{
    return d->userSpaceMatrix;
}

bool SvgSavingContext::isSavingInlineImages() const
{
    return d->saveInlineImages;
}

QString SvgSavingContext::createFileName(const QString &extension)
{
    QFile *file = qobject_cast<QFile*>(d->mainDevice);
    if (!file)
        return QString();

    QFileInfo fi(file->fileName());
    QString path = fi.absolutePath();
    QString dstBaseFilename = fi.baseName();

    // create a filename for the image file at the destination directory
    QString fname = dstBaseFilename + '_' + createUID("file");

    // check if file exists already
    int i = 0;
    QString counter;
    // change filename as long as the filename already exists
    while (QFile(path + fname + counter + extension).exists()) {
        counter = QString("_%1").arg(++i);
    }

    return fname + counter + extension;
}

QString SvgSavingContext::saveImage(const QImage &image)
{
    if (isSavingInlineImages()) {
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        if (image.save(&buffer, "PNG")) {
            const QString header("data:image/x-png;base64,");
            return header + ba.toBase64();
        }
    } else {
        // write to a temp file first
        QTemporaryFile imgFile;
        if (image.save(&imgFile, "PNG")) {
            QString dstFilename = createFileName(".png");
            if (QFile::copy(imgFile.fileName(), dstFilename)) {
                return dstFilename;
            }
            else {
                QFile f(imgFile.fileName());
                f.remove();
            }
        }
    }

    return QString();
}

QString SvgSavingContext::saveImage(KoImageData *image)
{
    if (isSavingInlineImages()) {
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        if (image->saveData(buffer)) {
            const QString header("data:" + KisMimeDatabase::mimeTypeForSuffix(image->suffix()) + ";base64,");
            return header + ba.toBase64();
        }
    } else {
        // write to a temp file first
        QTemporaryFile imgFile;
        if (image->saveData(imgFile)) {
            QString ext = image->suffix();
            QString dstFilename = createFileName(ext);
            // move the temp file to the destination directory
            if (QFile::copy(imgFile.fileName(), dstFilename)) {
                return dstFilename;
            }
            else {
                QFile f(imgFile.fileName());
                f.remove();
            }
        }
    }
    return QString();
}

void SvgSavingContext::setStrippedTextMode(bool value)
{
    d->strippedTextMode = value;
}

bool SvgSavingContext::strippedTextMode() const
{
    return d->strippedTextMode;
}
