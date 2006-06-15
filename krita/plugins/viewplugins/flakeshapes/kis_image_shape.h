/*
 * kis_image_flake.h -- Part of Krita
 *
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
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

#ifndef _KIS_IMAGE_SHAKE_H_
#define _KIS_IMAGE_SHAKE_H_

#include <QWidget>
//   #include <QPainter>
//
//   #include <KoViewConverter.h>
#include <KoShape.h>
#include <KoShapeFactory.h>
//
//   #include <kis_types.h>
#include "kis_image.h"

class QPainter;

/**
 * The paint device flake is a wrapper around a Krita image
 */
class KisImageShape : public KoShape {

public:

    KisImageShape();
    virtual void paint(QPainter &painter, KoViewConverter &converter);
    
private:

    KisImageSP m_image;
    
};


/**
 * Factory class for indirect creation of KisImageShapes
 */
class KisImageShapeFactory : public KoShapeFactory {

public:

    KisImageShapeFactory(QObject *parent,  const QStringList &);
    virtual ~KisImageShapeFactory() {};

public:

    virtual KoShape * createDefaultShape();
    virtual KoShape * createShape(const KoProperties * params) const;
    virtual QWidget * optionWidget();

};


#endif // _KIS_IMAGE_SHAKE_H_

