/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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
    KisSelection *m_parentSelection;
    KisThreadSafeSignalCompressor *m_updateSignalCompressor;
    QRect m_updateRect;
    bool m_fullUpdateRequested;

    bool m_hasStalledUpdate;
};

#endif // KISSELECTIONUPDATECOMPRESSOR_H
