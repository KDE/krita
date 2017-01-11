/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef LIBKIS_DOCUMENT_H
#define LIBKIS_DOCUMENT_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

class KisDocument;

/**
 * Document
 */
class KRITALIBKIS_EXPORT Document : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Document)

public:
    explicit Document(KisDocument *document, bool ownsDocument = false, QObject *parent = 0);
    virtual ~Document();

public Q_SLOTS:

    bool batchmode() const;
    void setBatchmode(bool value);

    Node* activeNode() const;
    void setActiveNode(Node* value);

    /**
     * colorDepth A string describing the color depth of the image:
     * <ul>
     * <li>U8: unsigned 8 bits integer, the most common type</li>
     * <li>U16: unsigned 16 bits integer</li>
     * <li>F16: half, 16 bits floating point. Only available if Krita was built with OpenEXR</li>
     * <li>F32: 32 bits floating point</li>
     * </ul>
     * @return the color depth.
     */
    QString colorDepth() const;

    /**
     * @brief colorModel retrieve the current color model of this document:
     * <ul>
     * <li>A: Alpha mask</li>
     * <li>RGBA: RGB with alpha channel (The actual order of channels is most often BGR!)</li>
     * <li>XYZA: XYZ with alpha channel</li>
     * <li>LABA: LAB with alpha channel</li>
     * <li>CMYKA: CMYK with alpha channel</li>
     * <li>GRAYA: Gray with alpha channel</li>
     * <li>YCbCrA: YCbCr with alpha channel</li>
     * </ul>
     * @return the internal color model string.
     */
    QString colorModel() const;

    /**
     * @return the name of the current color profile
     */
    QString colorProfile() const;

    /**
     * @brief setColorProfile set the color profile of the image to the given profile. The profile has to
     * be registered with krita and be compatible with the current color model and depth; the image data
     * is <i>not</i> converted.
     * @param colorProfile
     * @return false if the colorProfile name does not correspond to to a registered profile or if assigning
     * the profile failed.
     */
    bool setColorProfile(const QString &colorProfile);

    /**
     * @brief setColorSpace convert the nodes and the image to the given colorspace. The conversion is
     * done with Perceptual as intent, High Quality and No LCMS Optimizations as flags and no blackpoint
     * compensation.
     *
     * @param colorModel A string describing the color model of the image:
     * <ul>
     * <li>A: Alpha mask</li>
     * <li>RGBA: RGB with alpha channel (The actual order of channels is most often BGR!)</li>
     * <li>XYZA: XYZ with alpha channel</li>
     * <li>LABA: LAB with alpha channel</li>
     * <li>CMYKA: CMYK with alpha channel</li>
     * <li>GRAYA: Gray with alpha channel</li>
     * <li>YCbCrA: YCbCr with alpha channel</li>
     * </ul>
     * @param colorDepth A string describing the color depth of the image:
     * <ul>
     * <li>U8: unsigned 8 bits integer, the most common type</li>
     * <li>U16: unsigned 16 bits integer</li>
     * <li>F16: half, 16 bits floating point. Only available if Krita was built with OpenEXR</li>
     * <li>F32: 32 bits floating point</li>
     * </ul>
     * @param colorProfile a valid color profile for this color model and color depth combination.
     * @return false the combination of these arguments does not correspond to a colorspace.
     */
    bool setColorSpace(const QString &colorModel, const QString &colorDepth, const QString &colorProfile);


    /**
     * @brief documentInfo creates and XML document representing document and author information.
     * @return a string containing a valid XML document
     */
    QString documentInfo() const;
    void setDocumentInfo(const QString &document);

    QString fileName() const;
    void setFileName(QString value);

    int height() const;
    void setHeight(int value);

    InfoObject* metaData() const;
    void setMetaData(InfoObject* value);

    QString name() const;
    void setName(QString value);

    /**
     * @return the resolution in pixels per inch
     */
    int resolution() const;
    /**
     * @brief setResolution set the resolution of the image; this does not scale the image
     * @param value the resolution in pixels per inch
     */
    void setResolution(int value);

    Node* rootNode() const;

    Selection* selection() const;
    void setSelection(Selection* value);

    int width() const;
    void setWidth(int value);

    /**
     * @return xRes the horizontal resolution of the image in pixels per pt (there are 72 pts to an inch)
     */
    double xRes() const;
    void setXRes(double xRes) const;

    /**
     * @return yRes the vertical resolution of the image in pixels per pt (there are 72 pts to an inch)
     */
    double yRes() const;
    void setyRes(double yRes) const;

    QByteArray pixelData() const;


    bool close();

    void crop(int x, int y, int w, int h);

    bool exportImage(const QString &filename, const InfoObject &exportConfiguration);

    void flatten();

    void resizeImage(int w, int h);

    bool save();

    bool saveAs(const QString &filename);

    void openView();

    Node* createNode(const QString &name, const QString &nodeType);

Q_SIGNALS:

    void nodeCreated(Node *node);

private:

    friend class Krita;
    friend class Window;
    QPointer<KisDocument> document() const;


private:
    struct Private;
    Private *const d;

};

#endif // LIBKIS_DOCUMENT_H
