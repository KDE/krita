/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_CHANNELINFO_H_
#define KIS_CHANNELINFO_H_

#include "qstring.h"

/** 
 * This class gives some basic information about a channel,
 * that is, one of the components that makes up a particular 
 * pixel.
 *
 * XXX: refactor name to conform to Krita guidelines.
 */
class ChannelInfo {
	public:
		ChannelInfo() { };
		ChannelInfo( const QString & name, Q_INT32 npos ) : m_name (name), m_pos (npos) { };
	public:
		/**
		 * User-friendly name for this channel for presentation purposes in the gui
		 */
		inline QString name() const { return m_name; };
		/** 
		 * returns the position of the channel in the pixel
		 */
		inline Q_INT32 pos() const { return m_pos; };

	private:
		QString m_name;
		Q_INT32 m_pos;
};

#endif // KIS_CHANNELINFO_H_
