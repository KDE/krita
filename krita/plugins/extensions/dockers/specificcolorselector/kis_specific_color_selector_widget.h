/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_SPECIFIC_COLOR_SELECTOR_WIDGET_H_
#define _KIS_SPECIFIC_COLOR_SELECTOR_WIDGET_H_

#include <QWidget>

#include <KoColor.h>

class KoColorSpace;
class QVBoxLayout;
class KisColorInput;

class KisSpecificColorSelectorWidget : public QWidget
{
    Q_OBJECT
public:
    KisSpecificColorSelectorWidget(QWidget* parent);
    ~KisSpecificColorSelectorWidget();
public slots:
    void setColorSpace(const KoColorSpace*);
    void setColor(const KoColor&);
private slots:
    void update();
signals:
    void colorChanged(const KoColor&);
    void updated();
private:
    QList<KisColorInput*> m_inputs;
    const KoColorSpace* m_colorSpace;
    QVBoxLayout *m_layout;
    KoColor m_color;
    bool m_updateAllowed;
};

#endif
