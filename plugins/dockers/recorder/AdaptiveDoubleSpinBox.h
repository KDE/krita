/*
 *  SPDX-FileCopyrightText: 2022 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef ADAPTIVEDOUBLESPINBOX_H
#define ADAPTIVEDOUBLESPINBOX_H

#include <QDoubleSpinBox>

class AdaptiveDoubleSpinBoxPrivate;

/**
 * The AdaptiveDoubleSpinBox class is a QDoubleSpinBox which work in two ranges.
 * When current value is lies within first range from min to switchValue,
 * smallStep is used when you change value using mouse wheel or up/down keys.
 * When current value is above switch value, the largeStep is used.
 */
class AdaptiveDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

    Q_PROPERTY(double smallStep READ smallStep WRITE setSmallStep)
    Q_PROPERTY(double largeStep READ largeStep WRITE setLargeStep)
    Q_PROPERTY(double switchValue READ switchValue WRITE setSwitchValue)

public:
    AdaptiveDoubleSpinBox(QWidget *parent = nullptr);
    ~AdaptiveDoubleSpinBox();

    /**
     * @brief get current small step
     * @return the small step used for range min..<switchValue
     */
    double smallStep() const;

    /**
     * @brief setup smallStep used for range min..<switchValue
     * @param value new small step
     */
    void setSmallStep(double value);

    /**
     * @brief get current large step
     * @return the large step used for range switchValue..max
     */
    double largeStep() const;

    /**
     * @brief setup the large step used for range switchValue..max
     * @param value new large step
     */
    void setLargeStep(double value);

    /**
     * @brief current switch value
     * @return a value where step will be switched when crossing from smallStep to largeStep
     */
    double switchValue() const;

    /**
     * @brief setup switch value
     * @param value new switch value
     */
    void setSwitchValue(double value);

    /**
     * @brief overrode stepBy function used to switch from small step to large step
     * @param steps value change direction
     */
    void stepBy(int steps) override;

private Q_SLOTS:
    void updateSingleStep(double val);

private:
    AdaptiveDoubleSpinBoxPrivate *const d;
    Q_DISABLE_COPY(AdaptiveDoubleSpinBox)
};

#endif // ADAPTIVEDOUBLESPINBOX_H
