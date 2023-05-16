/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISLAYERTHUMBNAILCACHE_H
#define KISLAYERTHUMBNAILCACHE_H

#include <kritaui_export.h>
#include <QScopedPointer>
#include "kis_types.h"

class KisIdleTasksManager;


class KRITAUI_EXPORT KisLayerThumbnailCache : public QObject
{
    Q_OBJECT
public:
    KisLayerThumbnailCache();
    ~KisLayerThumbnailCache();

    void setImage(KisImageSP image);
    void setIdleTaskManager(KisIdleTasksManager *manager);
    void setImage(KisImageSP image, KisIdleTasksManager *manager);
    void setMaxSize(int maxSize);
    int maxSize() const;

    QImage thumbnail(KisNodeSP node) const;

    void notifyNodeAdded(KisNodeSP node);
    void notifyNodeRemoved(KisNodeSP node);

    void startThumbnailsUpdate();

    void clear();

Q_SIGNALS:
    void sigLayerThumbnailUpdated(KisNodeSP node);

private Q_SLOTS:
    void slotThumbnailGenerated(KisNodeSP node, int seqNo, int maxSize, const QImage &thumb);
private:
    void setIdleTaskManagerImpl(KisIdleTasksManager *manager);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISLAYERTHUMBNAILCACHE_H
