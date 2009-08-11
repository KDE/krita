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

#ifndef KIS_MODEL_H
#define KIS_MODEL_H

#include "kis_vec.h"

#include "krita_export.h"

#include <QString>
#include <QVector>

#include <config-opengl.h>

#ifdef HAVE_OPENGL
#include <GL/gl.h>
#endif


class PAINTOP_EXPORT KisModel
{
public:
    
    KisModel(){}
    KisModel(const QString &fileName);
    GLuint displayList();

private:
    QString m_fileName;
    QVector<KisVector3D> m_vertex;
    QVector<KisVector3D> m_normal;
    QVector<int> m_vertexIndex;
    QVector<int> m_normalIndex;
    
    bool m_cached;
    GLuint m_displayList;
};

#endif // KIS_MODEL_H
