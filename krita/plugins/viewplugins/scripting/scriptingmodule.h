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

#ifndef SCRIPTINGMODULE_H
#define SCRIPTINGMODULE_H

#include <QString>
#include <QStringList>
#include <QObject>

class KisView;
class ScriptingProgress;

/**
* The ScriptingModule class enables access to the KSpread
* functionality from within the scripting backends.
*/
class ScriptingModule : public QObject
{
		Q_OBJECT
	public:
		ScriptingModule(KisView* view, ScriptingProgress* progress);
		virtual ~ScriptingModule();

	public slots:

		/**
		* Returns the \a KoApplicationAdaptor object.
		*/
		QObject* application();

#if 0
//TODO KisView::document() needs to be public to have this working!
		/**
		* Returns the \a KoDocumentAdaptor object.
		*/
		QObject* document();
#endif

		/**
		* Returns the \a ScriptingProgress object.
		*/
		QObject* progress();

	private:
		class Private;
		Private* const d;
};

#endif
