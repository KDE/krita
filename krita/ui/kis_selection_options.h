/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef __KIS_SELECTION_OPTIONS_H__
#define __KIS_SELECTION_OPTIONS_H__

#include <qwidget.h>

#include "koffice_export.h"

class KisCanvasSubject;
class WdgSelectionOptions;

/**
 */
class KRITAUI_EXPORT KisSelectionOptions : public QWidget
{

    Q_OBJECT

    typedef QWidget super;

public:
    KisSelectionOptions( QWidget *parent, KisCanvasSubject * subject);
    virtual ~KisSelectionOptions();

    int action();

signals:
    void actionChanged(int);

public slots:
    void slotActivated();

private:
    WdgSelectionOptions * m_page;
    KisCanvasSubject* m_subject;
};

#endif

