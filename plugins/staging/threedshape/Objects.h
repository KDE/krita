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

#ifndef OBJECTS_H
#define OBJECTS_H

// Qt
#include <QVector3D>

// Calligra


// Shape
#include "Object3D.h"

class KoXmlElement;
class KoXmlWriter;


class Sphere : public Object3D, public KoShape
{
public:
    explicit Sphere(Object3D *parent);
    virtual ~Sphere();

    virtual void paint(QPainter &painter, const KoViewConverter &converter,
                       KoShapePaintingContext &context);
    virtual bool loadOdf(const KoXmlElement &objectElement, KoShapeLoadingContext &context);
    virtual void saveOdf(KoShapeSavingContext &context) const;

    // Really save the object.  See the explanation in Object3D.h.
    virtual void saveObjectOdf(KoShapeSavingContext &context) const;

    // getters
    QVector3D sphereCenter() const { return m_center; }
    QVector3D sphereSize()   const { return m_size;   }

private:
    QVector3D  m_center;
    QVector3D  m_size;
};

class Cube : public Object3D, public KoShape
{
public:
    explicit Cube(Object3D *parent);
    virtual ~Cube();

    virtual void paint(QPainter &painter, const KoViewConverter &converter,
                       KoShapePaintingContext &context);
    virtual bool loadOdf(const KoXmlElement &objectElement, KoShapeLoadingContext &context);
    virtual void saveOdf(KoShapeSavingContext &context) const;

    // Really save the object.  See the explanation in Object3D.h.
    virtual void saveObjectOdf(KoShapeSavingContext &context) const;

    // getters
    QVector3D minEdge() const { return m_minEdge;   }
    QVector3D maxEdge() const { return m_maxEdge; }

private:
    QVector3D  m_minEdge;
    QVector3D  m_maxEdge;
};

class Extrude : public Object3D, public KoShape
{
public:
    explicit Extrude(Object3D *parent);
    virtual ~Extrude();

    virtual void paint(QPainter &painter, const KoViewConverter &converter,
                       KoShapePaintingContext &context);
    virtual bool loadOdf(const KoXmlElement &objectElement, KoShapeLoadingContext &context);
    virtual void saveOdf(KoShapeSavingContext &context) const;

    virtual void loadStyle(const KoXmlElement &element, KoShapeLoadingContext &context);
    virtual QString saveStyle(KoGenStyle& style, KoShapeSavingContext& context) const;

    // Really save the object.  See the explanation in Object3D.h.
    virtual void saveObjectOdf(KoShapeSavingContext &context) const;

    // getters
    QString path()       const { return m_path; }
    QString viewBox()    const { return m_viewBox; }
    int     depth()      const { return m_depth; }
    bool    closeFront() const { return m_closeFront; }
    bool    closeBack()  const { return m_closeBack; }
    qreal   backScale()  const { return m_backScale; }

private:
    QString  m_path;            // The polygon
    QString  m_viewBox;         // Defines the coordinate system for svg:d
    qreal    m_depth;
    bool     m_closeFront;
    bool     m_closeBack;
    qreal    m_backScale;
};

class Rotate : public Object3D, public KoShape
{
public:
    explicit Rotate(Object3D *parent);
    virtual ~Rotate();

    // reimplemented from KoShape
    virtual void paint(QPainter &painter, const KoViewConverter &converter,
                       KoShapePaintingContext &context);
    virtual bool loadOdf(const KoXmlElement &objectElement, KoShapeLoadingContext &context);
    virtual void saveOdf(KoShapeSavingContext &context) const;

    virtual void loadStyle(const KoXmlElement &element, KoShapeLoadingContext &context);
    virtual QString saveStyle(KoGenStyle& style, KoShapeSavingContext& context) const;

    // Really save the object.  See the explanation in Object3D.h.
    virtual void saveObjectOdf(KoShapeSavingContext &context) const;

    // getters
    QString path()               const { return m_path; }
    QString viewBox()            const { return m_viewBox; }
    int     horizontalSegments() const { return m_horizontalSegments; }
    int     verticalSegments()   const { return m_verticalSegments; }
    qreal   endAngle()           const { return m_endAngle; }
    bool    closeFront()         const { return m_closeFront; }
    bool    closeBack()          const { return m_closeBack; }
    qreal   backScale()          const { return m_backScale; }

private:
    QString  m_path;               // The polygon
    QString  m_viewBox;            // Defines the coordinate system for svg:d
    int      m_horizontalSegments; // Defined in ODF
    int      m_verticalSegments;   // Produced by OOo(!)
    qreal    m_endAngle;
    bool     m_closeFront;
    bool     m_closeBack;
    qreal    m_backScale;
};

#endif
