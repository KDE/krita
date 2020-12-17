/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
