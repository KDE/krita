/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __SLIDER_AND_SPIN_BOX_SYNC_H
#define __SLIDER_AND_SPIN_BOX_SYNC_H

#include <QObject>
#include <functional>

class QSpinBox;
class KisDoubleSliderSpinBox;

/**
 * Syncs a slider measured in percentage with a spin box
 * measuring real value getting value from \p parentValueOp.
 *
 * E.g.
 *
 * parentValueOp() --- total system memory in MiB
 * slider --- percentage of the memory we can use
 * spinBox --- amount o fmemory we can use in MiB
 * slotParentValueChanged() --- should be called every time
 *                              total memory changes
 */
class SliderAndSpinBoxSync : public QObject
{
    Q_OBJECT
    using IntFunction = std::function<int()>;

public:
    SliderAndSpinBoxSync(KisDoubleSliderSpinBox *slider,
                         QSpinBox *spinBox,
                         IntFunction parentValueOp);

    ~SliderAndSpinBoxSync() override;

public Q_SLOTS:
    void slotParentValueChanged();

private Q_SLOTS:
    void sliderChanged(qreal value);
    void spinBoxChanged(int value);

private:
    KisDoubleSliderSpinBox *m_slider;
    QSpinBox *m_spinBox;
    IntFunction m_parentValueOp;

    bool m_blockUpdates;
};

#endif /* __SLIDER_AND_SPIN_BOX_SYNC_H */
