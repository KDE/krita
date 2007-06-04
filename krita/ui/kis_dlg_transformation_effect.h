/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_DLG_TRANSFORMATION_EFFECT_H_
#define KIS_DLG_TRANSFORMATION_EFFECT_H_

#include <kdialog.h>

class QString;
class QWidget;
class KisFilterStrategy;

#include "ui_wdgtransformationeffect.h"

class WdgTransformationEffect : public QWidget, public Ui::WdgTransformationEffect
{
    Q_OBJECT

    public:
        WdgTransformationEffect(QWidget *parent)
            : QWidget(parent)
        {
            setupUi(this);
        }
};

class KisDlgTransformationEffect : public KDialog {

    Q_OBJECT

public:

    KisDlgTransformationEffect(const QString & maskName,
                               double xScale,
                               double yScale,
                               double xShear,
                               double yShear,
                               double angle,
                               int moveX,
                               int moveY,
                               const KoID & filterId,
                               QWidget *parent = 0,
                               const char *name = 0);

    QString maskName() const;
    double xScale() const;
    double yScale() const;
    double xShear() const;
    double yShear() const;
    double rotation() const;
    int moveX() const;
    int moveY() const;
    KisFilterStrategy * filterStrategy();

private:
    WdgTransformationEffect * m_page;
};

#endif // KIS_DLG_TRANSFORMATION_EFFECT_H_

