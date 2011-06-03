/*
 *  Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
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

#ifndef KIS_SMALL_PRESET_HANDLER_H
#define KIS_SMALL_PRESET_HANDLER_H

#include "ui_wdgsmallpresethandler.h"

#include "KoResourceItemView.h"

#include <QWidget>


class WdgSmallPresetHandler : public QWidget, public Ui::WdgSmallPresetHandler
{
    Q_OBJECT
    
    public:
    WdgSmallPresetHandler(QWidget *parent);
    //QPoint scrollCoordinate(qint8 dx);
    
    public slots:
    void currentPaintopChanged(QString printme);
    
    private slots:
    void on_leftScrollBtn_pressed();
    void on_rightScrollBtn_pressed();
    
    private:
    KoResourceItemView* antiOOPHack;
    //qint16 m_coorX;
};


#endif // KIS_SMALL_PRESET_HANDLER_H
