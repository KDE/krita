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

#include <QDebug>
#include <iostream>

KisIntParseSpinBox::KisIntParseSpinBox(QWidget *parent) :
    QSpinBox(parent),
    _isLastValid(true)
{
    _lastExprParsed = new QString("0");

    connect(this, SIGNAL(noMoreParsingError()),
					this, SLOT(clearErrorStyle()));

	connect(this, SIGNAL(errorWhileParsing(QString)),
					this, SLOT(setErrorStyle()));

}

KisIntParseSpinBox::~KisIntParseSpinBox(){

    //needed to avoid a segfault during destruction.
    delete _lastExprParsed;

}

int KisIntParseSpinBox::valueFromText(const QString & text) const{

    *_lastExprParsed = text;

    bool ok;

    int val = KisNumericParser::parseIntegerMathExpr(text, &ok);

    if(text.trimmed().isEmpty()){ //an empty text is considered valid in this case.
            ok = true;
    }

	if(!ok){

			if(_isLastValid == true){
				_oldVal = value();
			}

            _isLastValid = false;
            //emit errorWhileParsing(text); //if uncommented become red everytime the string is wrong.
			val = _oldVal;
    } else {

			if(_isLastValid == false){
				_oldVal = val;
			}

            _isLastValid = true;
            //emit noMoreParsingError();
    }

    return val;

}

QString KisIntParseSpinBox::textFromValue(int val) const{

    if(!_isLastValid){
            emit errorWhileParsing(*_lastExprParsed);
            return *_lastExprParsed;
    }

    emit noMoreParsingError();
    return QSpinBox::textFromValue(val);

}

QValidator::State KisIntParseSpinBox::validate ( QString & input, int & pos ) const{

    Q_UNUSED(input);
    Q_UNUSED(pos);

    return QValidator::Acceptable;

}

void KisIntParseSpinBox::stepBy(int steps){

    _isLastValid = true;
    emit noMoreParsingError();

    QSpinBox::stepBy(steps);

}

void KisIntParseSpinBox::setValue(int val){

	if(!hasFocus()){
		clearError();
	}

	QSpinBox::setValue(val);
}

void KisIntParseSpinBox::setErrorStyle(){
    if(!_isLastValid){
        setStyleSheet("Background: red; color: white;");
    }
}

void KisIntParseSpinBox::clearErrorStyle(){
    if(_isLastValid){
        setStyleSheet("");
    }
}


void KisIntParseSpinBox::clearError(){
	_isLastValid = true;
	_oldVal = value();
	clearErrorStyle();
}
