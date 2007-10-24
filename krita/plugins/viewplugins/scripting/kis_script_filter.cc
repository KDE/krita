/*
 * This file is part of Krita
 *
 * Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_script_filter.h"

// kdelibs/kross
#include <kross/core/action.h>

class KisScriptFilter::Private
{
    public:
        Kross::Action* action;
        explicit Private(Kross::Action* a) : action(a) {}
};

KisScriptFilter::KisScriptFilter(Kross::Action* action) : KisFilter(KoID(action->name(),action->text()), KoID(action->property("categoryId").toString(), i18n(action->property("categoryName").toString().toUtf8().data() )), action->text()), d(new Private(action))
{
    d->action->addObject(this, "KritaFilter", Kross::ChildrenInterface::AutoConnectSignals);
}

KisScriptFilter::~KisScriptFilter()
{
    delete d;
}

QString KisScriptFilter::category() const
{
    //This is an example that demonstrates how to receive properties from within the
    //scripts.rc file. Such properties will be accessible from within scripting too.
    return d->action->property("category").toString();
}

void KisScriptFilter::process(const KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint& dstTopLeft, const QSize& size, const KisFilterConfiguration* config)
{
    Q_UNUSED(config);
    d->action->trigger();

    emit scriptProcess(new Scripting::ConstPaintDevice(src, 0), srcTopLeft, new Scripting::PaintDevice(dst, 0), dstTopLeft, size, 0);
    setProgressDone(); // Must be called even if you don't really support progression
}

bool KisScriptFilter::supportsPainting() const
{
    return d->action->property("supportsPainting").toBool();
}
bool KisScriptFilter::supportsPreview() const
{
    return d->action->property("supportsPreview").toBool();
}
bool KisScriptFilter::supportsAdjustmentLayers() const
{
    return d->action->property("supportsAdjustmentLayers").toBool();
}

bool KisScriptFilter::supportsIncrementalPainting() const
{
    return d->action->property("supportsIncrementalPainting").toBool();
}

bool KisScriptFilter::supportsThreading() const
{
    return d->action->property("supportsThreading").toBool();
}

#include "kis_script_filter.moc"
