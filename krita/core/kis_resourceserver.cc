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

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kinstance.h>

#include "kis_factory.h"
#include "kis_resourceserver.h"


KisResourceServer::KisResourceServer()
{
	m_brushes.setAutoDelete(true);
	m_pipebrushes.setAutoDelete(true);
	m_patterns.setAutoDelete(true);
}

KisResourceServer::~KisResourceServer()
{
	m_brushes.clear();
	m_pipebrushes.clear();
	m_patterns.clear();
}

void KisResourceServer::loadBrushes()
{
	m_brushFilenames += KisFactory::global() -> dirs() -> findAllResources("kis_brushes", "*.gbr");
	loadBrush();
}

void KisResourceServer::loadpipeBrushes()
{
	m_pipebrushFilenames += KisFactory::global() -> dirs() -> findAllResources("kis_brushes", "*.gih");
	loadpipeBrush();
}


void KisResourceServer::loadPatterns()
{
	m_patternFilenames += KisFactory::global() -> dirs() -> findAllResources("kis_patterns", "*.pat");
	loadPattern();
}

void KisResourceServer::loadBrush()
{
	if (!m_brushFilenames.empty()) {
		QString front = *m_brushFilenames.begin();
		KisBrush *brush;

		m_brushFilenames.pop_front();
		brush = new KisBrush(front);
		connect(brush, SIGNAL(loadComplete(KisResource*)), SLOT(brushLoaded(KisResource*)));
		connect(brush, SIGNAL(ioFailed(KisResource*)), SLOT(brushLoadFailed(KisResource*)));

		if (!brush -> loadAsync())
			loadBrush();
	}
}

void KisResourceServer::loadpipeBrush()
{
	if (!m_pipebrushFilenames.empty()) {
		QString front = *m_pipebrushFilenames.begin();
		KisImagePipeBrush *brush;

		m_pipebrushFilenames.pop_front();
		brush = new KisImagePipeBrush(front);
		connect(brush, SIGNAL(loadComplete(KisResource*)), SLOT(pipebrushLoaded(KisResource*)));
		connect(brush, SIGNAL(ioFailed(KisResource*)), SLOT(pipebrushLoadFailed(KisResource*)));

		if (!brush -> loadAsync())
			loadpipeBrush();
	}
}


void KisResourceServer::loadPattern()
{
	if (!m_patternFilenames.empty()) {
		QString front = *m_patternFilenames.begin();
		KisPattern *pattern;
		m_patternFilenames.pop_front();
		pattern = new KisPattern(front);

		connect(pattern, SIGNAL(loadComplete(KisResource*)), SLOT(patternLoaded(KisResource*)));
		connect(pattern, SIGNAL(ioFailed(KisResource*)), SLOT(patternLoadFailed(KisResource*)));

		if (!pattern -> loadAsync())
			loadPattern();
	}
}

void KisResourceServer::brushLoaded(KisResource *br)
{
	if (br && br -> valid()) {
		m_brushes.append(br);
		Q_ASSERT(br);
		emit loadedBrush(br);
	} else {
		delete br;
	}

	loadBrush();
}

void KisResourceServer::pipebrushLoaded(KisResource *br)
{
	if (br && br -> valid()) {
		m_pipebrushes.append(br);
		Q_ASSERT(br);
		emit loadedpipeBrush(br);
	} else {
		delete br;
	}

	loadpipeBrush();
}


void KisResourceServer::patternLoaded(KisResource *pat)
{
	if (pat && pat -> valid()) {
		m_patterns.append(pat);
		Q_ASSERT(pat);
		emit loadedPattern(pat);
	} else {
		delete pat;
	}

	loadPattern();
}
void KisResourceServer::brushLoadFailed(KisResource *r)
{
	delete r;
	loadBrush();
}

void KisResourceServer::pipebrushLoadFailed(KisResource *r)
{
	delete r;
	loadpipeBrush();
}

void KisResourceServer::patternLoadFailed(KisResource *r)
{
	kdDebug() << "loading pattern failed\n";
	delete r;
	loadPattern();
}


QPtrList<KisResource> KisResourceServer::brushes()
{
	if (m_brushes.isEmpty())
		loadBrushes();

	return m_brushes;
}


QPtrList<KisResource> KisResourceServer::pipebrushes()
{
	if (m_pipebrushes.isEmpty())
		loadpipeBrushes();

	return m_pipebrushes;
}

QPtrList<KisResource> KisResourceServer::patterns()
{
	if (m_patterns.isEmpty())
		loadPatterns();

	return m_patterns;
}

#include "kis_resourceserver.moc"
