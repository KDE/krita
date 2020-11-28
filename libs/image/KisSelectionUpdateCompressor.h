/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSELECTIONUPDATECOMPRESSOR_H
#define KISSELECTIONUPDATECOMPRESSOR_H

#include "kritaimage_export.h"
#include "kis_thread_safe_signal_compressor.h"

#include "kis_types.h"
#include <QRect>


class KisSelectionUpdateCompressor : public QObject
{
    Q_OBJECT
public:
    KisSelectionUpdateCompressor(KisSelection *selection);
    ~KisSelectionUpdateCompressor();

public Q_SLOTS:
    void requestUpdate(const QRect &updateRect);
    void tryProcessStalledUpdate();

private Q_SLOTS:
    void startUpdateJob();

private:
    KisSelection *m_parentSelection {0};
    KisThreadSafeSignalCompressor *m_updateSignalCompressor {0};
    QRect m_updateRect;
    bool m_fullUpdateRequested {false};

    bool m_hasStalledUpdate {false};
};

#endif // KISSELECTIONUPDATECOMPRESSOR_H
