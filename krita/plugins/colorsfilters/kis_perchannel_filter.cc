/* This file is part of the KDE project
   Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kis_perchannel_filter.h"
#include <kdebug.h>
#include "kis_multi_integer_filter_widget.h"
#include "kis_strategy_colorspace.h"

KisPerChannelFilterConfiguration::KisPerChannelFilterConfiguration(Q_INT32 nbintegers, ChannelInfo* ci)
{
	m_values = new Q_INT32[ nbintegers ];
	m_channels = new Q_INT32[ nbintegers ];
	for( Q_INT32 i = 0; i < nbintegers; i++ )
	{
		m_channels[ i ] = ci[ i ].pos();
		m_values[ i ] = 0;
	}
}

KisPerChannelFilter::KisPerChannelFilter( const QString& name, Q_INT32 min, Q_INT32 max, Q_INT32 initvalue ) : KisFilter( name ),
	m_min (min),
	m_max (max),
	m_initvalue (initvalue),
	m_nbchannels ( 0 )
{
	
}

KisFilterConfigurationWidget* KisPerChannelFilter::createConfigurationWidget(QWidget* parent)
{
	vKisIntegerWidgetParam param;
	m_nbchannels = colorStrategy()->depth() - colorStrategy()->alpha();
	for(Q_INT32 i = 0; i < m_nbchannels; i++)
	{
		ChannelInfo* cI = &colorStrategy()->channelsInfo()[i];
		param.push_back( KisIntegerWidgetParam( m_min, m_max, m_initvalue, cI->name().ascii() ) );
	}
	return new KisMultiIntegerFilterWidget(this, parent, name().ascii(), name().ascii(), param );
}

KisFilterConfiguration* KisPerChannelFilter::configuration()
{
	KisMultiIntegerFilterWidget* widget = (KisMultiIntegerFilterWidget*) configurationWidget();
	KisPerChannelFilterConfiguration* co = new KisPerChannelFilterConfiguration( m_nbchannels , colorStrategy()->channelsInfo() );
	if( widget == 0 )
	{
		for(Q_INT32 i = 0; i < m_nbchannels; i++)
		{
			co->valueFor( i ) = 0;
		}
	} else {
		for(Q_INT32 i = 0; i < m_nbchannels; i++)
		{
			co->valueFor( i ) = widget->valueAt( i );
		}
	}
	return co;
}
