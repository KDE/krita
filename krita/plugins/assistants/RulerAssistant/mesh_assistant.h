/*
 * Copyright (c) 2012 Shivaraman Aiyer <sra392@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef _MESH_ASSISTANT_H_
#define _MESH_ASSISTANT_H_

#include "kis_painting_assistant.h"
#include <QObject>
#include <QPolygonF>
#include <QLineF>
#include <QTransform>
#include <map>
#include <vector>
#include <klocale.h>
#include <assert.h>
#include <kis_tool.h>
#include <kis_config.h>
#include <kis_canvas2.h>

#include <opengl/kis_opengl.h>
#include <GL/glew.h>
#include <assimp.hpp>      // C++ importer interface
#include <aiScene.h>       // Output data structure
#include <aiPostProcess.h> // Post processing flags
#include <aiMesh.h>

#include "util.h"
#include "math_3d.h"

struct Vertex
{
    Vector3f m_pos;
    Vector2f m_tex;
    Vector3f m_normal;

    Vertex() {}

    Vertex(const Vector3f& pos, const Vector2f& tex, const Vector3f& normal)
    {
        m_pos    = pos;
        m_tex    = tex;
        m_normal = normal;
    }
};


class MeshAssistant : public KisPaintingAssistant
{
public:
    MeshAssistant();
    void initialize();
    virtual QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin);
    virtual QPointF buttonPosition() const;
    virtual int numHandles() const { return 4; }
    virtual void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached = true, KisCanvas2* canvas=0);

protected:
    virtual QRect boundingRect() const;
    virtual void drawCache(QPainter& gc, const KisCoordinatesConverter *converter);
    void beginOpenGL();
    void endOpenGL();

private:
    bool InitFromScene(const aiScene* pScene);
    void InitMesh(unsigned int Index, const aiMesh* paiMesh);
    void Clear();
    char* m_filename;
    uint m_initialized;
    KisCanvas2* m_canvas;

#define INVALID_MATERIAL 0xFFFFFFFF
    struct MeshEntry {
        MeshEntry();

        ~MeshEntry();

        bool Init(const std::vector<Vertex>& Vertices,
                  const std::vector<unsigned int>& Indices);

        GLuint VB;
        GLuint IB;
        unsigned int NumIndices;
    };

    std::vector<MeshEntry> m_Entries;


};

class MeshAssistantFactory : public KisPaintingAssistantFactory
{
public:
    MeshAssistantFactory();
    virtual ~MeshAssistantFactory();
    virtual QString id() const;
    virtual QString name() const;
    virtual KisPaintingAssistant* createPaintingAssistant() const;
};



#endif

