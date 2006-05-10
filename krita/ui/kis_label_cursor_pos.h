/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.g
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_LABEL_CURSOR_POS_H_
#define KIS_LABEL_CURSOR_POS_H_

#include <QLabel>

class KisLabelCursorPos : public QLabel {
    Q_OBJECT
    typedef QLabel super;

public:
    KisLabelCursorPos(QWidget *parent, const char *name = 0, Qt::WFlags f = 0);
    virtual ~KisLabelCursorPos();

public slots:
    void updatePos(qint32 xpos, qint32 ypos);
    void enter();
    void leave();

private:
    bool m_doUpdates;
    qint32 m_ypos;
};

#endif // KIS_LABEL_CURSOR_POS_H_

