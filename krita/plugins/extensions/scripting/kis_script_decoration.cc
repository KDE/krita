/*
 * This file is part of Krita
 *
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#include "kis_script_decoration.h"

#include <kross/core/action.h>

#include <kis_debug.h>

struct KisScriptDecoration::Private {
    Kross::Action* action;
};

KisScriptDecoration::KisScriptDecoration(Kross::Action* _action, KisView2* _view) :
        KisCanvasDecoration(_action->name(), _action->text(), _view), d(new Private)
{
    d->action = _action;
}

KisScriptDecoration::~KisScriptDecoration()
{
    delete d;
}

void KisScriptDecoration::drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter)
{
    dbgScript << "Call to drawDecoration";

    QVariantList list;
    list << qVariantFromValue((void*)&gc)
         << updateRect
         << qVariantFromValue((void*)converter);

    d->action->callFunction("drawDecoration", list);
}
