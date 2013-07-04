/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *  Copyright (C) 2005 Thomas Zander <zander@kde.org>
 *  Copyright (C) 2005 C. Boemann <cbo@boemann.dk>
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

#ifndef KIS_ANIMATION_SELECTOR_H
#define KIS_ANIMATION_SELECTOR_H

#include <QWidget>
#include "kis_global.h"
#include "KoUnit.h"
#include "kis_properties_configuration.h"
#include "ui_wdganimationselector.h"

class KisAnimationDoc;
class KoID;

class WdgAnimationSelector : public QWidget, public Ui::WdgAnimationSelector{
    Q_OBJECT
public:
    WdgAnimationSelector(QWidget* parent) : QWidget(parent){
        setupUi(this);
    }
};

class KisAnimationSelector : public WdgAnimationSelector
{
    Q_OBJECT
public:
    KisAnimationSelector(QWidget* parent, KisAnimationDoc *document, qint32 defWidth, qint32 defHeight, double resolution, const QString & defColorModel, const QString &defColorDepth, const QString &defColorProfile, const QString&animationName);
    virtual ~KisAnimationSelector();

signals:
    void documentSelected();

private slots:
    void createAnimation();
    void resolutionChanged(double value);
    void widthUnitChanged(int index);
    void heightUnitChanged(int index);
    void widthChanged(double value);
    void heightChanged(double value);

private:
    quint8 backgroundOpacity();
    KisAnimationDoc *m_document;
    double m_width, m_height;
    KoUnit m_widthUnit, m_heightUnit;
    QList<KisPropertiesConfiguration*> m_predefined;
};

#endif // KIS_ANIMATION_SELECTOR_H
