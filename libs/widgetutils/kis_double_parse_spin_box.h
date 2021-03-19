/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISDOUBLEPARSESPINBOX_H
#define KISDOUBLEPARSESPINBOX_H

#include <QDoubleSpinBox>
#include <QScopedPointer>

#include <kritawidgetutils_export.h>

template <typename SpinBoxTypeTP, typename BaseSpinBoxTypeTP>
class KisParseSpinBoxPrivate;

/**
 * @brief The KisDoubleParseSpinBox class is a cleverer doubleSpinBox, able to parse arithmetic expressions.
 *
 * Use this spinbox instead of the basic one from Qt if you want it to be able to parse arithmetic expressions.
 */
class KRITAWIDGETUTILS_EXPORT KisDoubleParseSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    KisDoubleParseSpinBox(QWidget *parent = 0);
    ~KisDoubleParseSpinBox() override;

    /**
     * @brief This is a reimplementation of @ref QDoubleSpinBox::stepBy that
     * uses @ref setValue
     * @param steps Number of steps that the value should change
     */
    void stepBy(int steps) override;
    /**
     * @brief Set the value of the spinbox
     * 
     * This reimplementation also tries to clear the current expression and
     * warning message whenever possible. This will happen when the new value
     * is different from the current one and the line edit has not the focus
     * or it is read-only. One can force the reset also by passing true to the
     * @p overwriteExpression parameter.
     * 
     * @param value The new value
     * @param overwriteExpression Get if the expression in the edit field
     * (and the warning message) should be reset to reflect the new value.
     * The default is false so that if the user is editing the expression
     * it won't be disrupted by any default call to this function
     */
    void setValue(double value, bool overwriteExpression = false);
    /**
     * @brief Get if the last expression entered is a valid one
     * @retval true if the last expression entered is valid
     * @retval false otherwise
     */
    bool isLastValid() const;
    /**
     * @brief This virtual function is similar to cleanText(). But child classes
     * may reimplement it to further process ("clean up") the expression.
     * @return The processed expression
     */
    virtual QString veryCleanText() const;

Q_SIGNALS:
    /**
     * @brief signal emitted when the last parsed expression is not valid.
     */
    void errorWhileParsing(const QString &expr) const;
    /**
     * @brief signal emitted when the last parsed expression is valid and
     * the expression before was not valid.
     */
    void noMoreParsingError() const;

protected:
    QValidator::State validate(QString &input, int &pos) const override;
    double valueFromText(const QString &text) const override;
    QString textFromValue(double value) const override;

private:
    template <typename SpinBoxTypeTP, typename BaseSpinBoxTypeTP>
    friend class KisParseSpinBoxPrivate;
    QScopedPointer<KisParseSpinBoxPrivate<KisDoubleParseSpinBox, QDoubleSpinBox>> d;
};

#endif // KISDOUBLEPARSESPINBOX_H
