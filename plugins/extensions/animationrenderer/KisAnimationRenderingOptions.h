/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KISANIMATIONRENDERINGOPTIONS_H
#define KISANIMATIONRENDERINGOPTIONS_H

#include <QString>
#include "kis_properties_configuration.h"

struct KisAnimationRenderingOptions
{
    KisAnimationRenderingOptions();

    QString lastDocuemntPath;
    QString videoMimeType;
    QString frameMimeType;

    QString basename;
    QString directory;
    int firstFrame = 0;
    int lastFrame = 0;
    int sequenceStart = 0;

    bool shouldEncodeVideo = false;
    bool shouldDeleteSequence = false;
    bool includeAudio = false;

    QString ffmpegPath;
    int frameRate = 25;
    int width = 0;
    int height = 0;
    QString videoFileName;

    QString customFFMpegOptions;
    KisPropertiesConfigurationSP frameExportConfig;

    QString resolveAbsoluteDocumentFilePath(const QString &documentPath) const;
    QString resolveAbsoluteVideoFilePath(const QString &documentPath) const;
    QString resolveAbsoluteFramesDirectory(const QString &documentPath) const;

    QString resolveAbsoluteVideoFilePath() const;
    QString resolveAbsoluteFramesDirectory() const;


    enum RenderMode {
        RENDER_FRAMES_ONLY,
        RENDER_VIDEO_ONLY,
        RENDER_FRAMES_AND_VIDEO
    };

    RenderMode renderMode() const;


    KisPropertiesConfigurationSP toProperties() const;
    void fromProperties(KisPropertiesConfigurationSP config);

};

#endif // KISANIMATIONRENDERINGOPTIONS_H
