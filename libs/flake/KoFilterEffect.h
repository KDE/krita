/* This file is part of the KDE project
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "flake_export.h"

/**
 * This is the base for filter effect (blur, invert...) that can be applied on a shape.
 *
 */
class FLAKE_EXPORT KoFilterEffect
{
public:
    KoFilterEffect( const QString& id, const QString& name );
    virtual ~KoFilterEffect();
    
    /**
     * Apply the effect on an image.
     */
    virtual void processImage(QImage &image) const = 0;
    
    /**
     * @param needed the rectangle on which the filter will be processed
     * @return the rectangle that is needed to process the @p needed rectangle 
     */
    virtual QRect inputRect(const QRect& needed) const = 0;

    /// Returns the user visible name of the filter
    QString name() const;
    
    /// Returns the unique id of the filter
    QString id() const;
    
private:
    struct Private;
    Private* const d;
};

#endif // _KO_FILTER_EFFECT_H_
