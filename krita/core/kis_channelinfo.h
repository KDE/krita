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

enum enumChannelType {
	COLOR,
	ALPHA,
	SUBSTANCE
};

/** 
 * This class gives some basic information about a channel,
 * that is, one of the components that makes up a particular 
 * pixel.
 */
class KisChannelInfo {
public:
	KisChannelInfo() { };
	KisChannelInfo( const QString & name, Q_INT32 npos, enumChannelType channelType)
		: m_name (name), m_pos (npos), m_size(1), m_channelType(channelType) { };
public:
	/**
	 * User-friendly name for this channel for presentation purposes in the gui
	 */
	inline QString name() const { return m_name; };
	
	/** 
	 * returns the position of the channel in the pixel
	 */
	inline Q_INT32 pos() const { return m_pos; };
	
	/**
	 * returns the number of bytes this channel takes
	 */
	inline Q_INT32 size() const { return m_size; };

	/**
	 * returns the type of the channel
	 */
	inline enumChannelType channelType() const { return m_channelType; };
	
private:
	QString m_name;
	Q_INT32 m_pos;
	Q_INT32 m_size;
	enumChannelType m_channelType;
};

#endif // KIS_CHANNELINFO_H_
