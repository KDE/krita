/*
 *  SPDX-FileCopyrightText: 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISINTPARSESPINBOX_H
#define KISINTPARSESPINBOX_H

#include <QSpinBox>

#include "kritawidgetutils_export.h"

class QLabel;

/*!
 * \brief The KisDoubleParseSpinBox class is a cleverer doubleSpinBox, able to parse arithmetic expressions.
 *
 * Use this spinbox instead of the basic one from Qt if you want it to be able to parse arithmetic expressions.
 */
class KRITAWIDGETUTILS_EXPORT KisIntParseSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    KisIntParseSpinBox(QWidget *parent = 0);
    ~KisIntParseSpinBox() override;

    int valueFromText(const QString & text) const override;
    QString textFromValue(int val) const override;
    QValidator::State validate ( QString & input, int & pos ) const override;

    void stepBy(int steps) override;

    void setValue(int val); //polymorphism won't work directly, we use a signal/slot hack to do so but if signals are disabled this function will still be useful.

    bool isLastValid() const{ return boolLastValid; }

Q_SIGNALS:

    //! \brief signal emitted when the last parsed expression create an error.
    void errorWhileParsing(QString expr) const;
    //! \brief signal emitted when the last parsed expression is valid.
    void noMoreParsingError() const;

public Q_SLOTS:

    //! \brief useful to let the widget change it's stylesheet when an error occurred in the last expression.
    void setErrorStyle();
    //! \brief useful to let the widget reset it's stylesheet when there's no more error.
    void clearErrorStyle();
    //! \brief say the widget to return to an error free state.
    void clearError();

protected:

    mutable QString* lastExprParsed;
    mutable bool boolLastValid;
    mutable int oldVal; //store the last correctly evaluated value.

    QLabel* warningIcon;

    QPalette oldPalette;
    bool isOldPaletteSaved;

    QMargins oldMargins;
    bool areOldMarginsSaved;
};

#endif // KISINTPARSESPINBOX_H
