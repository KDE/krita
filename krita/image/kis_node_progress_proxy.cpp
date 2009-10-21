/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#include "kis_node_progress_proxy.h"

#include <QApplication>

#include "kis_node.h"

struct KisNodeProgressProxy::Private {
    Private() : minimum(0), maximum(100), value(100), percentage(-1) {
    };
    KisNodeWSP node;
    int minimum;
    int maximum;
    int value;
    int percentage;
    bool computePercentage() {
        int old_percentage = percentage;
        if (value == maximum) {
            percentage = -1;
        } else {
            percentage = (100 * (value - minimum)) / (maximum - minimum);
            percentage = qBound(0, percentage, 100);
        }
        return old_percentage != percentage;
    }
};

KisNodeProgressProxy::KisNodeProgressProxy(KisNode* _node) : d(new Private)
{
    d->node = _node;
}

KisNodeProgressProxy::~KisNodeProgressProxy()
{
}

const KisNodeSP KisNodeProgressProxy::node() const
{
    return d->node;
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
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    d->value = _value;
    if (d->computePercentage()) {
        emit(percentageChanged(d->percentage, d->node));
    }
}

void KisNodeProgressProxy::setRange(int _minimum, int _maximum)
{
    d->minimum = _minimum;
    d->maximum = _maximum;
    if (d->computePercentage()) {
        emit(percentageChanged(d->percentage, d->node));
    }
}

void KisNodeProgressProxy::setFormat(const QString & _format)
{
    Q_UNUSED(_format);
}
