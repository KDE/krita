/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */

#include "kis_shape_model.h"
#include "kis_image.h"

class KisShapeModel::Private
{
public:
    KisImageSP image;
};

KisShapeModel::KisShapeModel( KisImageSP image, QObject * parent )
    : QObject( parent )
{
    m_d = new Private();
    m_d->image = image;
}

#include "kis_shape_model.moc"
