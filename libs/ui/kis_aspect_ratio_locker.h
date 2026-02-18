/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASPECT_RATIO_LOCKER_H
#define __KIS_ASPECT_RATIO_LOCKER_H

#include <QScopedPointer>
#include <QObject>
#include "kritaui_export.h"

class QSpinBox;
class QDoubleSpinBox;
class KisSliderSpinBox;
class KisDoubleSliderSpinBox;
class KoAspectButton;


/**
 * KisAspectRatioLocker is a class for locking the ratio between two sliders,
 * i.e. to keep width and height of the image proportional.
 *
 * When the locker takes responsibility of two sliders and a button, you
 * should **not** use the valueChanged() and toggled() signals of
 * the sliders and the button themselves. Instead you need to connect to
 * the corresponding signals of the locker.
 *
 * Usage:
 *
 *        \code{.cpp}
 *
 *        KisIntParseSpinBox *intRoundCornersX = ...;
 *        KisIntParseSpinBox *intRoundCornersY = ...;
 *        KoAspectButton *cornersAspectButton = ...;
 *
 *        auto *cornersAspectLocker = new KisAspectRatioLocker(this);
 *        cornersAspectLocker->connectSpinBoxes(intRoundCornersX, intRoundCornersY, cornersAspectButton);
 *
 *        connect(cornersAspectLocker, SIGNAL(sliderValueChanged()), SLOT(slotRoundCornersChanged()));
 *        connect(cornersAspectLocker, SIGNAL(aspectButtonChanged()), SLOT(slotRoundCornersAspectLockChanged()));
 *
 *        \endcode
 */
class KRITAUI_EXPORT KisAspectRatioLocker : public QObject
{
    Q_OBJECT
public:
    KisAspectRatioLocker(QObject *parent = 0);
    ~KisAspectRatioLocker() override;

    /**
     * Link two spin boxes and an aspect button together
     */
    template <class SpinBoxType>
        void connectSpinBoxes(SpinBoxType *spinOne, SpinBoxType *spinTwo, KoAspectButton *aspectButton);

    /**
     * Link two angular spin boxes and an aspect button together
     */
    template <class AngleBoxType>
        void connectAngleBoxes(AngleBoxType *spinOne, AngleBoxType *spinTwo, KoAspectButton *aspectButton);

    /**
     * Selects whether sliderValueChanged() signal should be emitted
     * continuously while the user drags one of the slider or only
     * once when the drag operation is completed
     */
    void setBlockUpdateSignalOnDrag(bool block);

    /**
     * Recalculate and lock the new aspect ratio of the sliders.
     * The function takes the current values of the sliders and
     * calculates the new ratio based on them.
     *
     * This function is expected to be used when a saved value
     * is restored into the sliders. Make sure that you block
     * signals of the controls while loading values.
     *
     * Usage:
     *
     *     \code{.cpp}
     *
     *     // load values in a locked state
     *     KisSignalsBlocker b(intRoundCornersX, intRoundCornersY, cornersAspectButton);
     *     intRoundCornersX->setValue(cfg.readEntry("roundCornersX", 0));
     *     intRoundCornersY->setValue(cfg.readEntry("roundCornersY", 0));
     *     cornersAspectButton->setKeepAspectRatio(cfg.readEntry("roundCornersAspectLocked", true));
     *
     *     // update new aspect ratio
     *     cornersAspectLocker->updateAspect();
     *     \endcode
     */
    void updateAspect();

private Q_SLOTS:
    void slotSpinOneChanged();
    void slotSpinTwoChanged();
    void slotAspectButtonChanged();
    void slotSpinDraggingFinished();

Q_SIGNALS:
    /**
     * Emitted when a value of any of the sliders has changed
     * and all aspect corrections have been completed.
     *
     * You should use this signal instead of the own signals
     * of the sliders, because these signals might be either
     * blocked or come in undefined order.
     */
    void sliderValueChanged();

    /**
     * Emitted when the aspec locking button state is changed.
     */
    void aspectButtonChanged();

    /**
     * Emitted when the aspec locking button state is changed,
     * but also passing its new state.
     */
    void aspectButtonToggled(bool value);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_ASPECT_RATIO_LOCKER_H */
