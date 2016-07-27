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

#include "kis_intparsespinbox.h"

#include "kis_numparser.h"

#include <QLabel>
#include <QPixmap>
#include <QIcon>
#include <QFile>
#include <QLineEdit>

KisIntParseSpinBox::KisIntParseSpinBox(QWidget *parent) :
    QSpinBox(parent),
    _isLastValid(true)
{
    _lastExprParsed = new QString("0");

    connect(this, SIGNAL(noMoreParsingError()),
					this, SLOT(clearErrorStyle()));

	//hack to let the clearError be called, even if the value changed method is the one from QSpinBox.
	connect(this, SIGNAL(valueChanged(int)),
					this, SLOT(clearError()));

	connect(this, SIGNAL(errorWhileParsing(QString)),
					this, SLOT(setErrorStyle()));

	_oldVal = value();

	_warningIcon = new QLabel(this);

	if (QFile(":/./16_light_warning.svg").exists()) {
		_warningIcon->setPixmap(QIcon(":/./16_light_warning.svg").pixmap(16, 16));
	} else {
		_warningIcon->setText("!");
	}

	_warningIcon->setStyleSheet("background:transparent;");
	_warningIcon->move(1, 1);
	_warningIcon->setVisible(false);

	_isOldPaletteSaved = false;
	_areOldMarginsSaved = false;

}

KisIntParseSpinBox::~KisIntParseSpinBox()
{

    //needed to avoid a segfault during destruction.
    delete _lastExprParsed;

}

int KisIntParseSpinBox::valueFromText(const QString & text) const
{

    *_lastExprParsed = text;

    bool ok;

	int val;

	if ( (suffix().isEmpty() || !text.endsWith(suffix())) &&
		 (prefix().isEmpty() || !text.startsWith(prefix())) ) {

		val = KisNumericParser::parseIntegerMathExpr(text, &ok);

	} else {

		QString expr = text;

		if (text.endsWith(suffix())) {
			expr.remove(text.size()-suffix().size(), suffix().size());
		}

		if(text.startsWith(prefix())){
			expr.remove(0, prefix().size());
		}

		*_lastExprParsed = expr;

		val = KisNumericParser::parseIntegerMathExpr(expr, &ok);

	}

	if (text.trimmed().isEmpty()) { //an empty text is considered valid in this case.
            ok = true;
    }

	if (!ok) {

			if (_isLastValid == true) {
				_oldVal = value();
			}

            _isLastValid = false;
            //emit errorWhileParsing(text); //if uncommented become red everytime the string is wrong.
			val = _oldVal;
    } else {

			if (_isLastValid == false) {
				_oldVal = val;
			}

            _isLastValid = true;
            //emit noMoreParsingError();
    }

    return val;

}

QString KisIntParseSpinBox::textFromValue(int val) const
{

	if (!_isLastValid) {
		emit errorWhileParsing(*_lastExprParsed);
		return *_lastExprParsed;
	}

    emit noMoreParsingError();

	int v = KisNumericParser::parseIntegerMathExpr(cleanText());
	if (hasFocus() && (v == value() || (v >= maximum() && value() == maximum()) || (v <= minimum() && value() == minimum())) ) { //solve a very annoying bug where the formula can collapse while editing. With this trick the formula is not lost until focus is lost.
		return cleanText();
	}

    return QSpinBox::textFromValue(val);

}

QValidator::State KisIntParseSpinBox::validate ( QString & input, int & pos ) const
{

    Q_UNUSED(input);
    Q_UNUSED(pos);

	//this simple definition is sufficient for the moment
	//TODO: see if needed to get something more complex.
    return QValidator::Acceptable;

}

void KisIntParseSpinBox::stepBy(int steps)
{

    _isLastValid = true;
    emit noMoreParsingError();

    QSpinBox::stepBy(steps);

}

void KisIntParseSpinBox::setValue(int val)
{

	if(val == _oldVal && hasFocus()){ //avoid to reset the button when it set the value of something that will recall this slot.
		return;
	}

	if (!hasFocus()) {
		clearError();
	}

	QSpinBox::setValue(val);
}

void KisIntParseSpinBox::setErrorStyle()
{

	if (!_isLastValid) {
		//setStyleSheet(_oldStyleSheet + "Background: red; color: white; padding-left: 18px;");

		if (!_isOldPaletteSaved) {
			_oldPalette = palette();
		}
		_isOldPaletteSaved = true;

		QPalette nP = _oldPalette;
		nP.setColor(QPalette::Background, Qt::red);
		nP.setColor(QPalette::Base, Qt::red);
		nP.setColor(QPalette::Text, Qt::white);
		setPalette(nP);

		if (!_areOldMarginsSaved) {
			_oldMargins = lineEdit()->textMargins();
		}
		_areOldMarginsSaved = true;

		if (width() - height() >= 3*height()) { //if we have twice as much place as needed by the warning icon then display it.
			QMargins newMargins = _oldMargins;
			newMargins.setLeft( newMargins.left() + height() - 4 );
			lineEdit()->setTextMargins(newMargins);

			int h = _warningIcon->height();
			int hp = height()-2;

			if (h != hp) {
				_warningIcon->resize(hp, hp);

				if (QFile(":/./16_light_warning.svg").exists()) {
					_warningIcon->setPixmap(QIcon(":/./16_light_warning.svg").pixmap(hp-7, hp-7));
				}
			}

			_warningIcon->move(_oldMargins.left()+4, 1);
			_warningIcon->setVisible(true);
		}
	}
}

void KisIntParseSpinBox::clearErrorStyle()
{
	if (_isLastValid) {
		_warningIcon->setVisible(false);

		//setStyleSheet("");

		setPalette(_oldPalette);
		_isOldPaletteSaved = false;

		lineEdit()->setTextMargins(_oldMargins);
		_areOldMarginsSaved = false;
    }
}


void KisIntParseSpinBox::clearError()
{
	_isLastValid = true;
	emit noMoreParsingError();
	_oldVal = value();
	clearErrorStyle();
}
