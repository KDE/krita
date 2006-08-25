/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Sebastian Sauer <mail@dipe.org>
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

#include "scriptingmodule.h"
#include "scriptingprogress.h"
#include "scriptingmonitor.h"

#include <kis_view.h>

#include <KoDocumentAdaptor.h>
#include <KoApplicationAdaptor.h>

#include <kapplication.h>
#include <kdebug.h>

class ScriptingModule::Private
{
	public:
		KisView* view;
		ScriptingProgress* progress;
};

ScriptingModule::ScriptingModule(KisView* view, ScriptingProgress* progress)
	: QObject()
	, d(new Private())
{
	setObjectName("Krita");
	d->view = view;
	d->progress = progress;

#if 0
	Kross::Manager::self().addObject(d->view->canvasSubject()->document(), "KritaDocument");
	Kross::Manager::self().addObject(d->view, "KritaView");
#endif
}

ScriptingModule::~ScriptingModule()
{
	delete d;
}

QObject* ScriptingModule::application()
{
	return KApplication::kApplication()->findChild< KoApplicationAdaptor* >();
}

#if 0
QObject* ScriptingModule::document()
{
	return d->view->document() ? d->view->document()->findChild< KoDocumentAdaptor* >() : 0;
}
#endif

QObject* ScriptingModule::progress()
{
	//return findChild< ScriptingProgress* >();
	return d->progress;
}

#include "scriptingmodule.moc"
