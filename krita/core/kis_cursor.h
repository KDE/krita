/*
 *  kis_cursor.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __kis_cursor_h__
#define __kis_cursor_h__

class QCursor;

class KisCursor
{

 public:

	KisCursor();

	// Predefined Qt cursors.
	static QCursor arrowCursor();         // standard arrow cursor
	static QCursor upArrowCursor();       // upwards arrow
	static QCursor crossCursor();         // crosshair
	static QCursor waitCursor();          // hourglass/watch
	static QCursor ibeamCursor();         // ibeam/text entry
	static QCursor sizeVerCursor();       // vertical resize
	static QCursor sizeHorCursor();       // horizontal resize
	static QCursor sizeBDiagCursor();     // diagonal resize (/)
	static QCursor sizeFDiagCursor();     // diagonal resize (\)
	static QCursor sizeAllCursor();       // all directions resize
	static QCursor blankCursor();         // blank/invisible cursor
	static QCursor splitVCursor();        // vertical splitting
	static QCursor splitHCursor();        // horziontal splitting
	static QCursor pointingHandCursor();  // a pointing hand

	// Custom KimageShop cursors. Use the X utility "bitmap" to create new cursors.
	static QCursor moveCursor();          // move tool cursor
	static QCursor penCursor();           // pen tool cursor
	static QCursor brushCursor();         // brush tool cursor
	static QCursor airbrushCursor();      // airbrush tool cursor
	static QCursor eraserCursor();        // eraser tool cursor
	static QCursor fillerCursor();        // filler tool cursor
	static QCursor pickerCursor();        // color picker cursor
	static QCursor colorChangerCursor();  // color changer tool cursor
	static QCursor selectCursor();        // select cursor
	static QCursor zoomCursor();          // zoom tool cursor
	static QCursor handCursor();          // hand tool cursor
};
#endif // __kis_cursor_h__
