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
#include <KoID.h>

class QString;
class QGridLayout;
class KisFilterStrategy;

class WdgTransformationEffect;
class WdgMaskSource;
class WdgMaskFromSelection;

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

    WdgTransformationEffect * transformationEffect()
        {
            return m_transformEffectWidget;
        }

    WdgMaskSource * maskSource()
        {
            return m_maskSource;
        }

    WdgMaskFromSelection * maskFromSelection()
        {
            return m_maskFromSelection;
        }


private:
    WdgTransformationEffect * m_transformEffectWidget;
    WdgMaskSource * m_maskSource;
    WdgMaskFromSelection * m_maskFromSelection;
    QGridLayout * m_grid;
};

#endif // KIS_DLG_TRANSFORMATION_EFFECT_H_
