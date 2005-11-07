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
#include <qpair.h>
#include <qsortedlist.h>
#include <koffice_export.h>
class KRITAUI_EXPORT KCurve : public QWidget
{
Q_OBJECT

public:
    KCurve(QWidget *parent = 0, const char *name = 0, WFlags f = 0);

    virtual ~KCurve();

    void reset(void);
    void setCurveGuide(QColor color);
    void setPixmap(QPixmap pix);


signals:
    
    void modified(void);
            
protected:

    void keyPressEvent(QKeyEvent *);
    void paintEvent(QPaintEvent *);
    void mousePressEvent (QMouseEvent * e);
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );
    void leaveEvent ( QEvent * );
    
public:
    static double getCurveValue(QPtrList<QPair<double,double> > &curve, double x);
    double getCurveValue(double x);
    
    QPtrList<QPair<double,double> > getCurve();
    void setCurve(QPtrList<QPair<double,double> >inlist);

private:
    double m_leftmost;
    double m_rightmost;
    QPair<double,double> *m_grab_point;
    bool m_dragging;
    double m_grabOffsetX;
    double m_grabOffsetY;
    
    bool m_readOnlyMode;
    bool m_guideVisible;
    QColor m_colorGuide;
    QPtrList<QPair<double,double> > m_points;
    QPixmap *m_pix;
};


#endif /* KCURVE_H */
