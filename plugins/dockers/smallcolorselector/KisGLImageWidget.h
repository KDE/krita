/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

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
#include <opengl/KisSurfaceColorSpace.h>


class KisGLImageWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    KisGLImageWidget(QWidget *parent = nullptr);
    KisGLImageWidget(KisSurfaceColorSpace colorSpace,
                     QWidget *parent = nullptr);

    ~KisGLImageWidget();

    void initializeGL() override;
    void paintGL() override;

    void loadImage(const KisGLImageF16 &image);

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    QSize sizeHint() const override;

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
