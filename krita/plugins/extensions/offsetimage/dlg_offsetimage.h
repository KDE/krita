/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef DLG_OFFSETIMAGE
#define DLG_OFFSETIMAGE

#include <kdialog.h>
#include <kis_global.h>

#include "ui_wdg_offsetimage.h"


class WdgOffsetImage : public QWidget, public Ui::WdgOffsetImage
{
    Q_OBJECT

public:
    WdgOffsetImage(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class DlgOffsetImage: public KDialog
{

    Q_OBJECT

public:
    DlgOffsetImage(QWidget * parent = 0, const char* name = 0, QSize imageSize = QSize());
    ~DlgOffsetImage();

    int offsetX() const { return m_offsetX;}
    int offsetY() const { return m_offsetY;}


private slots:
    void okClicked();
    void slotOffsetXChanged(int);
    void slotOffsetYChanged(int);
    void slotMiddleOffset();

private:
    WdgOffsetImage * m_page;
    int m_offsetX;
    int m_offsetY;
    bool m_lock;
    QSize m_offsetSize;

};

#endif // DLG_OFFSETIMAGE
