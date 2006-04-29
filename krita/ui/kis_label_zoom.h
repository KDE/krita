/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
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
#ifndef KIS_LABEL_ZOOM_H_
#define KIS_LABEL_ZOOM_H_

#include <qlabel.h>

class KisLabelZoom : public QLabel {
    Q_OBJECT

    KisLabelZoom( QWidget *parent, const char *name = 0) :
        QLabel( parent ) { setObjectName(name); }
    virtual ~KisLabelZoom() {}

};

#endif // KIS_LABEL_ZOOM_H_

