/* This file is part of the KDE project
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KO_FILTER_EFFECT_H_
#define _KO_FILTER_EFFECT_H_

class QImage;
class QString;
class QRect;
class QRectF;
class QDomElement;
class KoViewConverter;
class KoXmlWriter;

#include "flake_export.h"
#include <QtCore/QList>
#include <QtGui/QImage>

/**
 * This is the base for filter effect (blur, invert...) that can be applied on a shape.
 *
 */
class FLAKE_EXPORT KoFilterEffect
{
public:
    KoFilterEffect( const QString& id, const QString& name );
    virtual ~KoFilterEffect();
    
    /// Returns the user visible name of the filter
    QString name() const;
    
    /// Returns the unique id of the filter
    QString id() const;
    
    /// Sets the clipping rectangle used for this filter
    void setClipRect(const QRectF &clipRect);
    
    /// Returns the clipping rectangle used for this filter
    QRectF clipRect() const;
    
    /// Sets the region the filter is applied to
    void setFilterRect(const QRectF &filterRect);
    
    /// Returns the region this filter is applied to
    QRectF filterRect() const;
    
    /**
    * Sets the name of the output image
    *
    * The name is used so that other effects can reference
    * the output of this effect as one of their input images.
    *
    * @param output the output image name
    */
    void setOutput(const QString &output);
    
    /// Returns the name of the output image
    QString output() const;
    
    /**
     * Returns list of named input images of this filter effect.
     *
     * These names identify the input images for this filter effect.
     * These can be one of the keywords SourceGraphic, SourceAlpha,
     * BackgroundImage, BackgroundAlpha, FillPaint or StrokePaint, 
     * as well as a named output of another filter effect in the stack.
     * An empty input list of the first effect in the stack default to
     * SourceGraphic, whereas on subsequent effects it defaults to the
     * result of the previous filter effect.
     */
    QList<QString> inputs() const;
    
    /// Adds a new input at the end of the input list
    void addInput(const QString &input);
    
    /// Inserts an input at the giben position in the input list
    void insertInput(int index, const QString &input);
    
    /// Removes an input from the given position in the input list
    void removeInput(int index);
    
    /**
     * Returns if this effect only works on a single input image.
     *
     * The default implementation returns true, as most of the 
     * effects only have a single input image to process.
     */
    virtual bool hasSingleInput() const;
    
    /**
     * Apply the effect on an image.
     * @param image the image the filter should be applied to
     * @param filterRegion the region of the image corresponding to the filter region
     * @param converter to convert between document and view coordinates 
     */
    virtual QImage processImage(const QImage &image, const QRect &filterRegion, const KoViewConverter &converter) const = 0;

    /**
    * Apply the effect on a list of images.
    * @param images the images the filter should be applied to
    * @param filterRegion the region of the image corresponding to the filter region
    * @param converter to convert between document and view coordinates 
    */
    virtual QImage processImages(const QList<QImage> &images, const QRect &filterRegion, const KoViewConverter &converter) const;
    
    /**
     * Loads data from given xml element.
     * @param element the xml element to load data from
     * @return true if loading was successful, else false
     */
    virtual bool load(const QDomElement &element) = 0;
    
    /**
     * Writes custom data to given xml element.
     * @param writer the xml writer to write data to
     */
    virtual void save(KoXmlWriter &writer) = 0;
    
private:
    struct Private;
    Private* const d;
};

#endif // _KO_FILTER_EFFECT_H_
