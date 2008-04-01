/*
 * Copyright (C)  Hans Bakker <hansmbakker@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _SPACENAV_H_
#define _SPACENAV_H_

#include <kparts/plugin.h>
#include <QPoint>
#include <QX11Info>
#include <QThread>
#include <KoCanvasController.h>
#include <kis_view2.h>
#include <spnav.h>
#include "Ko3DMouseEvent.h"

class KisView2;

class PollingThread : public QThread
	{
	public:
	void run();
	bool polling;
	KisView2 * m_view;
	KoCanvasController * m_controller;
	QX11Info m_x11info;
	spnav_event sev;
	QPoint distance;
	private:
	bool drop;
	event = new Ko3DMouseEvent();
	};

	void PollingThread::run()
	{
		drop=false;
		while(polling)
		{
			if(drop)
			{
				spnav_remove_events(SPNAV_EVENT_MOTION);
				drop=false;
			}
			else
			{
				if(spnav_poll_event(&sev))
				{
					event->setType(sev.type);
					if(sev.type == SPNAV_EVENT_MOTION) {
						//printf("got motion event: t(%d, %d, %d) ", sev.motion.x, sev.motion.y, sev.motion.z);
						//printf("r(%d, %d, %d)\n", sev.motion.rx, sev.motion.ry, sev.motion.rz);
						event->setMovement(sev.motion.x, sev.motion.y, sev.motion.z, sev.motion.rx, sev.motion.ry, sev.motion.rz);
					} else {	/* SPNAV_EVENT_BUTTON */
						//printf("got button %s event b(%d)\n", sev.button.press ? "press" : "release", sev.button.bnum);
						event->setButton(sev.button.bnum, sev.button.press);
					}
					m_controller->mouse3DEvent(event);
				}
				drop=true;
			}
			msleep(10);
		}
		exec();
	}

/**
* Template of view plugin
*/
class SpaceNavigatorPlugin : public KParts::Plugin
{
Q_OBJECT
public:
SpaceNavigatorPlugin(QObject *parent, const QStringList &);
virtual ~SpaceNavigatorPlugin();

private slots:

void slotTogglePolling();

private:
PollingThread * thread;
Ko3DMouseEvent * event;
};

#endif // SpaceNavigatorPlugin_H
