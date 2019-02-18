#ifndef KISGLIMAGEWIDGET_H
#define KISGLIMAGEWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QTransform>
#include <KisGLImageF16.h>


class KisGLImageWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    KisGLImageWidget(QWidget *parent = nullptr);
    KisGLImageWidget(QSurfaceFormat::ColorSpace colorSpace,
                     QWidget *parent = nullptr);

    void initializeGL();
    void paintGL();

    void loadImage(const KisGLImageF16 &image);

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    QSize sizeHint() const;

public Q_SLOTS:

private Q_SLOTS:
    void slotOpenGLContextDestroyed();

private:
    void updateVerticesBuffer(const QRect &rect);

private:
    KisGLImageF16 m_sourceImage;

    QScopedPointer<QOpenGLShaderProgram> m_shader;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_verticesBuffer;
    QOpenGLBuffer m_textureVerticesBuffer;
    QOpenGLTexture m_texture;

    bool m_havePendingTextureUpdate = false;
};

#endif // KISGLIMAGEWIDGET_H
