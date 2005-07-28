/* ============================================================
 * 
 * Copyright 2004-2005 by Gilles Caulier (original work as  digikam curveswidget)
 * Copyright 2005 by Casper Boemann (reworked to be generic)
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef KCURVE_H
#define KCURVE_H

// Qt includes.

#include <qwidget.h>
#include <qcolor.h>
#include <qsortedlist.h>

class KCurve : public QWidget
{
Q_OBJECT

public:
	KCurve(int w, int h,                                      // Widget size.
		QWidget *parent=0,                                 // Parent widget instance.
		bool readOnly=false);                              // If true : widget with full edition mode capabilities.
                                                                    // If false : display curve data only without edition.

	KCurve(QWidget *parent, const char *n);
	
	virtual ~KCurve();

	void reset(void);
	void setCurveGuide(QColor color);
    

signals:
    
	void signalCurvesChanged(void);
            
protected:

	void keyPressEvent(QKeyEvent *);
	void paintEvent(QPaintEvent *);
	void mousePressEvent (QMouseEvent * e);
	void mouseReleaseEvent ( QMouseEvent * e );
	void mouseMoveEvent ( QMouseEvent * e );
	void leaveEvent ( QEvent * );
    
public:
	double getCurveValue(double x);
	
private:
	struct dpoint {
		double x,y;
		bool operator <(dpoint &rhs){return x < rhs.x;};
		bool operator ==(dpoint &rhs){return x == rhs.x;};
	};
	double m_leftmost;
	double m_rightmost;
	dpoint *m_grab_point;
	bool m_dragging;
    
	bool m_readOnlyMode;
	bool m_guideVisible;
	QColor m_colorGuide;
	QSortedList<dpoint> m_points;
};


#endif /* KCURVE_H */
