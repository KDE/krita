/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 C. Boemann <cbo@boemann.dk>
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

#include <KoIcon.h>
#include <klocalizedstring.h>
#include <QDebug>

#include <QMenu>
#include <QFrame>
#include <QGridLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QWidgetAction>

//This class is the main place where the expanding grid is done
class SizeChooserGrid : public QFrame
{
public:
    SizeChooserGrid(QuickTableButton *button, QAction *action);
    virtual QSize sizeHint() const;
    virtual void mouseMoveEvent(QMouseEvent *ev);
    virtual void enterEvent(QEvent *ev);
    virtual void leaveEvent(QEvent *ev);
    virtual void mouseReleaseEvent(QMouseEvent *ev);
    virtual void paintEvent(QPaintEvent *event);
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
    QAction *m_action;
};

SizeChooserGrid::SizeChooserGrid(QuickTableButton *button, QAction *action)
    : QFrame()
    , m_column(0)
    , m_row(0)
    , m_columnWidth(30)
    , m_button(button)
    , m_action(action)
{
    setFrameShadow(Sunken);
    setBackgroundRole(QPalette::Base);
    setFrameShape(StyledPanel);
    setMouseTracking(true);

    QFontMetrics metrics(font());
    m_rowHeight = metrics.height() + 2;
    m_columnWidth = metrics.width("8x22") + 2;

    getContentsMargins(&m_leftMargin, &m_topMargin, &m_extraWidth, &m_extraHeight);
    m_leftMargin += 4;
    m_topMargin += 4;
    m_extraWidth += m_leftMargin + 4 + 1;
    m_extraHeight += m_topMargin + 4 + 1;
}

QSize SizeChooserGrid::sizeHint() const
{
    return QSize(m_extraWidth + 8 * m_columnWidth, m_extraHeight + 8 * m_rowHeight);
}

void SizeChooserGrid::mouseMoveEvent(QMouseEvent *ev)
{
    m_column = qMin(qreal(7.0), (ev->x() - m_leftMargin) / m_columnWidth);
    m_row = qMin(qreal(7.0), (ev->y() - m_topMargin) / m_rowHeight);
    repaint();
}

void SizeChooserGrid::enterEvent(QEvent *event)
{
    m_action->activate(QAction::Hover);
    QFrame::enterEvent(event);
}

void SizeChooserGrid::leaveEvent(QEvent *)
{
    m_column = -1;
    m_row = -1;
    repaint();
}

void SizeChooserGrid::mouseReleaseEvent(QMouseEvent *ev)
{
    if (contentsRect().contains(ev->pos())) {
        m_button->emitCreate(m_row + 1, m_column + 1);
    }
    QFrame::mouseReleaseEvent(ev);
}

void SizeChooserGrid::paintEvent(QPaintEvent *event)
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
    painter.fillRect(QRectF(0.0, 0.0, (m_column + 1)  * m_columnWidth, (m_row + 1)  * m_rowHeight), palette().brush(QPalette::Highlight));
    for (int c = 0; c <= 8; c++) {
        painter.drawLine(QPointF(c * m_columnWidth, 0.0), QPointF(c * m_columnWidth, 8 * m_rowHeight));
    }
    for (int r = 0; r <= 8; r++) {
        painter.drawLine(QPointF(0.0, r * m_rowHeight), QPointF(8  * m_columnWidth, r * m_rowHeight));
    }
    QTextOption option(Qt::AlignCenter);
    option.setUseDesignMetrics(true);
    painter.drawText(QRectF(0.0, 0.0, m_columnWidth, m_rowHeight), QString("%1x%2").arg(m_column + 1).arg(m_row + 1), option);
    painter.end();
}

//This class is the main place where the expanding grid is done
class SizeChooserAction : public QWidgetAction
{
public:
    SizeChooserAction(QuickTableButton *button);
    SizeChooserGrid *m_widget;
};

SizeChooserAction::SizeChooserAction(QuickTableButton *button)
    : QWidgetAction(0)
{
    m_widget = new SizeChooserGrid(button, this);
    setDefaultWidget(m_widget);
}

//And now for the button itself
QuickTableButton::QuickTableButton(QWidget *parent)
    : QToolButton(parent)
{
    setToolTip(i18n("Insert a table"));
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setIcon(koIcon("insert-table"));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_menu = new QMenu(this);
    setMenu(m_menu);
    setPopupMode(InstantPopup);
}

void QuickTableButton::addAction(QAction *action)
{
    m_menu->addAction(action);
    m_menu->addAction(new SizeChooserAction(this));
}

void QuickTableButton::emitCreate(int rows, int columns)
{
    m_menu->hide();
    emit create(rows, columns);
}
