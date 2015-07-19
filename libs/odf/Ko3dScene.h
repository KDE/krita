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

#ifndef KO3DSCENE_H
#define KO3DSCENE_H

// Qt
#include <QColor>
#include <QVector3D>

class KoXmlWriter;
class KoXmlElement;


/** A scene in which to show 3d objects.
 *
 * The scene parameters include camera parameters (origin, direction
 * and up direction), the projection to be used and a shadow
 * slant. All these are attributes of the element.
 *
 * The scene can also have a number of light sources as child
 * elements.  These are picked up from the XML element but others are
 * ignored and have to be loaded by code that handles the actual
 * element.
 *
 * In ODF 1.2, a scene description can be part of a dr3d:scene or
 * chart:plot-area if the chart also has 3D mode set.
 */


#include "koodf_export.h"


class KOODF_EXPORT Ko3dScene
{
public:
    enum Projection {
        Parallel,
        Perspective
    };

    enum Shademode {
        Flat,
        Gouraud,
        Phong,
        Draft                   // Wireframe
    };

    class Lightsource
    {
    public:
        Lightsource();
        ~Lightsource();

        bool loadOdf(const KoXmlElement &lightElement);
        void saveOdf(KoXmlWriter &writer) const;

        // getters
        QColor diffuseColor() const;
        QVector3D direction() const;
        bool enabled() const;
        bool specular() const;

        // setters
        void setDiffuseColor(const QColor &color);
        void setDirection(const QVector3D &direction);
        void setEnabled(const bool enabled);
        void setSpecular(const bool specular);

    private:
        QColor m_diffuseColor;
        QVector3D  m_direction;
        bool m_enabled;
        bool m_specular;
    };

    Ko3dScene();
    ~Ko3dScene();

    bool loadOdf(const KoXmlElement &sceneElement);
    void saveOdfAttributes(KoXmlWriter &writer) const;
    void saveOdfChildren(KoXmlWriter &writer) const;

    // getters
    QVector3D vrp() const;
    QVector3D vpn() const;
    QVector3D vup() const;
    Projection projection() const;
    QString distance() const;
    QString focalLength() const;
    QString shadowSlant() const;
    Shademode shadeMode() const;
    QColor ambientColor() const;
    bool lightingMode() const;
    QString transform() const;

    // setters
    void setVrp(const QVector3D &vrp);
    void setVpn(const QVector3D &vpn);
    void setVup(const QVector3D &vup);
    void setProjection(Projection projection);
    void setDistance(const QString &distance);
    void setFocalLength(const QString &focalLength);
    void setShadowSlant(const QString &shadowSlant);
    void setShadeMode(Shademode shadeMode);
    void setAmbientColor(const QColor &ambientColor);
    void setLightingMode(bool lightingMode);
    void setTransform(const QString &transform);

private:
    class Private;
    Private * const d;
};

Q_DECLARE_TYPEINFO(Ko3dScene::Lightsource, Q_MOVABLE_TYPE);


/** Try to load a 3d scene from an element and return a pointer to a
 * Ko3dScene if it succeeded.
 */
KOODF_EXPORT Ko3dScene *load3dScene(const KoXmlElement &element);


#endif
