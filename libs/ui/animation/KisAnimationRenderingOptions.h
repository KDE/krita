/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KISANIMATIONRENDERINGOPTIONS_H
#define KISANIMATIONRENDERINGOPTIONS_H

#include <QString>
#include "kis_properties_configuration.h"

#include "kritaui_export.h"

class KRITAUI_EXPORT KisAnimationRenderingOptions
{
public:
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
    bool wantsOnlyUniqueFrameSequence = false;

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
