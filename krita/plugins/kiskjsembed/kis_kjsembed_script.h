 /* This file is part of the KDE project
   Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KIS_KJSEMBED_SCRIPT_H_
#define _KIS_KJSEMBED_SCRIPT_H_

#include <qobject.h>
#include <qstring.h>

namespace KJSEmbed {
	class KJSEmbedPart;
};

class KisKJSEmbedScript : public QObject {
	Q_OBJECT
	public:
		KisKJSEmbedScript(KJSEmbed::KJSEmbedPart* jsembedpart, const QString& script);
	public:
		static KisKJSEmbedScript* loadFromFile(KJSEmbed::KJSEmbedPart* jsembedpart, const QString& file);
	public slots:
		void execute();
	private:
		KJSEmbed::KJSEmbedPart* m_jsEmbedPart;
		QString m_script;
};

#endif
