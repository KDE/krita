/*
 * Kexi Report Plugin
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "krscriptdraw.h"
#include <renderobjects.h>
#include <krpos.h>
#include <krsize.h>
#include <QFont>
#include <QFontMetrics>

KRScriptDraw::KRScriptDraw(QObject *parent)
        : QObject(parent)
{
    m_curPage = 0;
}


KRScriptDraw::~KRScriptDraw()
{
}

void KRScriptDraw::setPage(OROPage *p)
{
    m_curPage = p;
}

void KRScriptDraw::setOffset(QPointF off)
{
    m_curOffset = off;
}

void KRScriptDraw::rectangle(qreal x, qreal y, qreal w, qreal h, const QString& lc, const QString& fc, qreal lw, int a)
{
    if (m_curPage) {
        ORORect *r = new ORORect();
        KRPos p;
        KRSize s;

        p.setPointPos(QPointF(x, y));
        s.setPointSize(QSizeF(w, h));
        r->setRect(QRectF(p.toScene() + m_curOffset, s.toScene()));

        QPen pen(QColor(lc), lw);
        QColor c(fc);
        c.setAlpha(a);
        QBrush bru(c);

        r->setBrush(bru);
        r->setPen(pen);
        m_curPage->addPrimitive(r);
    }
}

void KRScriptDraw::ellipse(qreal x, qreal y, qreal w, qreal h, const QString& lc, const QString& fc, qreal lw, int a)
{
    if (m_curPage) {
        OROEllipse *e = new OROEllipse();
        KRPos p;
        KRSize s;

        p.setPointPos(QPointF(x, y));
        s.setPointSize(QSizeF(w, h));
        e->setRect(QRectF(p.toScene() + m_curOffset, s.toScene()));

        QPen pen(QColor(lc), lw);
        QColor c(fc);
        c.setAlpha(a);
        QBrush bru(c);

        e->setBrush(bru);
        e->setPen(pen);
        m_curPage->addPrimitive(e);
    }
}

void KRScriptDraw::line(qreal x1, qreal y1, qreal x2, qreal y2, const QString& lc)
{
    if (m_curPage) {
        OROLine *ln = new OROLine();
        KRPos s;
        KRPos e;

        s.setPointPos(QPointF(x1, y1));
        e.setPointPos(QPointF(x2, y2));

        ln->setStartPoint(s.toScene() + m_curOffset);
        ln->setEndPoint(e.toScene() + m_curOffset);

        KRLineStyleData ls;
        ls.lineColor = QColor(lc);
        ls.weight = 1;
        if (ls.weight <= 0)
            ls.style = Qt::NoPen;
        else
            ls.style = Qt::SolidLine;

        ln->setLineStyle(ls);
        m_curPage->addPrimitive(ln);
    }
}

void KRScriptDraw::text(qreal x, qreal y, const QString &txt, const QString &fnt, int pt, const QString &fc, const QString&bc, const QString &lc, qreal lw, int o)
{
    if (m_curPage) {
        QFont f(fnt, pt);
        QRectF r = QFontMetrics(f).boundingRect(txt);

        KRTextStyleData ts;
        ts.font = f;
        ts.backgroundColor = QColor(bc);
        ts.foregroundColor = QColor(fc);
        ts.backgroundOpacity = o;

        KRLineStyleData ls;
        ls.lineColor = QColor(lc);
        ls.weight = lw;
        if (lw <= 0)
            ls.style = Qt::NoPen;
        else
            ls.style = Qt::SolidLine;


        OROTextBox *tb = new OROTextBox();
        tb->setPosition(QPointF(x, y) + m_curOffset);
        tb->setSize(r.size());
        tb->setTextStyle(ts);
        tb->setLineStyle(ls);

        tb->setText(txt);

        m_curPage->addPrimitive(tb);

    }
}

