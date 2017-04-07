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

#include "kis_double_parse_spin_box.h"

#include "kis_num_parser.h"

#include <QLabel>
#include <QPixmap>
#include <QIcon>
#include <QFile>
#include <QLineEdit>
#include <qnumeric.h> // for qIsNaN

KisDoubleParseSpinBox::KisDoubleParseSpinBox(QWidget *parent) :
    QDoubleSpinBox(parent),
    boolLastValid(true),
    lastExprParsed(QStringLiteral("0.0"))
{
    connect(this, SIGNAL(noMoreParsingError()),
                    this, SLOT(clearErrorStyle()));

    //hack to let the clearError be called, even if the value changed method is the one from QDoubleSpinBox.
    connect(this, SIGNAL(valueChanged(double)),
                    this, SLOT(clearError()));

    connect(this, SIGNAL(errorWhileParsing(QString)),
                    this, SLOT(setErrorStyle()));

    oldValue = value();

    warningIcon = new QLabel(this);

    if (QFile(":/./16_light_warning.svg").exists()) {
        warningIcon->setPixmap(QIcon(":/./16_light_warning.svg").pixmap(16, 16));
    } else {
        warningIcon->setText("!");
    }

    warningIcon->setStyleSheet("background:transparent;");
    warningIcon->move(1, 1);
    warningIcon->setVisible(false);

    isOldPaletteSaved = false;
    areOldMarginsSaved = false;

}

KisDoubleParseSpinBox::~KisDoubleParseSpinBox()
{
}

double KisDoubleParseSpinBox::valueFromText(const QString & text) const
{
    lastExprParsed = text;

    bool ok;

    double ret;

    if ( (suffix().isEmpty() || !text.endsWith(suffix())) &&
         (prefix().isEmpty() || !text.startsWith(prefix())) ) {

        ret = KisNumericParser::parseSimpleMathExpr(text, &ok);

    } else {

        QString expr = text;

        if (text.endsWith(suffix())) {
            expr.remove(text.size()-suffix().size(), suffix().size());
        }

        if(text.startsWith(prefix())){
            expr.remove(0, prefix().size());
        }

        lastExprParsed = expr;
        ret = KisNumericParser::parseSimpleMathExpr(expr, &ok);
    }

    if(qIsNaN(ret) || qIsInf(ret)){
        ok = false;
    }

    if (!ok) {
        if (boolLastValid) {
            oldValue = value();
        }

        boolLastValid = false;
        ret = oldValue; //in case of error set to minimum.
    } else {

        if (!boolLastValid) {
            oldValue = ret;
        }

        boolLastValid = true;
    }

    return ret;

}
QString KisDoubleParseSpinBox::textFromValue(double val) const
{

    if (!boolLastValid) {
        emit errorWhileParsing(lastExprParsed);
        return lastExprParsed;
    }
    emit noMoreParsingError();

    double v = KisNumericParser::parseSimpleMathExpr(veryCleanText());
    v = QString("%1").arg(v, 0, 'f', decimals()).toDouble();
    if (hasFocus() && (v == value() || (v > maximum() && value() == maximum()) || (v < minimum() && value() == minimum())) ) { //solve a very annoying bug where the formula can collapse while editing. With this trick the formula is not lost until focus is lost.
        return veryCleanText();
    }

    return QDoubleSpinBox::textFromValue(val);
}

QString KisDoubleParseSpinBox::veryCleanText() const
{
    return cleanText();
}

QValidator::State KisDoubleParseSpinBox::validate ( QString & input, int & pos ) const
{

    Q_UNUSED(input);
    Q_UNUSED(pos);

    return QValidator::Acceptable;

}

void KisDoubleParseSpinBox::stepBy(int steps)
{

    boolLastValid = true; //reset to valid state so we can use the up and down buttons.
    emit noMoreParsingError();

    QDoubleSpinBox::stepBy(steps);

}

void KisDoubleParseSpinBox::setValue(double value)
{
    if(value == oldValue && hasFocus()){ //avoid to reset the button when it set the value of something that will recall this slot.
        return;
    }

    QDoubleSpinBox::setValue(value);

    if (!hasFocus()) {
        clearError();
    }
}

void KisDoubleParseSpinBox::setErrorStyle()
{

    if (!boolLastValid) {
        //setStyleSheet(_oldStyleSheet + "Background: red; color: white; padding-left: 18px;");

        if (!isOldPaletteSaved) {
            oldPalette = palette();
        }
        isOldPaletteSaved = true;

        QPalette nP = oldPalette;
        nP.setColor(QPalette::Background, Qt::red);
        nP.setColor(QPalette::Base, Qt::red);
        nP.setColor(QPalette::Text, Qt::white);
        setPalette(nP);

        if (!areOldMarginsSaved) {
            oldMargins = lineEdit()->textMargins();
        }
        areOldMarginsSaved = true;

        if (width() - height() >= 3*height()) { //if we have twice as much place as needed by the warning icon then display it.
            QMargins newMargins = oldMargins;
            newMargins.setLeft( newMargins.left() + height() - 4 );
            lineEdit()->setTextMargins(newMargins);

            int h = warningIcon->height();
            int hp = height()-2;

            if (h != hp) {
                warningIcon->resize(hp, hp);

                if (QFile(":/./16_light_warning.svg").exists()) {
                    warningIcon->setPixmap(QIcon(":/./16_light_warning.svg").pixmap(hp-7, hp-7));
                }
            }

            warningIcon->move(oldMargins.left()+4, 1);
            warningIcon->setVisible(true);
        }
    }
}

void KisDoubleParseSpinBox::clearErrorStyle()
{
    if (boolLastValid) {
        warningIcon->setVisible(false);

        //setStyleSheet(QString());

        setPalette(oldPalette);
        isOldPaletteSaved = false;

        lineEdit()->setTextMargins(oldMargins);
        areOldMarginsSaved = false;
    }
}
void KisDoubleParseSpinBox::clearError()
{
    boolLastValid = true;
    emit noMoreParsingError();
    oldValue = value();
    clearErrorStyle();
}
