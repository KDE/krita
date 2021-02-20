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

class KRITAUI_EXPORT KisAspectRatioLocker : public QObject
{
    Q_OBJECT
public:
    KisAspectRatioLocker(QObject *parent = 0);
    ~KisAspectRatioLocker() override;

    template <class SpinBoxType>
        void connectSpinBoxes(SpinBoxType *spinOne, SpinBoxType *spinTwo, KoAspectButton *aspectButton);

    void setBlockUpdateSignalOnDrag(bool block);
    void updateAspect();

private Q_SLOTS:
    void slotSpinOneChanged();
    void slotSpinTwoChanged();
    void slotAspectButtonChanged();

Q_SIGNALS:
    void sliderValueChanged();
    void aspectButtonChanged();
    void aspectButtonToggled(bool value);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_ASPECT_RATIO_LOCKER_H */
