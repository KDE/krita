/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KFORMULAINPUTFILTER_H
#define KFORMULAINPUTFILTER_H

#include <QObject>
#include <qdom.h>

#include "kformuladefs.h"

KFORMULA_NAMESPACE_BEGIN

class KFInputFilter : public QObject
{
Q_OBJECT
    public:
	/*
	 * Get the just created DOM.
	 */
	virtual QDomDocument getKFormulaDom() =0;

	bool isDone() {return done; }


    public slots:
	virtual void startConversion() =0;

    signals:
	void conversionFinished();

    protected:

	bool done;
	bool conversion;
};

KFORMULA_NAMESPACE_END

#endif //
