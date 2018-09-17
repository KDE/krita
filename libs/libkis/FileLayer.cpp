/*
 *  Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "FileLayer.h"
#include <kis_file_layer.h>
#include <kis_image.h>
#include <QFileInfo>
#include <QDir>

FileLayer::FileLayer(KisImageSP image, const QString name, const QString baseName, const QString fileName, const QString scalingMethod, QObject *parent)
    : Node(image, new KisFileLayer(image, name, OPACITY_OPAQUE_U8), parent)
{
    KisFileLayer *file = dynamic_cast<KisFileLayer*>(this->node().data());
    KisFileLayer::ScalingMethod sm = KisFileLayer::None;
    if (scalingMethod=="ToImageSize") {
        sm= KisFileLayer::ToImageSize;
    } else if (scalingMethod=="ToImagePPI") {
        sm= KisFileLayer::ToImagePPI;
    }
    file->setScalingMethod(sm);

    const QString &basePath = QFileInfo(baseName).absolutePath();
    const QString &absoluteFilePath = QFileInfo(fileName).absoluteFilePath();
    file->setFileName(basePath, getFileNameFromAbsolute(basePath, absoluteFilePath));
}

FileLayer::FileLayer(KisFileLayerSP layer, QObject *parent)
    : Node(layer->image(), layer, parent)
{

}

FileLayer::~FileLayer()
{

}

QString FileLayer::type() const
{
    return "filelayer";
}

void FileLayer::setProperties(QString fileName, QString scalingMethod)
{
    KisFileLayer *file = dynamic_cast<KisFileLayer*>(this->node().data());
    KisFileLayer::ScalingMethod sm = KisFileLayer::None;
    if (scalingMethod.toLower() == "toimagesize") {
        sm= KisFileLayer::ToImageSize;
    } else if (scalingMethod.toLower() == "toimageppi") {
        sm= KisFileLayer::ToImagePPI;
    }
    file->setScalingMethod(sm);

    const QString basePath = QFileInfo(file->path()).absolutePath();
    const QString absoluteFilePath = QFileInfo(fileName).absoluteFilePath();
    file->setFileName(basePath, getFileNameFromAbsolute(basePath, absoluteFilePath));
}

QString FileLayer::path() const
{
    const KisFileLayer *file = qobject_cast<const KisFileLayer*>(this->node());
    return file->path();
}

QString FileLayer::scalingMethod() const
{
    const KisFileLayer *file = qobject_cast<const KisFileLayer*>(this->node());
    KisFileLayer::ScalingMethod sm = file->scalingMethod();
    QString method = "None";

    if (sm==KisFileLayer::ToImageSize) {
        method = "ToImageSize";
    } else if (sm==KisFileLayer::ToImagePPI) {
        method = "ToImagePPI";
    }
    return method;
}

QString FileLayer::getFileNameFromAbsolute(const QString &basePath, QString filePath)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(QFileInfo(filePath).isAbsolute(), filePath);

    // try to resolve symlink
    {
        const QFileInfo fi(filePath);
        if (fi.isSymLink()) {
            filePath = fi.symLinkTarget();
        }
    }

    if (!basePath.isEmpty()) {
        QDir directory(basePath);
        filePath = directory.relativeFilePath(filePath);
    }

    return filePath;
}


