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

#ifndef KOFAKEPROGRESSPROXY_H
#define KOFAKEPROGRESSPROXY_H

#include <KoProgressProxy.h>


/**
 * KoFakeProgressProxy is a simple class for using as a default sink of
 * progress for the algorithms for cases when the user is not interested
 * in progress reporting.
 *
 * Please note that KoFakeProgressProxy::instance() object can be used from
 * the context of multiple threads, so the class should have *no* state. If
 * you introduce any state, please make sure singleton object is removed from
 * the API.
 */
class KRITAWIDGETUTILS_EXPORT KoFakeProgressProxy : public KoProgressProxy
{
public:
    int maximum() const override;
    void setValue(int value) override;
    void setRange(int minimum, int maximum) override;
    void setFormat(const QString &format) override;
    void setAutoNestedName(const QString &name) override;

    static KoProgressProxy* instance();
};

#endif // KOFAKEPROGRESSPROXY_H
