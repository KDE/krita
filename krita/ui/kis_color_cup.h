/* This file is part of the KDE project
 *
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_COLOR_CUP_H
#define KIS_COLOR_CUP_H

#include <QPushButton>
#include <QColor>
#include <QFrame>
#include <QStyleOption>

#include <krita_export.h>

class QSize;
class QPainter;
class QWidget;
class KHSSelector;
class KValueSelector;

class KoColorPopup : public QFrame {

    Q_OBJECT

public:

    KoColorPopup(QColor color, QWidget * w, const char * name);
    virtual ~KoColorPopup() {}

signals:

    void changed(const QColor &);

private:

    KHSSelector * m_khsSelector;
    KValueSelector * m_valueSelector;

    QColor m_color;
};

class KRITAUI_EXPORT KoColorCup : public QPushButton {

    Q_OBJECT

public:
    KoColorCup(QWidget * parent, const char * name = 0);
    virtual ~KoColorCup() {}

    QColor color() { return m_color; }

signals:
    void changed(const QColor &);

public:
    QSize sizeHint() const;

public slots:
    void setColor(const QColor & c);

private slots:
    void slotClicked();

protected:
    void paintEvent(QPaintEvent *e);

private:

    KoColorPopup * m_popup;
    QColor m_color;
};

#endif
