/* This file is part of the KDE project
 *
 * Copyright (C) 2012 Inge Wallin <inge@lysator.liu.se>
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

// Own
#include "Object3D.h"

// Qt

// KDE
#include <KDebug>

// Calligra
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoGenStyle.h>
#include <KoStyleStack.h>
#include <KoShapeSavingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>

// Shape
#include "utils.h"


//#define OdfObjectAttributes (OdfAllAttributes & ~(OdfGeometry | OdfTransformation))
#define OdfObjectAttributes (OdfAdditionalAttributes | OdfMandatories)

// ================================================================
//                             Object3D


Object3D::Object3D(Object3D *parent)
    : m_parent(parent)
{
}

Object3D::~Object3D()
{
}


QString Object3D::transform()
{
    return m_transform3D;
}

Object3D *Object3D::parent()
{
    return m_parent;
}


bool Object3D::loadOdf(const KoXmlElement &objectElement, KoShapeLoadingContext &context)
{
    Q_UNUSED(context);

    m_transform3D = objectElement.attributeNS(KoXmlNS::dr3d, "transform", "");

    return true;
}

void Object3D::saveOdf(KoShapeSavingContext &context) const
{
    Q_UNUSED(context);

}
void Object3D::saveObjectOdf(KoShapeSavingContext &context) const
{
    if (!m_transform3D.isEmpty()) {
        KoXmlWriter &writer = context.xmlWriter();
        writer.addAttribute("dr3d:transform", m_transform3D);
    }
}
