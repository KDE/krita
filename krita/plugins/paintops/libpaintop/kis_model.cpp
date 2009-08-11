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
#include <QString>
#include <QStringList>

#include "kis_model.h"
#include "kis_vec.h"
#include "kis_debug.h"


#include <config-opengl.h>

#ifdef HAVE_OPENGL
#include <GL/gl.h>
#endif


KisModel::KisModel(const QString& fileName)
{
    m_cached = false;

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
            m_normal.append(normal);
        }else if (id == "f"){
            QString edge;
            QStringList edgeList;
            // triangles are supported so far and format f vertexIndex//vertexNormalIndex
            for (int i = 0; i < 3; i++){
                ts >> edge;
                edgeList = edge.split('/');
                m_vertexIndex.append( edgeList.value(0).toInt() - 1);
                m_normalIndex.append( edgeList.value(2).toInt() - 1);
            }
        }
    }
}

#define MODEL_SCALE 30
GLuint KisModel::displayList()
{
    if (m_cached){ 
        return m_displayList; 
    }
    
    // nothing to render
    if (m_vertex.size() == 0){
        return 0;
    }
    
    KisVector3D vertex;
    KisVector3D normal;
    m_displayList = glGenLists(1);
   
    glNewList(m_displayList, GL_COMPILE);
    glScalef( MODEL_SCALE,MODEL_SCALE,MODEL_SCALE);
    glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_vertexIndex.size() - 3; i += 3){
            for (int j = 0; j < 3; j++){
                int vi = m_vertexIndex[i+j];
                int ni = m_normalIndex[i+j];
                vertex = m_vertex[ vi ];
                normal = m_normal[ ni ];
                glNormal3f(normal.x(), normal.y(), normal.z() );
                glVertex3f(vertex.x(), vertex.y(), vertex.z() );
            }
        }
    glEnd();
    glScalef( 1.0/MODEL_SCALE,1.0/MODEL_SCALE,1.0/MODEL_SCALE);
    glEndList();    
/*        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, (float *)m_vertex.data());
        glNormalPointer(GL_FLOAT, 0, (float *)m_normal.data());
        glDrawElements(GL_TRIANGLES, m_vertexIndex.size(), GL_UNSIGNED_INT, m_vertexIndex.data());

        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
*/
    

    // list is ready
    m_cached = true;
    return m_displayList;
    
}
