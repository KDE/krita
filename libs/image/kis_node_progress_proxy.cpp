/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_progress_proxy.h"

#include <QApplication>

#include "kis_node.h"

struct Q_DECL_HIDDEN KisNodeProgressProxy::Private {
    Private()
        : minimum(0)
        , maximum(100)
        , value(100)
        , percentage(-1)
    {
    }

    KisNodeWSP node;
    int minimum;
    int maximum;
    int value;
    int percentage;

    bool computePercentage() {
        int old_percentage = percentage;
        if (value == maximum) {
            percentage = -1;
        } else if (minimum == maximum && minimum == 0) {
            percentage = 0;
        } else {
            percentage = (100 * (value - minimum)) / (maximum - minimum);
            percentage = qBound(0, percentage, 100);
        }
        return old_percentage != percentage;
    }
};

KisNodeProgressProxy::KisNodeProgressProxy(KisNode* _node)
    : d(new Private)
{
    d->node = _node;
}

KisNodeProgressProxy::~KisNodeProgressProxy()
{
    delete d;
}

void KisNodeProgressProxy::prepareDestroying()
{
    d->node = 0;
}


int KisNodeProgressProxy::maximum() const
{
    return d->maximum;
}

int KisNodeProgressProxy::percentage() const
{
    return d->percentage;
}

void KisNodeProgressProxy::setValue(int _value)
{
    d->value = _value;
    if (d->node && d->computePercentage()) {
        emit(percentageChanged(d->percentage, d->node));
    }
}

void KisNodeProgressProxy::setRange(int _minimum, int _maximum)
{
    d->minimum = _minimum;
    d->maximum = _maximum;
    if (d->node && d->computePercentage()) {
        emit(percentageChanged(d->percentage, d->node));
    }
}

void KisNodeProgressProxy::setFormat(const QString & _format)
{
    Q_UNUSED(_format);
}
