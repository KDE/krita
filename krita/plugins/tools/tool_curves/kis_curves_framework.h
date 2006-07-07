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
    CurvePoint (KisPoint&, bool = false, bool = false);
    CurvePoint (double, double, bool = false, bool = false);
    CurvePoint (QPoint&, bool = false, bool = false);
    CurvePoint (KoPoint&, bool = false, bool = false);
    
    ~CurvePoint () {}
    
public:

    /* Generic Functions */

    bool operator!= (KisPoint p2) const { if (p2 != m_point) return true; else return false; }
    bool operator!= (CurvePoint p2) const { if (p2.getPoint() != m_point) return true; else return false; }

    bool operator== (KisPoint p2) const { if (p2 == m_point) return true; else return false; }
    bool operator== (CurvePoint p2) const { if (p2.getPoint() == m_point) return true; else return false; }
    
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


typedef QValueList<CurvePoint> PointList;
typedef QValueList<CurvePoint>::iterator CurveIterator;

class KisCurve {

    PointList m_curve;
    
public:
    
    KisCurve () {}
    
    virtual ~KisCurve () {m_curve.clear();}
    
public:

    CurvePoint& operator[](int i) {return m_curve[i];}

    CurveIterator add(CurvePoint, CurveIterator = 0);
    CurveIterator add(KisPoint, bool = false, bool = false, CurveIterator = 0);

    CurveIterator addPivot(CurvePoint, CurveIterator = 0);
    CurveIterator addPivot(KisPoint, bool = false, CurveIterator = 0);

    int count() {return m_curve.count();}
    bool isEmpty() {return m_curve.isEmpty();}
    CurveIterator begin() {return m_curve.begin();}
    CurveIterator end() {return m_curve.end();}
    CurvePoint first() {return m_curve.front();}
    CurvePoint last() {return m_curve.back();}
    void clear() {m_curve.clear();}

    CurveIterator find(CurvePoint pt) {return m_curve.find(pt);}
    CurveIterator find(KisPoint pt) {return m_curve.find(CurvePoint(pt));}
    CurveIterator find(CurveIterator it, CurvePoint pt) {return m_curve.find(it, pt);}
    CurveIterator find(CurveIterator it, KisPoint pt) {return m_curve.find(it, CurvePoint(pt));}

    CurveIterator getPreviousPivot(KisPoint);
    CurveIterator getPreviousPivot(CurvePoint);
    CurveIterator getPreviousPivot(CurveIterator);

    CurveIterator getNextPivot(KisPoint);
    CurveIterator getNextPivot(CurvePoint);
    CurveIterator getNextPivot(CurveIterator);

    void setPivot (CurvePoint, bool = true);
    void setPivot (CurveIterator, bool = true);

    void deleteLastPivot();

    virtual void deleteCurve(KisPoint, KisPoint);
    virtual void deleteCurve(CurvePoint, CurvePoint);
    virtual void deleteCurve(CurveIterator, CurveIterator);

    virtual void calculateCurve(KisPoint, KisPoint, CurveIterator) {return;}
    virtual void calculateCurve(CurvePoint, CurvePoint, CurveIterator) {return;}
    virtual void calculateCurve(CurveIterator, CurveIterator, CurveIterator) {return;}

    virtual void selectPivot(CurvePoint) {return;}
    virtual void selectPivot(int) {return;}
    
    virtual bool movePivot(CurvePoint, KisPoint);
    virtual bool movePivot(KisPoint, KisPoint);
    virtual bool movePivot(CurveIterator, KisPoint);

    virtual bool deletePivot(CurvePoint);
    virtual bool deletePivot(KisPoint);
    virtual bool deletePivot(CurveIterator);
};

#endif // KIS_CURVES_FRAMEWORK_H_
