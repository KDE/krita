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

#include <kmimetype.h>
#include <ktemporaryfile.h>
#include <KIO/NetAccess>
#include <KIO/CopyJob>

#include <QBuffer>
#include <QHash>
#include <QFile>
#include <QFileInfo>

class SvgSavingContext::Private
{
public:
    Private(QIODevice &outputDevice)
        : output(outputDevice), styleWriter(0), shapeWriter(0)
        , saveInlineImages(true)
    {
        styleWriter = new KoXmlWriter(&styleBuffer, 1);
        styleWriter->startElement("defs");
        shapeWriter = new KoXmlWriter(&shapeBuffer, 1);

        const qreal scaleToUserSpace = SvgUtil::toUserSpace(1.0);
        userSpaceMatrix.scale(scaleToUserSpace, scaleToUserSpace);
    }

    ~Private()
    {
        delete styleWriter;
        delete shapeWriter;
    }

    QIODevice &output;
    QBuffer styleBuffer;
    QBuffer shapeBuffer;
    KoXmlWriter *styleWriter;
    KoXmlWriter *shapeWriter;

    QHash<QString, int> uniqueNames;
    QHash<const KoShape*, QString> shapeIds;
    QTransform userSpaceMatrix;
    bool saveInlineImages;
};

SvgSavingContext::SvgSavingContext(QIODevice &outputDevice, bool saveInlineImages)
    : d(new Private(outputDevice))
{
    d->saveInlineImages = saveInlineImages;
}

SvgSavingContext::~SvgSavingContext()
{
    d->styleWriter->endElement();
    d->output.write(d->styleBuffer.data());
    d->output.write("\n");
    d->output.write(d->shapeBuffer.data());

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
            // create a compeletely new id based on object name
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
    QFile *file = qobject_cast<QFile*>(&d->output);
    if (!file)
        return QString();

    // get url of destination directory
    KUrl url(file->fileName());
    QString dstBaseFilename = QFileInfo(url.fileName()).baseName();
    url.setDirectory(url.directory());
    // create a filename for the image file at the destination directory
    QString fname = dstBaseFilename + '_' + createUID("file");
    url.setFileName(fname + extension);
    // check if file exists already
    int i = 0;
    // change filename as long as the filename already exists
    while (KIO::NetAccess::exists(url, KIO::NetAccess::DestinationSide, 0))
        url.setFileName(fname + QString("_%1").arg(++i) + extension);

    return url.fileName();
}

QString SvgSavingContext::saveImage(const QImage &image)
{
    if (isSavingInlineImages()) {
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        if (image.save(&buffer, "PNG")) {
            const QString mimeType(KMimeType::findByContent(ba)->name());
            const QString header("data:" + mimeType + ";base64,");
            return header + ba.toBase64();
        }
    } else {
        // write to a temp file first
        KTemporaryFile imgFile;
        if (image.save(&imgFile, "PNG")) {
            // tz: TODO the new version of KoImageData has the extension save inside maybe that can be used
            // get the mime type from the temp file content
            KMimeType::Ptr mimeType = KMimeType::findByFileContent(imgFile.fileName());
            // get extension from mimetype
            QString ext = "";
            QStringList patterns = mimeType->patterns();
            if (patterns.count())
                ext = patterns.first().mid(1);

            QString dstFilename = createFileName(ext);

            // move the temp file to the destination directory
            KIO::Job * job = KIO::move(KUrl(imgFile.fileName()), KUrl(dstFilename));
            if (job && KIO::NetAccess::synchronousRun(job, 0))
                return dstFilename;
            else
                KIO::NetAccess::removeTempFile(imgFile.fileName());
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
            const QString mimeType(KMimeType::findByContent(ba)->name());
            const QString header("data:" + mimeType + ";base64,");
            return header + ba.toBase64();
        }
    } else {
        // write to a temp file first
        KTemporaryFile imgFile;
        if (image->saveData(imgFile)) {
            // tz: TODO the new version of KoImageData has the extension save inside maybe that can be used
            // get the mime type from the temp file content
            KMimeType::Ptr mimeType = KMimeType::findByFileContent(imgFile.fileName());
            // get extension from mimetype
            QString ext = "";
            QStringList patterns = mimeType->patterns();
            if (patterns.count())
                ext = patterns.first().mid(1);

            QString dstFilename = createFileName(ext);

            // move the temp file to the destination directory
            KIO::Job * job = KIO::move(KUrl(imgFile.fileName()), KUrl(dstFilename));
            if (job && KIO::NetAccess::synchronousRun(job, 0))
                return dstFilename;
            else
                KIO::NetAccess::removeTempFile(imgFile.fileName());
        }
    }
    return QString();
}
