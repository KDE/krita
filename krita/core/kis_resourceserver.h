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
#if !defined KIS_RESOURCESERVER_H_
#define KIS_RESOURCESERVER_H_

#include <qobject.h>
#include <qptrlist.h>
#include <qstring.h>

#include "kis_resource.h"
#include "kis_brush.h"
#include "kis_pattern.h"

class KisResourceServer : public QObject {
	typedef QObject super;
	Q_OBJECT

public:
	KisResourceServer();
	virtual ~KisResourceServer();

public:
	Q_INT32 brushCount() const { return m_brushes.count(); }
	Q_INT32 patternCount() const { return m_patterns.count(); }

	QPtrList<KisResource> brushes();
	QPtrList<KisResource> patterns();

	void loadBrushes();
	void loadPatterns();

signals:
	void loadedBrush(KisBrush *br);
	void loadedPattern(KisPattern *pat);
	
private:
	void loadBrush();
	void loadPattern();

private slots:
	void brushLoaded(KisResource *r);
	void brushLoadFailed(KisResource *br);

	void patternLoaded(KisResource *r);
	void patternLoadFailed(KisResource *br);

private:
	QPtrList<KisResource> m_patterns;
	QStringList m_brushFilenames;

	QPtrList<KisResource> m_brushes;
	QStringList m_patternFilenames;
};

#endif // KIS_RESOURCESERVER_H_

