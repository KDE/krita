/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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

#ifndef KIS_PROGRESS_DISPLAY_INTERFACE_H_
#define KIS_PROGRESS_DISPLAY_INTERFACE_H_

class KisProgressSubject;

class KisProgressDisplayInterface {
public:
	KisProgressDisplayInterface() {}
	virtual ~KisProgressDisplayInterface() {}

	virtual void setSubject(KisProgressSubject *subject, bool modal, bool canCancel) = 0;

private:
	KisProgressDisplayInterface(const KisProgressDisplayInterface&);
	KisProgressDisplayInterface& operator=(const KisProgressDisplayInterface&);
};

#endif // KIS_PROGRESS_DISPLAY_INTERFACE_H_

