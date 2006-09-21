/*
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
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

#ifndef KIS_CANVAS_OBSERVER_H_
#define KIS_CANVAS_OBSERVER_H_

class KisCanvasSubject;

/**
 * This is the base interface plugins use to implement the Observer
 * design pattern. Observer can register themselves with an implementation
 * of KisCanvasSubject. The KisCanvasSubject will then call update()
 * on all registered observers whenever something interesting has happened.
 *
 * (This is something my predecessor should have done with signals and slots,
 * I think...)
 */
class KisCanvasObserver {
public:
    KisCanvasObserver() {};
    virtual ~KisCanvasObserver() {};

public:
    /**
     * Implement this function to query the KisCanvasSubject implementation
     * about state that may be interesting, such as current paint color and
     * so on.
     *
     * @param subject the KisCanvasSubject that may know something that's 
     *                interesting for us.
     */
    virtual void update(KisCanvasSubject *subject) = 0;

private:
    KisCanvasObserver(const KisCanvasObserver&);
    KisCanvasObserver& operator=(const KisCanvasObserver&);
};

#endif // KIS_CANVAS_OBSERVER_H_

