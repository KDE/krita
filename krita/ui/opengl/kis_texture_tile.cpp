/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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


#include "kis_texture_tile.h"


#ifdef HAVE_OPENGL

#ifndef GL_BGRA
#define GL_BGRA 0x814F
#endif

inline QRectF relativeRect(const QRect &br /* baseRect */,
                           const QRect &cr /* childRect */,
                           const KisGLTexturesInfo *texturesInfo)
{
    const qreal x = qreal(cr.x() - br.x()) / texturesInfo->width;
    const qreal y = qreal(cr.y() - br.y()) / texturesInfo->height;
    const qreal w = qreal(cr.width()) / texturesInfo->width;
    const qreal h = qreal(cr.height()) / texturesInfo->height;

    return QRectF(x, y, w, h);
}


KisTextureTile::KisTextureTile(QRect imageRect, const KisGLTexturesInfo *texturesInfo,
                               const GLvoid *fillData, FilterMode filter)

    : m_tileRectInImagePixels(imageRect), m_texturesInfo(texturesInfo)
{
    m_textureRectInImagePixels =
        stretchRect(m_tileRectInImagePixels, texturesInfo->border);

    m_tileRectInTexturePixels = relativeRect(m_textureRectInImagePixels,
                                             m_tileRectInImagePixels,
                                             m_texturesInfo);

    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    switch(filter) {
        case NearestFilterMode:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            break;
        case BilinearFilterMode:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            break;
        case TrilinearFilterMode:
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            break;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0,
                 m_texturesInfo->format,
                 m_texturesInfo->width,
                 m_texturesInfo->height, 0,
                 GL_BGRA, GL_UNSIGNED_BYTE, fillData);
}

KisTextureTile::~KisTextureTile() {
    glDeleteTextures(1, &m_textureId);
}

void KisTextureTile::update(const KisTextureTileUpdateInfo &updateInfo) {
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (updateInfo.isEntireTileUpdated()) {

        glTexImage2D(GL_TEXTURE_2D, 0,
                     m_texturesInfo->format,
                     m_texturesInfo->width,
                     m_texturesInfo->height, 0,
                     GL_BGRA, m_texturesInfo->type,
                     updateInfo.data());
    } else {
        QPoint patchOffset = updateInfo.patchOffset();
        QSize patchSize = updateInfo.patchSize();

        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        patchOffset.x(), patchOffset.y(),
                        patchSize.width(), patchSize.height(),
                        GL_BGRA, m_texturesInfo->type,
                        updateInfo.data());
    }

    /**
     * On the boundaries of KisImage, there is a border-effect as well.
     * So we just repeat the bounding pixels of the image to make
     * bilinear interpolator happy.
     */
    repeatStripes(updateInfo);
}

inline void KisTextureTile::repeatStripes(const KisTextureTileUpdateInfo &updateInfo) {
    /**
     * WARN: This function can repeat stripes of 1px width only!
     */

    const int pixelSize = updateInfo.pixelSize();
    const QRect imageRect = updateInfo.imageRect();
    const QPoint patchOffset = updateInfo.patchOffset();
    const QSize patchSize = updateInfo.patchSize();
    const QRect patchRect = QRect(m_textureRectInImagePixels.topLeft() +
                                  patchOffset,
                                  patchSize);

    if(imageRect.top() == patchRect.top()) {
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        patchOffset.x(), patchOffset.y() - 1,
                        patchSize.width(), 1,
                        GL_BGRA, m_texturesInfo->type,
                        updateInfo.data());

    }

    if(imageRect.bottom() == patchRect.bottom()) {
        int shift = patchSize.width() * (patchSize.height() - 1) *
            pixelSize;

        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        patchOffset.x(), patchOffset.y() + patchSize.height(),
                        patchSize.width(), 1,
                        GL_BGRA, m_texturesInfo->type,
                        updateInfo.data() + shift);

    }

    if(imageRect.left() == patchRect.left()) {
        QByteArray columnBuffer(patchSize.height() * pixelSize, 0);

        quint8 *srcPtr = updateInfo.data();
        quint8 *dstPtr = (quint8*) columnBuffer.data();
        for(int i = 0; i < patchSize.height(); i++) {
            memcpy(dstPtr, srcPtr, pixelSize);

            srcPtr += patchSize.width() * pixelSize;
            dstPtr += pixelSize;
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        patchOffset.x() - 1, patchOffset.y(),
                        1, patchSize.height(),
                        GL_BGRA, m_texturesInfo->type,
                        columnBuffer.constData());
    }

    if(imageRect.right() == patchRect.right()) {
        QByteArray columnBuffer(patchSize.height() * pixelSize, 0);

        quint8 *srcPtr = updateInfo.data() + (patchSize.width() - 1) * pixelSize;
        quint8 *dstPtr = (quint8*) columnBuffer.data();
        for(int i = 0; i < patchSize.height(); i++) {
            memcpy(dstPtr, srcPtr, pixelSize);

            srcPtr += patchSize.width() * pixelSize;
            dstPtr += pixelSize;
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        patchOffset.x() + patchSize.width(), patchOffset.y(),
                        1, patchSize.height(),
                        GL_BGRA, m_texturesInfo->type,
                        columnBuffer.constData());
    }

}

void KisTextureTile::drawPoints() {
    /**
     * We create a float rect here to workaround Qt's
     * "history reasons" in calculation of right()
     * and bottom() coordinates of integer rects.
     */
    QRectF imageRect(m_tileRectInImagePixels);
    QPointF ipt;
    QPointF tpt;

    glBegin(GL_QUADS);

    ipt = imageRect.topLeft();
    tpt = m_tileRectInTexturePixels.topLeft();
    glTexCoord2f(tpt.x(), tpt.y());
    glVertex2f(ipt.x(), ipt.y());

    ipt = imageRect.topRight();
    tpt = m_tileRectInTexturePixels.topRight();
    glTexCoord2f(tpt.x(), tpt.y());
    glVertex2f(ipt.x(), ipt.y());

    ipt = imageRect.bottomRight();
    tpt = m_tileRectInTexturePixels.bottomRight();
    glTexCoord2f(tpt.x(), tpt.y());
    glVertex2f(ipt.x(), ipt.y());

    ipt = imageRect.bottomLeft();
    tpt = m_tileRectInTexturePixels.bottomLeft();
    glTexCoord2f(tpt.x(), tpt.y());
    glVertex2f(ipt.x(), ipt.y());

    glEnd();
}


#endif /* HAVE_OPENGL */

