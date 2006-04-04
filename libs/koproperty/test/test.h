/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>

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

#ifndef _TEST_H_
#define _TEST_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kmainwindow.h>

#include "set.h"

/**
 * @short Application Main Window
 * @author Cédric Pasteur <cedric.pasteur@free.fr>
 * @version 0.1
 */
class test : public KMainWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    test();

    /**
     * Default Destructor
     */
    virtual ~test();
    
private:
	KoProperty::Set *m_set;
};

#endif // _TEST_H_
