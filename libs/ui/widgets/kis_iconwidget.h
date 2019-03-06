/*
 *  Copyright (c) 2000 Matthias Elter  <elter@kde.org>
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_ICONWIDGET_H_
#define KIS_ICONWIDGET_H_

#include <kis_popup_button.h>

class KoResource;

/**
 * The icon widget is used in the control box where the current color and brush
 * are shown.
 */
class KisIconWidget : public KisPopupButton
{

    Q_OBJECT

public:
    KisIconWidget(QWidget *parent = 0, const char *name = 0);
    void setResource(KoResource * resource);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    KoResource *m_resource;
};

#endif // KIS_ICONWIDGET_H_

