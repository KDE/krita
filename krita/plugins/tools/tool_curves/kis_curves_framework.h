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

class CurvePoint {

    KisPoint* m_point;
    bool m_pivot;
    bool m_selected; // Only pivots can be selected
    
public:

    /* Constructors and Destructor */
    
    CurvePoint ();
    CurvePoint (KisPoint&, bool, bool);
    CurvePoint (double, double, bool, bool);
    CurvePoint (QPoint&, bool, bool);
    CurvePoint (KoPoint&, bool, bool);
    
    virtual ~CurvePoint () {delete m_point;}
    
public:

    /* Generic Functions */

    bool operator!= (CurvePoint p2) const { if (p2.getPoint() != *m_point) return true; else return false;}
    
    KisPoint getPoint() {return *m_point;}
    
    bool setPoint(KisPoint);
    bool setPoint(double, double);
    bool setPoint(QPoint&);
    bool setPoint(KoPoint&);
    
    bool isPivot() {return m_pivot;}
    bool isSelected() {return m_selected;}
    
    void setPivot(bool p) {m_pivot = p;}
    void setSelected(bool s) {m_selected = ((m_pivot) ? s : false);}  /* Only pivots can be selected */
};


typedef QValueVector<CurvePoint> PointList;

class KisCurve {

    enum Status {DRAWING, EDITING, ENDING};

    PointList m_curve;
    PointList m_pivots;
    Status m_current;

    KisPaintDeviceSP m_dev;
    
public:
    
    KisCurve () {m_dev = 0; m_current=DRAWING;}
    KisCurve (KisPaintDevice dev) {m_dev = new KisPaintDevice(dev); m_current=DRAWING;}
    
    virtual ~KisCurve ();
    
public:

    CurvePoint& operator[](int i) {return m_curve[i];}
    
/*
    void operator+(CurvePoint pt);
    void operator+(KisCurve cv);
    void operator+(PointList cv);
*/

    void setPaintDevice(KisPaintDevice dev) {if (m_dev) delete m_dev; m_dev = new KisPaintDevice (dev);}
    void setStatus(Status st) {m_current = st;}

    int add(CurvePoint, int);
    int add(KisPoint, bool, bool, int);

    PointList getCurve() {return m_curve;}
    PointList getCurve(CurvePoint, CurvePoint);
    PointList getCurve(int, int);

    bool setPivot (CurvePoint);
    bool setPivot (int);
    
    PointList getPivots() {return m_pivots;}
    
    CurvePoint getLastPivot() {return m_pivots.last();}
    int getLastPivotIndex() {return m_pivots.count()-1;}

    bool deleteLastPivot();

    virtual bool selectPivot(CurvePoint);
    virtual bool selectPivot(int);

    virtual bool calculateCurve(KisPoint, KisPoint) = 0;
    virtual bool calculateCurve(CurvePoint, CurvePoint) = 0;

    virtual bool movePivot(CurvePoint, KisPoint);
    virtual bool movePivot(CurvePoint, CurvePoint);
    virtual bool movePivot(int, CurvePoint);

    virtual bool deletePivot(CurvePoint);
    virtual bool deletePivot(int);
};
