/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __TIMELINE_FRAMES_MODEL_TESTING_H
#define __TIMELINE_FRAMES_MODEL_TESTING_H

#include <QAbstractTableModel>

#include <QScopedPointer>
#include <QIcon>

#include "kritaanimationdocker_export.h"

#include "timeline_frames_model_base.h"


class KRITAANIMATIONDOCKER_EXPORT TimelineFramesModelTesting : public TimelineFramesModelBase
{
    Q_OBJECT
public:
    TimelineFramesModelTesting(QObject *parent);
    ~TimelineFramesModelTesting();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __TIMELINE_FRAMES_MODEL_TESTING_H */
