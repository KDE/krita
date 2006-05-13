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

#include <QWidget>
#include <QColor>
#include <QPair>
#include <QPixmap>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QPaintEvent>
#include <QList>

#include <krita_export.h>

class KRITAUI_EXPORT KCurve : public QWidget
{
Q_OBJECT

public:
    KCurve(QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);

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
    static double getCurveValue(const QList<QPair<double,double> > &curve, double x);
    double getCurveValue(double x);

    QList<QPair<double,double> > getCurve();
    void setCurve(QList<QPair<double,double> > inlist);

private:
    int nearestPointInRange(double x, double y) const;

    int m_grab_point_index;
    bool m_dragging;
    double m_grabOffsetX;
    double m_grabOffsetY;

    bool m_readOnlyMode;
    bool m_guideVisible;
    QColor m_colorGuide;
    QList<QPair<double,double> > m_points;
    QPixmap m_pix;
};


#endif /* KCURVE_H */
