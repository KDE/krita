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
#ifndef KIS_TEXTURE_TILE_H_
#define KIS_TEXTURE_TILE_H_

#ifdef HAVE_OPENGL

#include "kis_texture_tile_update_info.h"


struct KisGLTexturesInfo {
    int width;
    int height;

    GLint format;
    GLint type;
};


class KisTextureTile
{
public:
    KisTextureTile(QRect imageRect, const KisGLTexturesInfo *texturesInfo,
                   const GLvoid *fillData)

        : m_tileRectInImagePixels(imageRect), m_texturesInfo(texturesInfo)
    {
        m_textureRectInImagePixels =
            m_tileRectInImagePixels/*.adjusted(-1,-1,1,1)*/;

//        m_tileRectInTexturePixels = QRectF(0.0, 0.0, 1.0, 1.0);

        glGenTextures(1, &m_textureId);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0,
                     m_texturesInfo->format,
                     m_texturesInfo->height, m_texturesInfo->width, 0,
                     GL_BGRA, GL_UNSIGNED_BYTE, fillData);
    }

    ~KisTextureTile() {
        glDeleteTextures(1, &m_textureId);
    }

    inline void update(KisTextureTileUpdateInfo updateInfo) {
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        if (updateInfo.isEntireTileUpdated()) {
            glTexImage2D(GL_TEXTURE_2D, 0,
                         m_texturesInfo->format,
                         m_texturesInfo->height, m_texturesInfo->width, 0,
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

    }

    inline void drawPoints() {
        /**
         * We create a float rect here to workaround Qt's
         * "history reasons" in calculation of right()
         * and bottom() coordinates of integer rects.
         */
        QRectF imageRect(m_tileRectInImagePixels);
        QPointF pt;

        glBegin(GL_QUADS);

        pt = imageRect.topLeft();
        glTexCoord2f(0.0, 0.0);
        glVertex2f(pt.x(), pt.y());

        pt = imageRect.topRight();
        glTexCoord2f(1.0, 0.0);
        glVertex2f(pt.x(), pt.y());

        pt = imageRect.bottomRight();
        glTexCoord2f(1.0, 1.0);
        glVertex2f(pt.x(), pt.y());

        pt = imageRect.bottomLeft();
        glTexCoord2f(0.0, 1.0);
        glVertex2f(pt.x(), pt.y());

        glEnd();
    }

//    inline QRectF tileRectInTexturePixels() {
//        return m_tileRectInTexturePixels;
//    }

    inline QRect tileRectInImagePixels() {
        return m_tileRectInImagePixels;
    }

    inline GLuint textureId() {
        return m_textureId;
    }

//    inline QRect textureRectInImagePixels() {
//        return m_textureRectInImagePixels;
//    }


private:
    GLuint m_textureId;

    QRect m_tileRectInImagePixels;
//    QRectF m_tileRectInTexturePixels;
    QRect m_textureRectInImagePixels;

    const KisGLTexturesInfo *m_texturesInfo;

    Q_DISABLE_COPY(KisTextureTile);
};


#endif /* HAVE_OPENGL */

#endif /* KIS_TEXTURE_TILE_H_ */

