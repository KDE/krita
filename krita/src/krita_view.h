/***************************************************************************
 *   Copyright (C) 2004 by Boudewijn Rempt                                 *
 *   boud@valdyas.org                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/* This template is based off of the KOffice example written by Torben Weis <weis@kde.org
   It was converted to a KDevelop template by Ian Reinhart Geiser <geiseri@yahoo.com>
*/

#ifndef krita_VIEW
#define krita_VIEW

#include <koView.h>

class KAction;
class QPaintEvent;

class kritaPart;

class kritaView : public KoView
{
    Q_OBJECT
public:
    kritaView( kritaPart* part, QWidget* parent = 0, const char* name = 0 );

protected slots:
    void cut();

protected:
    void paintEvent( QPaintEvent* );

    virtual void updateReadWrite( bool readwrite );

private:
    //KAction* m_cut;
};

#endif
