/*
 *  kis_resourceserver.h - part of KImageShop
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
#ifndef KIS_RESOURCESERVER_H_
#define KIS_RESOURCESERVER_H_

#include <qobject.h>
#include <qptrlist.h>
#include <qstring.h>

#include "kis_resource.h"
#include "kis_brush.h"
#include "kis_imagepipe_brush.h"
#include "kis_pattern.h"
#include "kis_gradient.h"
#include "kis_palette.h"
#include "kis_types.h"

// XXX: Encapsulate resources in shared pointers.
class KisResourceServer : public QObject {
	typedef QObject super;
	Q_OBJECT

public:
	KisResourceServer();
	virtual ~KisResourceServer();
	

	void loadBrushes();
	void loadpipeBrushes();
	void loadPatterns();
	void loadGradients();
	void loadPalettes();

public:
	Q_INT32 brushCount() const { return m_brushes.count(); }
	Q_INT32 pipebrushCount() const { return m_pipebrushes.count(); }
	Q_INT32 patternCount() const { return m_patterns.count(); }
	Q_INT32 gradientCount() const { return m_gradients.count(); }
	Q_INT32 paletteCount() const { return m_palettes.count(); }
	
	// XXX (BSAR): Does this mean the the lists are copied for every call?
	QPtrList<KisResource> brushes();
	QPtrList<KisResource> pipebrushes();
	QPtrList<KisResource> patterns();
	QPtrList<KisResource> gradients();
	QPtrList<KisResource> palettes();

signals:
	void loadedBrush(KisResource *br);
	void loadedpipeBrush(KisResource *br);
	void loadedPattern(KisResource *pat);
	void loadedGradient(KisResource *pat);
	void loadedPalette(KisResource *pal);
private:

	void loadBrush();
	void loadpipeBrush();
	void loadPattern();
	void loadGradient();
	void loadPalette();

	KisResourceServer(const KisResourceServer&);
	KisResourceServer& operator=(const KisResourceServer&);

private slots:
	void brushLoaded(KisResource *r);
	void brushLoadFailed(KisResource *br);

	void pipebrushLoaded(KisResource *r);
	void pipebrushLoadFailed(KisResource *br);

	void patternLoaded(KisResource *r);
	void patternLoadFailed(KisResource *br);

	void gradientLoaded(KisResource *r);
	void gradientLoadFailed(KisResource *br);

	void paletteLoaded(KisResource *r);
	void paletteLoadFailed(KisResource *br);

private:
	QPtrList<KisResource> m_brushes;
	QStringList m_brushFilenames;

	QPtrList<KisResource> m_pipebrushes;
	QStringList m_pipebrushFilenames;

	QPtrList<KisResource> m_patterns;
	QStringList m_patternFilenames;

	QPtrList<KisResource> m_gradients;
	QStringList m_gradientFilenames;

	QPtrList<KisResource> m_palettes;
	QStringList m_paletteFilenames;

};

#endif // KIS_RESOURCESERVER_H_

