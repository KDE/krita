#include <qglwidget>
#include <kis_opengl_shader.h>
#include <kis_painter_device.h>

#include "kis_glsl_widget.h"

KisGlslWidget::KisGlslWidget(QWidget *parent) : QGLWidget(parent)
{
    
}

KisGlslWidget::~KisGlslWidget()
{
}

void KisGlslWidget::initializeGL()
{
    int err = glewInit();
    
    //if glew can't initialize, everything following is useless
    if(GLEW_OK != err) {
        qDebug("Unable to initialize glew, usefule information follows");
        qDebug("OpenGL version: %s", glGetString(GL_VERSION));
        qDebug("Error: %s", glewGetErrorString(err));
        m_valid = false;
        return;
    }
    
    //if glew can't find support for the needed features, 
    //everything following is useless as well
    if (glewIsSupported("GL_VERSION_2_0") != GL_TRUE ||
        glewGetExtension("GL_ARB_fragment_shader")      != GL_TRUE ||
        glewGetExtension("GL_ARB_vertex_shader")        != GL_TRUE ||
        glewGetExtension("GL_ARB_shader_objects")       != GL_TRUE ||
        glewGetExtension("GL_ARB_shading_language_100") != GL_TRUE ||
        glewGetExtension("GL_EXT_framebuffer_object")   != GL_TRUE ||
        glewGetExtension("GL_ARB_texture_rectangle")    != GL_TRUE )
    {
        QMessageBox::warning( this, i18n( "Krita" ), i18n( "The OpenGL filter cannot run. Your graphics card or driver is missing the necessary extensions" ) );
        m_valid = false;
        return;
    }
    
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glShadeModel(GL_SMOOTH); // Enables Smooth Shading
    glEnable(GL_DEPTH_TEST); // Enables Depth Testing
    glDepthFunc(GL_LEQUAL); // The Type Of Depth Test To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective
    
    //Setup orthogonal rendering
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewPort(0.0, 0.0, (GLfloat)imageWidth, (GLfloat)imageHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (GLfloat)imageWidth, 0.0, (GLfloat)imageHeight);
    
    //bind the texture from krita using readBytes
    //quint8* imgbuf = readBytes()
    glGenTexture(1, m_texture);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_texture);
    
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAX_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, imageWidth, imageHeight,
                 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, imgbuf);
    
}