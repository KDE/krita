/*
 *  Copyright (c) 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
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

#ifndef KISDOUBLEPARSESPINBOX_H
#define KISDOUBLEPARSESPINBOX_H

#include <QDoubleSpinBox>

#include "kritawidgets_export.h"

class QLabel;

/*!
 * \brief The KisDoubleParseSpinBox class is a cleverer doubleSpinBox, able to parse arithmetic expressions.
 *
 * Use this spinbox instead of the basic one from Qt if you want it to be able to parse arithmetic expressions.
 */
class KRITAWIDGETS_EXPORT KisDoubleParseSpinBox : public QDoubleSpinBox
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
