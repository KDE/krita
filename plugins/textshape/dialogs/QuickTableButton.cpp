/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Casper Boemann <cbo@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "QuickTableButton.h"

#include <klocale.h>
#include <kicon.h>
#include <kdebug.h>

#include <QMenu>
#include <QFrame>
#include <QGridLayout>
#include <QPainter>
#include <QMouseEvent>

//This class is the main place where the expanding grid is done
class QuickTableGrid : public QFrame
{
    public:
        QuickTableGrid(QuickTableButton *button, QWidget * parent = 0);
        virtual QSize sizeHint() const;
        virtual void mouseMoveEvent (QMouseEvent *ev);
        virtual void leaveEvent(QEvent *ev);
        virtual void mouseReleaseEvent (QMouseEvent *ev);
        virtual void paintEvent(QPaintEvent * event);
    private:
        int m_column;
        int m_row;
        qreal m_columnWidth;
        qreal m_rowHeight;
        int m_leftMargin;
        int m_topMargin;
        int m_extraWidth;
        int m_extraHeight;
        QuickTableButton *m_button;
};

QuickTableGrid::QuickTableGrid(QuickTableButton *button, QWidget * parent)
 : QFrame(parent)
 ,m_column(0)
 ,m_row(0)
 ,m_columnWidth(30)
 ,m_button(button)
{
    setFrameShadow(Sunken);
    setBackgroundRole(QPalette::Base);
    setFrameShape(StyledPanel);
    setMouseTracking(true);

    QFontMetrics metrics(font());
    m_rowHeight = metrics.height() + 2;
    m_columnWidth = metrics.width("22x22") + 2;

    getContentsMargins(&m_leftMargin, &m_topMargin, &m_extraWidth, &m_extraHeight);
    m_leftMargin += 4;
    m_topMargin += 4;
    m_extraWidth += m_leftMargin+4+1;
    m_extraHeight += m_topMargin+4+1;
}

QSize QuickTableGrid::sizeHint() const
{
    return QSize(m_extraWidth+qMax(4, m_column+2)  * m_columnWidth, m_extraHeight+qMax(4, m_row+2)  * m_rowHeight);
}

void QuickTableGrid::mouseMoveEvent(QMouseEvent *ev)
{
    m_column = (ev->x()-m_leftMargin) / m_columnWidth;
    m_row = (ev->y()-m_topMargin) / m_rowHeight;
    updateGeometry();
    repaint();
}

void QuickTableGrid::leaveEvent(QEvent *ev)
{
    m_column = -1;
    m_row = -1;
    updateGeometry();
    repaint();
}

void QuickTableGrid::mouseReleaseEvent(QMouseEvent *ev)
{
    if (contentsRect().contains(ev->pos())) {
        m_button->emitCreate(m_row+1, m_column+1);
    }
    QFrame::mouseReleaseEvent(ev);
}

void QuickTableGrid::paintEvent(QPaintEvent * event)
{
    QFrame::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(contentsRect(), palette().brush(QPalette::Base));
    painter.translate(m_leftMargin, m_topMargin);
    painter.translate(0.5, 0.5);
    QPen pen = painter.pen();
    pen.setWidthF(0.5);
    painter.setPen(pen);
    painter.fillRect(QRectF(0.0, 0.0, (m_column+1)  * m_columnWidth, (m_row+1)  * m_rowHeight), palette().brush(QPalette::Highlight));
    for(int c=0; c <=qMax(4, m_column+2); c++) {
        painter.drawLine(QPointF(c*m_columnWidth, 0.0), QPointF(c*m_columnWidth, qMax(4, m_row+2)  * m_rowHeight));
    }
    for(int r=0; r <=qMax(4, m_row+2); r++) {
        painter.drawLine(QPointF(0.0, r*m_rowHeight), QPointF(qMax(4, m_column+2)  * m_columnWidth, r*m_rowHeight));
    }
    QTextOption option(Qt::AlignCenter);
    option.setUseDesignMetrics(true);
    painter.drawText(QRectF(0.0, 0.0, m_columnWidth, m_rowHeight), QString("%1x%2").arg(m_column+1).arg(m_row+1), option);
    painter.end();
}


//This class is needed so that the menu returns a sizehint based on the layout and not on the number (0) of menu items
class QuickTableMenu : public QMenu
{
    public:
        QuickTableMenu(QuickTableButton *button, QWidget * parent = 0);
        virtual QSize sizeHint() const;
};

QuickTableMenu::QuickTableMenu(QuickTableButton *button, QWidget * parent)
 : QMenu(parent)
{
    QGridLayout *containerLayout = new QGridLayout(this);
    containerLayout->setMargin(4);
    containerLayout->addWidget(new QuickTableGrid(button, this), 0, 0);
    containerLayout->setSizeConstraint(QLayout::SetFixedSize);
}

QSize QuickTableMenu::sizeHint() const
{
    return layout()->sizeHint();
}

//And now for the button itself
QuickTableButton::QuickTableButton(QWidget *parent)
    : QToolButton(parent)
{
    setToolTip(i18n("Insert a table"));
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setIcon(KIcon("insert-table"));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_menu = new QuickTableMenu(this);
    setMenu(m_menu);
    setPopupMode(InstantPopup);
}

void QuickTableButton::emitCreate(int rows, int columns)
{
    m_menu->hide();
    emit create(rows, columns);
}


#include <QuickTableButton.moc>
