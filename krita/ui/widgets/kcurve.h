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

#define CLAMP0255(x) (x)
class ImageCurves
{
public:
	virtual int getCurveValue(int x);
	virtual int setCurveValue(int x, int y);
	virtual void curvesCalculateCurve();
	virtual QPoint getCurvePoint(int p);
	virtual void setCurvePoint(int p, QPoint qp);
	virtual int getCurvePointX(int p);
	virtual void setCurvePointX(int p, int x);
};

class KCurve : public QWidget
{
Q_OBJECT

public:
	enum CurveType
	{
		CURVE_SMOOTH,
		CURVE_FREE
	};

	KCurve(int w, int h,                                      // Widget size.
		ImageCurves *curves,                      // Curves data instance to use.
		QWidget *parent=0,                                 // Parent widget instance.
		bool readOnly=false);                              // If true : widget with full edition mode capabilities.
                                                                    // If false : display curve data only without edition.
                 
	~KCurve();

    void reset(void);
    void curveTypeChanged(CurveType);
    void setCurveGuide(QColor color);
    

signals:
    
    void signalMouseMoved( int x, int y );
    void signalCurvesChanged(void);
            
protected:

    void paintEvent( QPaintEvent * );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );
    void leaveEvent ( QEvent * );
    
private:

    int                   m_clearFlag;          // Clear drawing zone with message.
    int                   m_leftmost;
    int                   m_rightmost;
    int                   m_grab_point;
    int                   m_last;
    
    bool                  m_readOnlyMode;
    bool                  m_guideVisible;
    
    QColor                m_colorGuide;
    
    QTimer               *m_blinkTimer;
    
    CurveType m_curveType;       // Scale to use for drawing.    
    ImageCurves *m_curves;             // Curves data instance.    
};


#endif /* KCURVE_H */
