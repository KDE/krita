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

#include <QFile>
#include <QTextStream>

#include "kis_model.h"
#include "kis_vec.h"
#include "kis_debug.h"


KisModel::KisModel(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        kDebug() << "File Not Found";
        return;
    }
    
    QTextStream in(&file);
    while (!in.atEnd()) {

        QString line = in.readLine();
        if (line.isEmpty() || line[0] == '#'){
            continue;
        }
        
        QTextStream ts(&line);
        QString id;
        ts >> id;
        if (id == "v") {
            KisVector3D vector;
            for (int i = 0; i < 3; i++){
                ts >> vector[i];
            }
            m_vertex.append(vector);
        }else if (id == "vn"){
            KisVector3D normal;
            for (int i = 0; i < 3; i++){
                ts >> normal[i];
            }
            m_normal.append(vector);
        }


    }

    kDebug() << "Vertices: " << m_vertex.size();
    kDebug() << "Vertices [0]: " << m_vertex[0].x() << " " << m_vertex[0].y() << " " << m_vertex[0].z();
    kDebug() << "Vertices [last]: " << m_vertex[m_vertex.size() - 1].x() << " " << m_vertex[ m_vertex.size() - 1 ].y() << " " << m_vertex[ m_vertex.size() - 1 ].z();
}

