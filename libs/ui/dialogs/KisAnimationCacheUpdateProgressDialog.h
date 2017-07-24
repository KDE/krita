/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISANIMATIONCACHEUPDATEPROGRESSDIALOG_H
#define KISANIMATIONCACHEUPDATEPROGRESSDIALOG_H

#include <QObject>
#include <QScopedPointer>

#include "kis_types.h"

class KisTimeRange;
class KisViewManager;


class KisAnimationCacheUpdateProgressDialog : public QObject
{
    Q_OBJECT
public:
    explicit KisAnimationCacheUpdateProgressDialog(int busyWait = 200, QWidget *parent = 0);
    ~KisAnimationCacheUpdateProgressDialog() override;

    void regenerateRange(KisAnimationFrameCacheSP cache, const KisTimeRange &playbackRange, KisViewManager *viewManager);

Q_SIGNALS:

public Q_SLOTS:
    void slotFrameFinished();
    void slotFrameCancelled();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISANIMATIONCACHEUPDATEPROGRESSDIALOG_H
