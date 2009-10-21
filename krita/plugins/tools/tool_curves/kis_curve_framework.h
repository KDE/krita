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

#ifndef KIS_CURVE_FRAMEWORK_H_
#define KIS_CURVE_FRAMEWORK_H_

#include <QList>
#include <QPointF>
#include <QList>
#include <algorithm>

const int NOHINTS = 0x0000;
const int POINTHINT = 0x0001;
const int LINEHINT = 0x0002;

const int NOOPTIONS = 0x0000;
const int SHIFTOPTION = 0x0001;
const int CONTROLOPTION = 0x0002;
const int ALTOPTION = 0x0004;

const int KEEPSELECTEDOPTION = CONTROLOPTION;

class CurvePoint
{

    QPointF m_point;
    bool m_pivot;
    bool m_selected; // Only pivots can be selected

    int m_hint;

public:

    /* Constructors and Destructor */

    CurvePoint();
    CurvePoint(const QPointF&, bool = false, bool = false, int = POINTHINT);
    CurvePoint(double, double, bool = false, bool = false, int = POINTHINT);

    ~CurvePoint() {}

public:

    /* Generic Functions */

    bool operator!= (QPointF p2) const {
        if (p2 != m_point) return true; else return false;
    }
    bool operator!= (CurvePoint p2) const {
        if (p2.point() != m_point ||
                p2.isPivot() != m_pivot ||
                p2.hint() != m_hint) return true; else return false;
    }

    bool operator== (QPointF p2) const {
        if (p2 == m_point) return true; else return false;
    }
    bool operator== (CurvePoint p2) const {
        if (p2.point() == m_point &&
                p2.isPivot() == m_pivot &&
                p2.hint() == m_hint) return true; else return false;
    }

    QPointF point() const {
        return m_point;
    }

    void setPoint(const QPointF&);
    void setPoint(double, double);

    bool isPivot() const {
        return m_pivot;
    }
    bool isSelected() const {
        return m_selected;
    }
    int hint() const {
        return m_hint;
    }

    void setPivot(bool p) {
        m_pivot = p;
    }
    void setSelected(bool s) {
        m_selected = ((m_pivot) ? s : false);
    }  /* Only pivots can be selected */
    void setHint(int h) {
        m_hint = h;
    }
};

typedef QList<CurvePoint> PointList;
typedef PointList::iterator BaseIterator;

class CurveIterator;

class KisCurve
{

public:

    KisCurve() {
        m_actionOptions = NOOPTIONS; m_standardkeepselected = true;
    }
    virtual ~KisCurve() {
        m_curve.clear();
    }

    friend class CurveIterator;
    typedef CurveIterator iterator;

protected:
    /* I need it to be mutable because my iterator needs to access
       m_curve's end() and begin() functions using a const KisCurve
       (see below in CurveIterator) */
    mutable PointList m_curve;
    int m_actionOptions;

    bool m_standardkeepselected;

    bool checkIterator(iterator checking) const;

public:

    void setActionOptions(int options) {
        m_actionOptions = options;
    }
    void endActionOptions() {
        m_actionOptions = NOOPTIONS;
    }

    CurvePoint& operator[](int i) {
        return m_curve[i];
    }

    iterator addPoint(iterator, const CurvePoint&);
    iterator addPoint(iterator, const QPointF&, bool = false, bool = false, int = POINTHINT);

    iterator pushPoint(const CurvePoint&);
    iterator pushPoint(const QPointF&, bool = false, bool = false, int = POINTHINT);

    virtual iterator addPivot(iterator, const QPointF&);
    virtual iterator pushPivot(const QPointF&);

    int count() const {
        return m_curve.size();
    }
    bool isEmpty() const {
        return m_curve.empty();
    }
    CurvePoint first() {
        return m_curve.front();
    }
    CurvePoint last() {
        return m_curve.back();
    }
    void clear() {
        m_curve.clear();
    }

    /* These needs iterators so they are implemented inline after the definition of CurveIterator */
    iterator begin() const;
    iterator lastIterator() const;
    iterator end() const;
    iterator find(const CurvePoint& pt);
    iterator find(const QPointF& pt);
    iterator find(iterator it, const CurvePoint& pt);
    iterator find(iterator it, const QPointF& pt);

    KisCurve pivots();
    KisCurve selectedPivots(bool = true);
    KisCurve subCurve(const QPointF&);
    KisCurve subCurve(const CurvePoint&);
    KisCurve subCurve(iterator);
    KisCurve subCurve(const QPointF&, const QPointF&);
    KisCurve subCurve(const CurvePoint&, const CurvePoint&);
    KisCurve subCurve(iterator, iterator);

    /* Core virtual functions */
    virtual void deleteFirstPivot();
    virtual void deleteLastPivot();

    virtual iterator deleteCurve(const QPointF&, const QPointF&);
    virtual iterator deleteCurve(const CurvePoint&, const CurvePoint&);
    virtual iterator deleteCurve(iterator, iterator);

    /* Core of the Core, calculateCurve is the only function that *needs* an implementation in the derived curves */
    virtual void calculateCurve(const QPointF&, const QPointF&, iterator);
    virtual void calculateCurve(const CurvePoint&, const CurvePoint&, iterator);
    virtual void calculateCurve(iterator, iterator, iterator);
    virtual void calculateCurve(iterator*);
    virtual void calculateCurve();

    virtual iterator selectPivot(const CurvePoint&, bool = true);
    virtual iterator selectPivot(const QPointF&, bool = true);
    virtual iterator selectPivot(iterator, bool = true);

