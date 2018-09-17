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
#ifndef LIBKIS_FILELAYER_H
#define LIBKIS_FILELAYER_H

#include <QObject>
#include "Node.h"

#include <kis_types.h>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * @brief The FileLayer class
 * A file layer is a layer that can reference an external image
 * and show said reference in the layer stack.
 *
 * If the external image is updated, Krita will try to update the
 * file layer image as well.
 */

class KRITALIBKIS_EXPORT FileLayer : public Node
{
    Q_OBJECT
    Q_DISABLE_COPY(FileLayer)

public:
    explicit FileLayer(KisImageSP image,
                        const QString name = QString(),
                        const QString baseName=QString(),
                        const QString fileName=QString(),
                        const QString scalingMethod=QString(),
                        QObject *parent = 0);
    explicit FileLayer(KisFileLayerSP layer, QObject *parent = 0);
    ~FileLayer() override;
public Q_SLOTS:

    /**
     * @brief type Krita has several types of nodes, split in layers and masks. Group
     * layers can contain other layers, any layer can contain masks.
     *
     * @return "filelayer"
     */
    QString type() const override;

    /**
     * @brief setProperties
     * Change the properties of the file layer.
     * @param fileName - A String containing the absolute file name.
     * @param scalingMethod - a string with the scaling method, defaults to "None",
     *  other options are "ToImageSize" and "ToImagePPI"
     */
    void setProperties(QString fileName, QString scalingMethod = QString("None"));

    /**
     * @brief path
     * @return A QString with the full path of the referenced image.
     */
    QString path() const;

    /**
     * @brief scalingMethod
     * returns how the file referenced is scaled.
     * @return one of the following:
     *  <li> None - The file is not scaled in any way.
     *  <li> ToImageSize - The file is scaled to the full image size;
     *  <li> ToImagePPI - The file is scaled by the PPI of the image. This keep the physical dimensions the same.
     */
    QString scalingMethod() const;

private:
    /**
     * @brief getFileNameFromAbsolute
     * referenced from the fileLayer dialog, this will jumps through all the hoops
     * to ensure that an appropriate filename will be gotten.
     * @param baseName the location of the document.
     * @param absolutePath the absolute location of the file referenced.
     * @return the appropriate relative path.
     */
    QString getFileNameFromAbsolute(const QString &basePath, QString filePath);
    QString m_baseName;
};

#endif // LIBKIS_FILELAYER_H

