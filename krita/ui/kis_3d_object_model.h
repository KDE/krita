/* This file is part of the KDE project
 * Copyright (C) Lukáš Tvrdý, lukast.dev@gmail.com (c) 2009
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

#ifndef KIS_3D_OBJECT_MODEL_H
#define KIS_3D_OBJECT_MODEL_H

#include "kis_vec.h"

#include "krita_export.h"

#include <QString>
#include <QVector>
#include <QHash>

#include <config-opengl.h>

#ifdef HAVE_OPENGL
#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
#endif
#include <GL/gl.h>
#endif

struct Material {
    KisVector3D Ka; // ambient
    KisVector3D Kd; // diffuse
    KisVector3D Ks; // specularity
    qreal Ns; // hardness ??? [from Blender import script]
    qreal Ni; // index of refraction
    qreal d; // alpha
};

class KRITAUI_EXPORT Kis3DObjectModel
{
public:

    Kis3DObjectModel() {}
    Kis3DObjectModel(const QString &model, const QString &material);
    //void parseModel();
    void parseMaterial(const QString &fileName);

#ifdef HAVE_OPENGL
    GLuint displayList();
#endif // HAVE_OPENGL

private:
    QString m_fileName;
    QVector<KisVector3D> m_vertex;
    QVector<KisVector3D> m_normal;
    QVector<int> m_vertexIndex;
    QVector<int> m_normalIndex;
    QHash<QString, QVector<int> > m_vertexHash;
    QHash<QString, QVector<int> > m_normalHash;
    QHash<QString, Material> m_material;

    void debug(Material m);

    bool m_cached;
#ifdef HAVE_OPENGL
    GLuint m_displayList;
#endif // HAVE_OPENGL
};

#endif // KIS_3D_OBJECT_MODEL_H