    virtual iterator movePivot(const CurvePoint&, const QPointF&);
    virtual iterator movePivot(const QPointF&, const QPointF&);
    virtual iterator movePivot(iterator, const QPointF&);

    virtual void deletePivot(const CurvePoint&);
    virtual void deletePivot(const QPointF&);
    virtual void deletePivot(iterator);

    virtual void moveSelected(const QPointF&);
    virtual void deleteSelected();
    virtual void selectAll(bool = true);
};

class CurveIterator
{

    const KisCurve *m_target;

    BaseIterator m_position;

public:

    CurveIterator() {
        m_target = 0;
    }

    CurveIterator(const KisCurve &target) {
        m_target = &target;
    }

    CurveIterator(const CurveIterator &it) {
        m_position = it.position(); m_target = it.target();
    }

    CurveIterator(const KisCurve &target, BaseIterator it) {
        m_position = it; m_target = &target;
    }

    ~CurveIterator() {}

    bool operator==(BaseIterator it) {
        return m_position == it;
    }
    bool operator==(CurveIterator it) {
        return m_position == it.position();
    }
    bool operator!=(BaseIterator it) {
        return m_position != it;
    }
    bool operator!=(CurveIterator it) {
        return m_position != it.position();
    }

    CurveIterator operator++() {
        ++m_position; return *this;
    }
    CurveIterator operator++(int) {
        CurveIterator temp = *this; m_position++; return temp;
    }
    CurveIterator operator--() {
        --m_position; return *this;
    }
    CurveIterator operator--(int) {
        CurveIterator temp = *this; m_position--; return temp;
    }
    CurveIterator operator+=(int i) {
        m_position += i; return *this;
    }
    CurveIterator operator-=(int i) {
        m_position -= i; return *this;
    }
    CurveIterator operator=(const BaseIterator &it) {
        m_position = it; return *this;
    }
    CurvePoint& operator*() {
        return (*m_position);
    }

    const KisCurve* target() const {
        return m_target;
    }
    BaseIterator position() const {
        return m_position;
    }

    CurveIterator next() {
        CurveIterator it = *this;
        return ++it;
    }

    CurveIterator previous() {
        CurveIterator it = *this;
        return --it;
    }

    CurveIterator previousPivot() {
        CurveIterator it = *this;
        while (it != m_target->begin()) {
            it -= 1;
            if ((*it).isPivot())
                return it;
        }

        return it;
    }

    CurveIterator nextPivot() {
        BaseIterator it = m_position;
        while (it != m_target->m_curve.end()) {
            it += 1;
            if (it == m_target->m_curve.end())
                return CurveIterator(*m_target, it);
            if ((*it).isPivot())
                return CurveIterator(*m_target, it);
        }
        return CurveIterator(*m_target, it);
    }
};

/* ************************************* *
 * CurvePoint inline methods definitions *
 * ************************************* */

inline CurvePoint::CurvePoint()
        : m_pivot(0), m_selected(0), m_hint(POINTHINT)
{

}

inline CurvePoint::CurvePoint(const QPointF& pt, bool p, bool s, int h)
        : m_pivot(p), m_selected((p) ? s : false), m_hint(h)
{
    m_point = pt;
}

inline CurvePoint::CurvePoint(double x, double y, bool p, bool s, int h)
        : m_pivot(p), m_selected((p) ? s : false), m_hint(h)
{
    QPointF tmp(x, y);
    m_point = tmp;
}

inline void CurvePoint::setPoint(const QPointF& p)
{
    m_point = p;
}

inline void CurvePoint::setPoint(double x, double y)
{
    QPointF tmp(x, y);
    m_point = tmp;
}


/* *********************************** *
 * KisCurve inline methods definitions *
 * *********************************** */

inline bool KisCurve::checkIterator(KisCurve::iterator checking) const
{
    if (checking.target() != this)
        return false;
    else
        return true;
}

inline KisCurve::iterator KisCurve::begin() const
{
    return iterator(*this, m_curve.begin());
}

inline KisCurve::iterator KisCurve::lastIterator() const
{
    return (iterator(*this, --m_curve.end()));
}

inline KisCurve::iterator KisCurve::end() const
{
    return iterator(*this, m_curve.end());
}

inline KisCurve::iterator KisCurve::find(const CurvePoint& pt)
{
    return iterator(*this, m_curve.find(pt));
}

inline KisCurve::iterator KisCurve::find(const QPointF& pt)
{
    return iterator(*this, m_curve.find(CurvePoint(pt)));
}

inline KisCurve::iterator KisCurve::find(KisCurve::iterator it, const CurvePoint& pt)
{
    return iterator(*this, m_curve.find(it.position(), pt));
}

inline KisCurve::iterator KisCurve::find(iterator it, const QPointF& pt)
{
    return iterator(*this, m_curve.find(it.position(), CurvePoint(pt)));
}

inline void KisCurve::calculateCurve(const QPointF& start, const QPointF& end, KisCurve::iterator it)
{
    calculateCurve(find(CurvePoint(start)), find(CurvePoint(end)), it);
}

inline void KisCurve::calculateCurve(const CurvePoint& start, const CurvePoint& end, KisCurve::iterator it)
{
    calculateCurve(find(start), find(end), it);
}

inline void KisCurve::calculateCurve(KisCurve::iterator, KisCurve::iterator, KisCurve::iterator)
{
    return;
}

/* Really generic functions, provided if someone _really_ needs them: array of iterators and no iterators. */
inline void KisCurve::calculateCurve(KisCurve::iterator*)
{
    return;
}
inline void KisCurve::calculateCurve()
{
    return;
}

#endif // KIS_CURVE_FRAMEWORK_H_
