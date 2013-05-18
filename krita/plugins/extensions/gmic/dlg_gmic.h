/*
 *  dlg_gmic.h -- part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef DLG_GMIC
#define DLG_GMIC

#include <kdialog.h>

#include "kis_gmic.h"

#include "ui_wdg_gmic.h"

class QColor;

class WdgGmic : public QWidget, public Ui::WdgGmic
{
    Q_OBJECT

public:
    WdgGmic(QWidget *parent, const char *name) : QWidget(parent) {
        setObjectName(name); setupUi(this);
    }
};

class DlgGmic: public KDialog
{

    Q_OBJECT

public:

    DlgGmic(const QString & imageCS, const QString & layerCS, QWidget * parent = 0,
                  const char* name = 0);
    ~DlgGmic();

    QString gmicScript();

private slots:
    void okClicked();

private:
    WdgGmic * m_page;
};

#endif // DLG_GMIC
