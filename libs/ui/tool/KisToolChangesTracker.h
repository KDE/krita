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

#ifndef KISTOOLCHANGESTRACKER_H
#define KISTOOLCHANGESTRACKER_H

#include "kritaui_export.h"
#include <QScopedPointer>
#include "KisToolChangesTrackerData.h"

class KRITAUI_EXPORT KisToolChangesTracker :public QObject
{
    Q_OBJECT

public:
    KisToolChangesTracker();
    ~KisToolChangesTracker();

    void commitConfig(KisToolChangesTrackerDataSP state);
    void requestUndo();
    KisToolChangesTrackerDataSP lastState() const;
    void reset();

    bool isEmpty() const;

Q_SIGNALS:
    void sigConfigChanged(KisToolChangesTrackerDataSP state);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISTOOLCHANGESTRACKER_H
