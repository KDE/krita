/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2004 Sven Langkamp <longamp@reallygood.de>
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

#ifndef _KIS_AUTOGRADIENT_H_ 
#define _KIS_AUTOGRADIENT_H_

#include "kis_wdg_autogradient.h"
#include "kis_gradient.h"

class KisAutogradientResource : public KisGradient
{
	public:
		KisAutogradientResource() : KisGradient("")
		{
		}
	public:
		void createSegment( int interpolation, int colorInterpolation, double startOffset, double endOffset, double middleOffset, QColor left, QColor right );
		const QValueVector<double> getHandlePositions() const;

		/** 
		 * Moves the StartOffset of the specified segment to the specified value
		 * and corrects the EndOffset of the previous segment.
		 * If the Segment is the first Segment the StartOffset will be set to 0.0 .
		 * The offset will maximally be moved till the middle of the current or the previous
		 * segment
		 */
		void moveSegmentStartOffset( KisGradientSegment* segment, double t);

		/** 
		 * Moves the EndOffset of the specified segment to the specified value
		 * and corrects the StartOffset of the following segment.
		 * If the Segment is the last Segment the EndOffset will be set to 1.0 .
		 * The offset will maximally be moved till the middle of the current or the following
		 * segment
		 */
		void moveSegmentEndOffset( KisGradientSegment* segment, double t);

		/** 
		 * Moves the Middle of the specified segment to the specified value
		 * The offset will maximally be moved till the EndOffset or StartOffset of the segment
		 */
		void moveSegmentMiddleOffset( KisGradientSegment* segment, double t);
	public:
		virtual bool loadAsync() { return false; };
};

class KisAutogradient : public KisWdgAutogradient
{
	Q_OBJECT
	public:
		KisAutogradient(QWidget *parent, const char* name, const QString& caption);;
	signals:
		void activatedResource(KisResource *r);
	private:
		KisAutogradientResource* m_autogradientResource;
	private slots:
		void slotSelectedSegment(KisGradientSegment* segment);
		void slotChangedInterpolation(int type);
		void slotChangedColorInterpolation(int type);
		void slotChangedLeftColor( const QColor& color);
		void slotChangedRightColor( const QColor& color);
};

#endif
