/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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


class KisGLImageWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    KisGLImageWidget(QWidget *parent = nullptr);
    KisGLImageWidget(QSurfaceFormat::ColorSpace colorSpace,
                     QWidget *parent = nullptr);

    ~KisGLImageWidget();

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
