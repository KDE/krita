/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
