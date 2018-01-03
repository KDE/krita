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

#ifndef KISINTPARSESPINBOX_H
#define KISINTPARSESPINBOX_H

#include <QSpinBox>

#include "kritawidgets_export.h"

class QLabel;

/*!
 * \brief The KisDoubleParseSpinBox class is a cleverer doubleSpinBox, able to parse arithmetic expressions.
 *
 * Use this spinbox instead of the basic one from Qt if you want it to be able to parse arithmetic expressions.
 */
class KRITAWIDGETS_EXPORT KisIntParseSpinBox : public QSpinBox
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
