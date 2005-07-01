/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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

#ifndef KIS_TRANSFORM_VISITOR_H_
#define KIS_TRANSFORM_VISITOR_H_

#include "kis_types.h"
#include "kis_progress_subject.h"

class KisPaintDevice;
class KisProgressDisplayInterface;
class KisHLineIteratorPixel;
class KisVLineIteratorPixel;
class KisFilterStrategy;

	template <class iter> iter createIterator(KisPaintDevice *dev, Q_INT32 start, Q_INT32 lineNum, Q_INT32 len);
	template <> KisHLineIteratorPixel createIterator <KisHLineIteratorPixel> (KisPaintDevice *dev, Q_INT32 start, Q_INT32 lineNum, Q_INT32 len);
	template <> KisVLineIteratorPixel createIterator<KisVLineIteratorPixel>(KisPaintDevice *dev, Q_INT32 start, Q_INT32 lineNum, Q_INT32 len);

class KisTransformVisitor : public KisProgressSubject {
	typedef KisProgressSubject super;

public:
	KisTransformVisitor();
	~KisTransformVisitor();

	void visitKisPaintDevice(KisPaintDevice* dev);

	void transform(double  xscale, double  yscale, 
		Q_INT32  xshear, Q_INT32  yshear,
		Q_INT32  xtranslate, Q_INT32  ytranslate,
		KisProgressDisplayInterface *m_progress, enumFilterType filterType = MITCHELL_FILTER);

private:
	// XXX (BSAR): Why didn't we use the shared-pointer versions of the paint device classes?
	template <class T> void transformPass(KisPaintDevice *src, KisPaintDevice *dst, double xscale, Q_INT32  shear, Q_INT32 dx,   KisProgressDisplayInterface *m_progress, KisFilterStrategy *filterStrategy);

	void transformy(KisPaintDevice *src, KisPaintDevice *dst, double yscale, Q_INT32  shear, Q_INT32 dy,   KisProgressDisplayInterface *m_progress, KisFilterStrategy *filterStrategy);
	
private:
	KisPaintDevice* m_dev;
	// Implement KisProgressSubject
	bool m_cancelRequested;
	virtual void cancel() { m_cancelRequested = true; }
};

inline KisTransformVisitor::KisTransformVisitor()
{
}

inline KisTransformVisitor::~KisTransformVisitor()
{
}

inline void KisTransformVisitor::visitKisPaintDevice(KisPaintDevice* dev)
{
	m_dev = dev;
}
#endif // KIS_TRANSFORM_VISITOR_H_
