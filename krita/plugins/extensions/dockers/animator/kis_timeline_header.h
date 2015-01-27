/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_TIMELINE_HEADER_H
#define KIS_TIMELINE_HEADER_H

#include <QWidget>

#include "kis_frame_box.h"

/**
 * The widget class for the timeline header
 * displaying the frame numbers.
 */
class KisTimelineHeader : public QWidget
{
    Q_OBJECT
public:
    KisTimelineHeader(KisFrameBox* parent = 0);

protected:
    void paintEvent(QPaintEvent *event);
};

#endif // KIS_TIMELINE_HEADER_H
