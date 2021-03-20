/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISASYNCANIMATIONFRAMESSAVEDIALOG_H
#define KISASYNCANIMATIONFRAMESSAVEDIALOG_H

#include "KisAsyncAnimationRenderDialogBase.h"
#include "kis_types.h"


class KRITAUI_EXPORT KisAsyncAnimationFramesSaveDialog : public KisAsyncAnimationRenderDialogBase
{
public:
    KisAsyncAnimationFramesSaveDialog(KisImageSP image,
                                      const KisTimeSpan &range,
                                      const QString &baseFilename,
                                      int sequenceNumberingOffset,
                                      bool onlyNeedsUniqueFrames,
                                      KisPropertiesConfigurationSP exportConfiguration);

    ~KisAsyncAnimationFramesSaveDialog();

    Result regenerateRange(KisViewManager *viewManager) override;

    QString savedFilesMask() const;
    QString savedFilesMaskWildcard() const;

    QList<int> getUniqueFrames() const;

protected:
    QList<int> calcDirtyFrames() const override;
    KisAsyncAnimationRendererBase* createRenderer(KisImageSP image) override;
    void initializeRendererForFrame(KisAsyncAnimationRendererBase *renderer,
                                    KisImageSP image, int frame) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISASYNCANIMATIONFRAMESSAVEDIALOG_H
