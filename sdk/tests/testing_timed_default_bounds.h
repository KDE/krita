/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef __TESTING_TIMED_DEFAULT_BOUNDS_H
#define __TESTING_TIMED_DEFAULT_BOUNDS_H

#include "kis_default_bounds_base.h"


namespace TestUtil {

struct TestingTimedDefaultBounds : public KisDefaultBoundsBase {
    TestingTimedDefaultBounds(const QRect &bounds = QRect(0,0,100,100))
        : m_time(0),
          m_lod(0),
          m_bounds(bounds)
    {
    }

    QRect bounds() const override {
        return m_bounds;
    }

    bool wrapAroundMode() const override {
        return false;
    }

    int currentLevelOfDetail() const override {
        return m_lod;
    }

    int currentTime() const override {
        return m_time;
    }

    bool externalFrameActive() const override {
        return false;
    }

    void testingSetTime(int time) {
        m_time = time;
    }

    void testingSetLod(int lod) {
        m_lod = lod;
    }

    void * sourceCookie() const override {
        return 0;
    }

private:
    int m_time;
    int m_lod;
    QRect m_bounds;
};

}

#endif /* __TESTING_TIMED_DEFAULT_BOUNDS_H */
