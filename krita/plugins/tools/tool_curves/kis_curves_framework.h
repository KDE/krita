/*
 *  Copyright (c) 2006 Emanuele Tamponi <emanuele@valinor.it>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* Initial Commit for the Curves Framework. E.T. */

#ifndef KIS_CURVES_FRAMEWORK_H_
#define KIS_CURVES_FRAMEWORK_H_

class CurvePoint {

    KisPoint m_point;
    bool m_pivot;
    bool m_selected; // Only pivots can be selected
    
public:

    /* Constructors and Destructor */
    
    CurvePoint ();
    CurvePoint (KisPoint&, bool, bool);
    CurvePoint (double, double, bool, bool);
    CurvePoint (QPoint&, bool, bool);
    CurvePoint (KoPoint&, bool, bool);
    
    ~CurvePoint () {}
    
public:

    /* Generic Functions */

    bool operator!= (CurvePoint p2) const { if (p2.getPoint() != m_point) return true; else return false;}
    
    KisPoint getPoint() {return m_point;}
    
    void setPoint(KisPoint);
    void setPoint(double, double);
    void setPoint(QPoint&);
    void setPoint(KoPoint&);
    
    bool isPivot() {return m_pivot;}
    bool isSelected() {return m_selected;}
    
    void setPivot(bool p) {m_pivot = p;}
    void setSelected(bool s) {m_selected = ((m_pivot) ? s : false);}  /* Only pivots can be selected */
};


typedef QValueVector<CurvePoint> PointList;

class KisCurve {

    enum tStatus {DRAWING, EDITING, ENDING};

    PointList m_curve;
    tStatus m_current;

protected:

    KisPaintDeviceSP m_dev;
    
public:
    
    KisCurve () {m_dev = 0; m_current=DRAWING;}
    KisCurve (KisPaintDevice& dev) {m_dev = new KisPaintDevice(dev); m_current=DRAWING;}
    
    virtual ~KisCurve () {m_curve.clear();}
    
public:

    CurvePoint& operator[](int i) {return m_curve[i];}
    
/*
    void operator+(CurvePoint pt);
    void operator+(KisCurve cv);
    void operator+(PointList cv);
*/

    void setPaintDevice(KisPaintDevice& dev) {if (m_dev) delete m_dev; m_dev = new KisPaintDevice (dev);}
    void setStatus(tStatus st) {m_current = st;}

    int add(CurvePoint, int = -1);
    int add(KisPoint, bool = false, bool = false, int = -1);

    int count() {return m_curve.count();}

    PointList getCurve() {return m_curve;}
    PointList getCurve(CurvePoint, CurvePoint);
    PointList getCurve(int, int);

    bool setPivot (CurvePoint);
    bool setPivot (int);

    void clear() {m_curve.clear();}
    void deleteLastPivot();

    virtual bool selectPivot(CurvePoint) {return true;}
    virtual bool selectPivot(int) {return true;}

    virtual bool calculateCurve(KisPoint, KisPoint) {return true;}
    virtual bool calculateCurve(CurvePoint, CurvePoint) {return true;}

    virtual bool movePivot(CurvePoint, KisPoint) {return true;}
    virtual bool movePivot(CurvePoint, CurvePoint) {return true;}
    virtual bool movePivot(int, CurvePoint) {return true;}

    virtual bool deletePivot(CurvePoint) {return true;}
    virtual bool deletePivot(int) {return true;}
};

#endif // KIS_CURVES_FRAMEWORK_H_
