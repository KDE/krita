/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISASYNCANIMATIONFRAMESSAVINGRENDERER_H
#define KISASYNCANIMATIONFRAMESSAVINGRENDERER_H

#include <KisAsyncAnimationRendererBase.h>

class KisDocument;
class KisTimeSpan;

class KisAsyncAnimationFramesSavingRenderer : public KisAsyncAnimationRendererBase
{
    Q_OBJECT
public:
    KisAsyncAnimationFramesSavingRenderer(KisImageSP image,
                                          const QString &fileNamePrefix,
                                          const QString &fileNameSuffix,
                                          const QByteArray &outputMimeType,
                                          const KisTimeSpan &range,
                                          const int sequenceNumberingOffset,
                                          const bool onlyNeedsUniqueFrames,
                                          KisPropertiesConfigurationSP exportConfiguration);
    ~KisAsyncAnimationFramesSavingRenderer();

protected:
    void frameCompletedCallback(int frame, const KisRegion &requestedRegion) override;
    void frameCancelledCallback(int frame, CancelReason cancelReason) override;

Q_SIGNALS:
    void sigCompleteRegenerationInternal(int frame);
    void sigCancelRegenerationInternal(int frame, KisAsyncAnimationRendererBase::CancelReason cancelReason);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISASYNCANIMATIONFRAMESSAVINGRENDERER_H
