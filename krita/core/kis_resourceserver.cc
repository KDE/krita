/*
 *  kis_resourceserver.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <qimage.h>
#include <qstringlist.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kinstance.h>
#include "kis_factory.h"
#include "kis_resourceserver.h"

KisResourceServer::KisResourceServer()
{
	m_brushes.setAutoDelete(true);
	m_patterns.setAutoDelete(true);
	loadPatterns();
}

KisResourceServer::~KisResourceServer()
{
	m_brushes.clear();
	m_patterns.clear();
}

void KisResourceServer::loadBrushes()
{
	m_brushFilnames += KisFactory::global() -> dirs() -> findAllResources("kis_brushes", "*.gbr");
	loadBrush();
}

void KisResourceServer::loadPatterns()
{
	QStringList formats;
	QStringList lst;
	QStringList list = QImage::inputFormatList();

	for (QStringList::Iterator it = list.begin(); it != list.end(); it++)
		formats << QString("*.%1").arg((*it).lower());

	for (QStringList::Iterator it = formats.begin(); it != formats.end(); it++) {
		QStringList l = KisFactory::global() -> dirs() -> findAllResources("kis_pattern", *it, false, true);

		lst += l;
	}

	for (QStringList::Iterator it = lst.begin(); it != lst.end(); it++)
		loadPattern(*it);
}

void KisResourceServer::loadBrush()
{
	if (!m_brushFilnames.empty()) {
		QString front = *m_brushFilnames.begin();
		KisBrush *brush;

		m_brushFilnames.pop_front();
		brush = new KisBrush(front);
		connect(brush, SIGNAL(loadComplete(KisResource*)), SLOT(brushLoaded(KisResource*)));
		connect(brush, SIGNAL(ioFailed(KisResource*)), SLOT(resourceLoadFailed(KisResource*)));

		if (!brush -> loadAsync())
			loadBrush();
	}
}

const KisPattern *KisResourceServer::loadPattern(const QString& filename)
{
	KisPattern *pattern = new KisPattern( filename );

	if (pattern -> isValid()) {
		m_patterns.append(pattern);
	} else {
		delete pattern;
		pattern = 0;
	}

	return pattern;
}

void KisResourceServer::brushLoaded(KisResource *br)
{
	if (br && br -> valid()) {
		m_brushes.append(br);
		Q_ASSERT(dynamic_cast<KisBrush*>(br));
		emit loadedBrush(static_cast<KisBrush*>(br));
	} else {
		delete br;
	}

	loadBrush();
}

void KisResourceServer::resourceLoadFailed(KisResource *r)
{
	delete r;
	loadBrush();
}

QPtrList<KisResource> KisResourceServer::brushes()
{ 
	if (m_brushes.isEmpty())
		loadBrushes();

	return m_brushes; 
}

QPtrList<KoIconItem> KisResourceServer::patterns() const 
{ 
	return m_patterns; 
}

#include "kis_resourceserver.moc"

