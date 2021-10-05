/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISPRESETSHADOWUPDATER_H
#define KISPRESETSHADOWUPDATER_H

#include <QObject>
#include <QScopedPointer>
#include <KoResourceCacheInterface.h>

class KisViewManager;

class KisPresetShadowUpdater : public QObject
{
    Q_OBJECT
public:
    KisPresetShadowUpdater(KisViewManager *view);
    ~KisPresetShadowUpdater();


public Q_SLOTS:
    void slotCanvasResourceChanged(int key, const QVariant & value);
    void slotPresetChanged();
    void slotStartPresetPreparation();

    void slotCacheGenerationFinished(int sequenceNumber,
                                     KoResourceCacheInterfaceSP cacheInterface);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISPRESETSHADOWUPDATER_H
