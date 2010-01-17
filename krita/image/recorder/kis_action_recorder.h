/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_ACTION_RECORDER_H_
#define _KIS_ACTION_RECORDER_H_

#include <recorder/kis_macro.h>

class KisRecordedAction;

/**
 * This class record actions and allow other \ref KisMacro to connect to it
 * to get the action that are currently created.
 */
class KRITAIMAGE_EXPORT KisActionRecorder : public KisMacro
{

    Q_OBJECT

public:
    KisActionRecorder();
    ~KisActionRecorder();

public slots:

    virtual void addAction(const KisRecordedAction& action, const KisRecordedAction* before = 0);

signals:
    /**
     * This signal is emitted each time an action is added to this recorder.
     */
    void addedAction(const KisRecordedAction& action);
};

#endif
