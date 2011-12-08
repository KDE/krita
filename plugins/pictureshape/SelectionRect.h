/* This file is part of the KDE project
   Copyright 2011 Silvio Heinrich <plassy@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef H_SELECTION_RECT_H
#define H_SELECTION_RECT_H

#include <QRectF>
#include <QPointF>
#include <QSizeF>
#include <limits>

class SelectionRect
{
public:
    typedef int HandleFlags;
    
    enum PositionInfo
    {
        INSIDE_RECT   = 0x01,
        TOP_HANDLE    = 0x02,
        BOTTOM_HANDLE = 0x04,
        LEFT_HANDLE   = 0x08,
        RIGHT_HANDLE  = 0x10
    };
    
    SelectionRect(const QRectF& rect=QRectF(), qreal handleSize=10.0):
        m_rect(rect),
        m_minSize(0,0),
        m_handleSize(handleSize),
        m_currentHandle(0)
    {
        m_lConstr = -std::numeric_limits<qreal>::infinity();
        m_rConstr =  std::numeric_limits<qreal>::infinity();
        m_tConstr = -std::numeric_limits<qreal>::infinity();
        m_bConstr =  std::numeric_limits<qreal>::infinity();
    }

    void setRect(const QRectF& rect)
    {
        m_rect = rect;
    }

    void setHandleSize(qreal size)
    {
        m_handleSize = size;
    }

    void setConstrainingRect(const QRectF& rect)
    {
        m_lConstr = rect.left();
        m_rConstr = rect.right();
        m_tConstr = rect.top();
        m_bConstr = rect.bottom();
    }

    bool beginDragging(const QPointF& pos)
    {
        m_tempPos       = pos;
        m_currentHandle = getHandleFlags(pos);
        return bool(m_currentHandle);
    }
    
    void doDragging(const QPointF& pos)
    {
        if (m_currentHandle & INSIDE_RECT) {
            m_rect.moveTo(m_rect.topLeft() + (pos - m_tempPos));
            m_tempPos = pos;
            
            if (m_rect.left() < m_lConstr) {
                m_rect.moveLeft(m_lConstr);
            }
            if (m_rect.right() > m_rConstr) {
                m_rect.moveRight(m_rConstr);
            }
            if (m_rect.top() < m_tConstr) {
                m_rect.moveTop(m_tConstr);
            }
            if (m_rect.bottom() > m_bConstr) {
                m_rect.moveBottom(m_bConstr);
            }
        }
        else {
            if (m_currentHandle & TOP_HANDLE) {
                m_rect.setTop(qBound(m_tConstr, pos.y(), m_bConstr));
            }
            if (m_currentHandle & BOTTOM_HANDLE) {
                m_rect.setBottom(qBound(m_tConstr, pos.y(), m_bConstr));
            }
            if (m_currentHandle & LEFT_HANDLE) {
                m_rect.setLeft(qBound(m_lConstr, pos.x(), m_rConstr));
            }
            if (m_currentHandle & RIGHT_HANDLE) {
                m_rect.setRight(qBound(m_lConstr, pos.x(), m_rConstr));
            }
        }
    }

    void finishDragging()
    {
        m_currentHandle = 0;
        m_rect          = m_rect.normalized();
    }

    int    getNumHandles() const { return 8;      }
    QRectF getRect      () const { return m_rect; }

    HandleFlags getHandleFlags(const QPointF& pos) const
    {
        for(int i=0; i<getNumHandles(); ++i) {
            if(getHandleRect(getHandleFlags(i)).contains(pos))
                return getHandleFlags(i);
        }

        return m_rect.contains(pos) ? INSIDE_RECT : 0;
    }

    HandleFlags getHandleFlags(int handleIndex) const
    {
        switch(handleIndex)
        {
            case 0: return TOP_HANDLE|LEFT_HANDLE;
            case 1: return TOP_HANDLE;
            case 2: return TOP_HANDLE|RIGHT_HANDLE;
            case 3: return RIGHT_HANDLE;
            case 4: return BOTTOM_HANDLE|RIGHT_HANDLE;
            case 5: return BOTTOM_HANDLE;
            case 6: return BOTTOM_HANDLE|LEFT_HANDLE;
            case 7: return LEFT_HANDLE;
        }
        
        return 0;
    }
    
    QRectF getHandleRect(HandleFlags handle) const
    {
        qreal x = (m_rect.left() + m_rect.right()) / 2.0;
        qreal y = (m_rect.top()  + m_rect.bottom()) / 2.0;
        qreal h = m_handleSize / 2.0;
        
        x = (handle & LEFT_HANDLE  ) ? m_rect.left()   : x;
        y = (handle & TOP_HANDLE   ) ? m_rect.top()    : y;
        x = (handle & RIGHT_HANDLE ) ? m_rect.right()  : x;
        y = (handle & BOTTOM_HANDLE) ? m_rect.bottom() : y;

        return QRectF(x-h, y-h, m_handleSize, m_handleSize);
    }
    
private:
    QPointF     m_tempPos;
    QRectF      m_rect;
    qreal       m_lConstr;
    qreal       m_rConstr;
    qreal       m_tConstr;
    qreal       m_bConstr;
    QSizeF      m_minSize;
    qreal       m_handleSize;
    HandleFlags m_currentHandle;
};

#endif // H_SELECTION_RECT_H
