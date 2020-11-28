/*
 *  SPDX-FileCopyrightText: 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISDOUBLEPARSESPINBOX_H
#define KISDOUBLEPARSESPINBOX_H

#include <QDoubleSpinBox>

#include "kritawidgetutils_export.h"

class QLabel;

/*!
 * \brief The KisDoubleParseSpinBox class is a cleverer doubleSpinBox, able to parse arithmetic expressions.
 *
 * Use this spinbox instead of the basic one from Qt if you want it to be able to parse arithmetic expressions.
 */
class KRITAWIDGETUTILS_EXPORT KisDoubleParseSpinBox : public QDoubleSpinBox
{

    Q_OBJECT
public:
    KisDoubleParseSpinBox(QWidget* parent = 0);
    ~KisDoubleParseSpinBox() override; //KisDoubleParseSpinBox may be used polymorphycally as a QDoubleSpinBox.

    double valueFromText(const QString & text) const override;
    QString textFromValue(double val) const override;
    QValidator::State validate ( QString & input, int & pos ) const override;

    void stepBy(int steps) override;

    void setValue(double value); //polymorphism won't work directly, we use a signal/slot hack to do so but if signals are disabled this function will still be useful.

    bool isLastValid() const{ return boolLastValid; }

    //! \brief this virtual function is similar to cleanText(); for KisDoubleParseSpinBox. But child class may remove additional artifacts.
    virtual QString veryCleanText() const;

Q_SIGNALS:

    //! \brief signal emitted when the last parsed expression creates an error.
    void errorWhileParsing(QString expr) const;
    //! \brief signal emitted when the last parsed expression is valid.
    void noMoreParsingError() const;

public Q_SLOTS:

    //! \brief useful to let the widget change its stylesheet when an error occurred in the last expression.
    void setErrorStyle();
    //! \brief useful to let the widget reset its stylesheet when there's no more error.
    void clearErrorStyle();
    //! \brief say the widget to return to an error free state.
    void clearError();

protected:

    mutable bool boolLastValid;
    mutable double oldValue;
    mutable QString lastExprParsed;

    QLabel* warningIcon;

    QPalette oldPalette;
    bool isOldPaletteSaved;

    QMargins oldMargins;
    bool areOldMarginsSaved;
};

#endif // KISDOUBLEPARSESPINBOX_H
