/*
 *  SPDX-FileCopyrightText: 2022 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef ADAPTIVEDOUBLESPINBOX_H
#define ADAPTIVEDOUBLESPINBOX_H

#include <QDoubleSpinBox>

class AdaptiveDoubleSpinBoxPrivate;
class AdaptiveDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

    Q_PROPERTY(double smallStep READ smallStep WRITE setSmallStep)
    Q_PROPERTY(double largeStep READ largeStep WRITE setLargeStep)
    Q_PROPERTY(double switchValue READ switchValue WRITE setSwitchValue)

public:
    AdaptiveDoubleSpinBox(QWidget *parent = nullptr);
    ~AdaptiveDoubleSpinBox();

    double smallStep() const;
    void setSmallStep(double value);

    double largeStep() const;
    void setLargeStep(double value);

    double switchValue() const;
    void setSwitchValue(double value);

    void stepBy(int steps) override;

private Q_SLOTS:
    void updateSingleStep(double val);

private:
    AdaptiveDoubleSpinBoxPrivate *const d;
    Q_DISABLE_COPY(AdaptiveDoubleSpinBox)
};

#endif // ADAPTIVEDOUBLESPINBOX_H
