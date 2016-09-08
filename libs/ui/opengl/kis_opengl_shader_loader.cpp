#include "kis_opengl_shader_loader.h"

#include "opengl/kis_opengl.h"
#include "kis_config.h"

#include <QFile>
#include <QMessageBox>

#include <KLocalizedString>

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

KisOpenGLShaderLoader::KisOpenGLShaderLoader()
{

}

KisShaderProgram *KisOpenGLShaderLoader::loadShader(QString vertPath, QString fragPath,
                                                       QByteArray vertHeader, QByteArray fragHeader)
{
    bool result;

    KisShaderProgram *shader = new KisShaderProgram();

    // Load vertex shader
    QByteArray vertSource;

    vertSource.append(KisOpenGL::hasOpenGL3() ? "#version 150 core\n" : "#version 120");
    vertSource.append(vertHeader);
    QFile vertexShaderFile(":/" + vertPath);
    vertexShaderFile.open(QIODevice::ReadOnly);
    vertSource.append(vertexShaderFile.readAll());

    result = shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSource);
    reportShaderLinkFailedAndExit(result, "Adding vertex shader source", shader->log());

    // Load fragment shader
    QByteArray fragSource;

    fragSource.append(KisOpenGL::hasOpenGL3() ? "#version 150 core\n" : "#version 120");
    fragSource.append(fragHeader);
    QFile fragmentShaderFile(":/" + fragPath);
    fragmentShaderFile.open(QIODevice::ReadOnly);
    fragSource.append(fragmentShaderFile.readAll());

    result = shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSource);
    reportShaderLinkFailedAndExit(result, "Adding fragment shader source", shader->log());

    // Bind uniforms
    shader->bindAttributeLocation("a_vertexPosition", PROGRAM_VERTEX_ATTRIBUTE);
    shader->bindAttributeLocation("a_textureCoordinate", PROGRAM_TEXCOORD_ATTRIBUTE);

    // Link
    result = shader->link();
    reportShaderLinkFailedAndExit(result, "Linking shader", shader->log());

    Q_ASSERT(shader->isLinked());

    return shader;
}

KisShaderProgram *KisOpenGLShaderLoader::loadDisplayShader(KisDisplayFilter *displayFilter, bool useHiQualityFiltering)
{
    QByteArray fragHeader;

    if (KisOpenGL::hasOpenGL3()) {
        fragHeader.append("#define DIRECT_LOD_FETCH\n");
        if (useHiQualityFiltering) {
            fragHeader.append("#define HIGHQ_SCALING\n");
        }
    }

    bool haveDisplayFilter = displayFilter && !displayFilter->program().isEmpty();
    if (haveDisplayFilter) {
        fragHeader.append("#define USE_OCIO\n");
        fragHeader.append(displayFilter->program().toLatin1());
    }

    QString vertPath, fragPath;
    // Select appropriate vertex shader
    if (KisOpenGL::hasOpenGL3()) {
        vertPath = "matrix_transform.vert";
        fragPath = "highq_downscale.frag";
    } else {
        vertPath = "matrix_transform_legacy.vert";
        fragPath = "simple_texture_legacy.frag";
    }

    KisShaderProgram *shader = loadShader(vertPath, fragPath, QByteArray(), fragHeader);

    return shader;
}

KisShaderProgram *KisOpenGLShaderLoader::loadCheckerShader()
{
    QString vertPath, fragPath;
    // Select appropriate vertex shader
    if (KisOpenGL::hasOpenGL3()) {
        vertPath = "matrix_transform.vert";
        fragPath = "simple_texture.frag";
    } else {
        vertPath = "matrix_transform_legacy.vert";
        fragPath = "simple_texture_legacy.frag";
    }

    KisShaderProgram *shader = loadShader(vertPath, fragPath, QByteArray(), QByteArray());

    return shader;
}

KisShaderProgram *KisOpenGLShaderLoader::loadCursorShader()
{
    QString vertPath, fragPath;
    // Select appropriate vertex shader
    if (KisOpenGL::hasOpenGL3()) {
        vertPath = "cursor.vert";
        fragPath = "cursor.frag";
    } else {
        vertPath = "cursor_legacy.vert";
        fragPath = "cursor_legacy.frag";
    }

    KisShaderProgram *shader = loadShader(vertPath, fragPath, QByteArray(), QByteArray());

    return shader;
}

void KisOpenGLShaderLoader::reportShaderLinkFailedAndExit(bool result, const QString &context, const QString &log)
{
    KisConfig cfg;

    if (cfg.useVerboseOpenGLDebugOutput()) {
        dbgUI << "GL-log:" << context << log;
    }

    if (result) return;

    qDebug() << "Shader failed to compile/link/anything: " << context;
    //QMessageBox::critical(this, i18nc("@title:window", "Krita"),
    //                      QString(i18n("Krita could not initialize the OpenGL canvas:\n\n%1\n\n%2\n\n Krita will disable OpenGL and close now.")).arg(context).arg(log),
    //                      QMessageBox::Close);

    cfg.setUseOpenGL(false);
    cfg.setCanvasState("OPENGL_FAILED");
}
