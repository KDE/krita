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

#include "mesh_assistant.h"
#include <opengl/kis_opengl_canvas2.h>

MeshAssistant::MeshAssistant()
    : KisPaintingAssistant("mesh",i18n("Mesh assistant"))
{
    initialize();
    m_canvas = 0;
}

void MeshAssistant::initialize(){
    KUrl file = KFileDialog::getOpenUrl(KUrl(), QString("*.blend"));
    m_filename = file.toLocalFile().toUtf8().data();
    m_initialized = 1;

}

QPointF MeshAssistant::adjustPosition(const QPointF& pt, const QPointF& /*strokeBegin*/)
{
    return QPointF();
}

void MeshAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter)
{
    if(m_initialized){
        Assimp::Importer imp;
        const aiScene* scene = imp.ReadFile( m_filename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals  );
        if(!scene)
        {
            return;
        }
        InitFromScene(scene);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        for (unsigned int i = 0 ; i < m_Entries.size() ; i++) {
            glBindBuffer(GL_ARRAY_BUFFER, m_Entries[i].VB);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Entries[i].IB);


            glDrawElements(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT, 0);
        }

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        glEnable(GL_DEPTH_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_COLOR_MATERIAL);

    }
    else {
        initialize();
        drawCache(gc,converter);
    }

}

QRect MeshAssistant::boundingRect() const
{
    return QRect();
}

QPointF MeshAssistant::buttonPosition() const
{
    return QPointF();
}

void MeshAssistant::drawAssistant(QPainter &gc, const QRectF &updateRect, const KisCoordinatesConverter *converter, bool cached,KisCanvas2* canvas)
{

    m_canvas = canvas;
#if (HAVE_OPENGL)
    qDebug() << "opengl found:"<<m_canvas<<"\n";
    if(m_canvas->canvasIsOpenGL()){
        qDebug() << "Canvas is Opengl";
        beginOpenGL();
        qDebug() << "Opengl Begun\n";
        drawCache(gc,converter);
        qDebug() << "Cache Drawn\n";
    }
    else {
        qDebug()<<"canvas not opengl";
    }
    endOpenGL();
    qDebug()<<"opengl closed";
#endif
}

void MeshAssistant::beginOpenGL()
{
#if defined(HAVE_OPENGL)
    KisOpenGLCanvas2 *canvasWidget = dynamic_cast<KisOpenGLCanvas2 *>(m_canvas->canvasWidget());
    Q_ASSERT(canvasWidget);

    if (canvasWidget) {
        canvasWidget->beginOpenGL();
        canvasWidget->setupFlakeToWidgetTransformation();
    }
#endif
}

void MeshAssistant::endOpenGL()
{
#if defined(HAVE_OPENGL)
    KisOpenGLCanvas2 *canvasWidget = dynamic_cast<KisOpenGLCanvas2 *>(m_canvas->canvasWidget());
    Q_ASSERT(canvasWidget);

    if (canvasWidget) {
        canvasWidget->endOpenGL();
    }
#endif
}

MeshAssistantFactory::MeshAssistantFactory()
{
}

MeshAssistantFactory::~MeshAssistantFactory()
{
}

QString MeshAssistantFactory::id() const
{
    return "mesh";
}

QString MeshAssistantFactory::name() const
{
    return i18n("Mesh assistant");
}


KisPaintingAssistant* MeshAssistantFactory::createPaintingAssistant() const
{
    MeshAssistant* assistant = new MeshAssistant();
    return assistant;
}


MeshAssistant::MeshEntry::MeshEntry()
{
    VB = INVALID_OGL_VALUE;
    IB = INVALID_OGL_VALUE;
    NumIndices  = 0;
}

MeshAssistant::MeshEntry::~MeshEntry()
{
    if (VB != INVALID_OGL_VALUE)
    {
        glDeleteBuffers(1, &VB);
    }

    if (IB != INVALID_OGL_VALUE)
    {
        glDeleteBuffers(1, &IB);
    }
}

bool MeshAssistant::MeshEntry::Init(const std::vector<Vertex>& Vertices,
                          const std::vector<unsigned int>& Indices)
{
    NumIndices = Indices.size();

    glGenBuffers(1, &VB);
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &IB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * NumIndices, &Indices[0], GL_STATIC_DRAW);
    return true;
}


bool MeshAssistant::InitFromScene(const aiScene* pScene)
{
    m_Entries.resize(pScene->mNumMeshes);
    qDebug()<< pScene->mNumMeshes;
    // Initialize the meshes in the scene one by one
    for (unsigned int i = 0 ; i < m_Entries.size() ; i++) {
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitMesh(i, paiMesh);
    }

    return true;
}

void MeshAssistant::InitMesh(unsigned int Index, const aiMesh* paiMesh)
{
    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;
    qDebug() << "in initMesh";
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
    qDebug() << "in initMesh: before loop for vertices";
    for (unsigned int i = 0 ; i < paiMesh->mNumVertices ; i++) {
        const aiVector3D* pPos      = &(paiMesh->mVertices[i]);
        const aiVector3D* pNormal   = &(paiMesh->mNormals[i]);
        const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

        Vertex v(Vector3f(pPos->x, pPos->y, pPos->z),
                 Vector2f(pTexCoord->x, pTexCoord->y),
                 Vector3f(pNormal->x, pNormal->y, pNormal->z));

        Vertices.push_back(v);
    }
    qDebug() << "in initMesh: before loop for indices   " << paiMesh->mNumFaces;
    if(paiMesh->HasFaces()) {
        for (int i = 0 ; i < paiMesh->mNumFaces ; i++) {
            qDebug() << "in initMesh: before faces" << paiMesh->mNumFaces;
            aiFace Face = paiMesh->mFaces[i];
            qDebug() << "in initMesh: faces" << paiMesh->mNumFaces;
            for( uint j = 0; j < Face.mNumIndices; ++j ) {
                qDebug() << "in initMesh: indices" << i;
                Indices.push_back(Face.mIndices[j]);
            }
        }
    }
    qDebug() << "in initMesh: before init";
    m_Entries[Index].Init(Vertices, Indices);
}
