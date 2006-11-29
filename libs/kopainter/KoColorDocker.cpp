/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002 - 2005, The Karbon Developers
   Copyright (C) 2006 Jan Hambecht <jaham@gmx.net>

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

#include <QDockWidget>
#include <QColor>
#include <QToolTip>
#include <qevent.h>
#include <QMouseEvent>

#include <klocale.h>
#include <KoUniColorChooser.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoCommand.h>
#include <KoToolManager.h>
#include <KoCanvasController.h>

#include "vcolordocker.h"

#include <kdebug.h>

KoColorDockerFactory::KoColorDockerFactory()
{
}

QString KoColorDockerFactory::dockId() const
{
    return QString("Color Chooser");
}

Qt::DockWidgetArea KoColorDockerFactory::defaultDockWidgetArea() const
{
    return Qt::RightDockWidgetArea;
}

QDockWidget* KoColorDockerFactory::createDockWidget()
{
    KoColorDocker* widget = new KoColorDocker();
    widget->setObjectName(dockId());

    return widget;
}

KoColorDocker::KoColorDocker()
: m_isStrokeDocker( false )
{
	setWindowTitle( i18n( "Color Chooser" ) );

	m_colorChooser = new KoUniColorChooser( this, true );
	setWidget( m_colorChooser );
	//setMaximumHeight( 174 );
	setMinimumWidth( 194 );

	connect( m_colorChooser, SIGNAL( sigColorChanged( const KoColor &) ), this, SLOT( updateColor( const KoColor &) ) );
	//connect( m_colorChooser, SIGNAL( sigColorChanged( const QColor &) ), this, SLOT( updateBgColor( const QColor &) ) );
	connect(this, SIGNAL(colorChanged(const KoColor &)), m_colorChooser, SLOT(setColor(const KoColor &)));
	//connect(this, SIGNAL(bgColorChanged(const QColor &)), mHSVWidget, SLOT(setBgColor(const QColor &)));
}

KoColorDocker::~KoColorDocker()
{
}

void KoColorDocker::updateColor( const KoColor &c )
{
	KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();
	KoSelection *selection = canvasController->canvas()->shapeManager()->selection();
	if( ! selection )
		return;

	QColor color;
	quint8 opacity;
	c.toQColor(&color, &opacity);
	color.setAlpha(opacity);

	KoShapeBackgroundCommand *cmd = new KoShapeBackgroundCommand( selection->selectedShapes(), QBrush( color ) );
	canvasController->canvas()->addCommand( cmd, true );
}

void KoColorDocker::updateFgColor(const KoColor &c)
{
	m_colorChooser->blockSignals(true);

	m_oldColor = m_color;

	m_color = c;
	/*
	KoColor v = KoColor(c);
	v.setOpacity( m_opacity );

	KoCommandHistory* history = m_part->commandHistory();
	const Q3PtrList<KoCommand>* commandList = history->commands();
	VStrokeCmd* command = dynamic_cast<VStrokeCmd*>(commandList->getLast());

	if(command == 0 || m_strokeCmd == 0)
	{
		m_strokeCmd = new VStrokeCmd( &m_part->document(), v );
		m_part->addCommand( m_strokeCmd, true );
	}
	else
	{

		Q3PtrList<VObject> VOldObjectList = command->getSelection()->objects();
		Q3PtrList<VObject> VNewObjectList = m_part->document().selection()->objects();

		if( VOldObjectList == VNewObjectList )
		{
			m_strokeCmd->changeStroke(v);
			m_part->repaintAllViews();
		}
		else
		{
			m_strokeCmd = new VStrokeCmd( &m_part->document(), v );
			m_part->addCommand( m_strokeCmd, true );
		}
	}
	*/
	emit colorChanged( c );

	m_colorChooser->blockSignals(false);
}

void KoColorDocker::updateBgColor(const KoColor &c)
{
	m_colorChooser->blockSignals(true);

	m_oldColor = m_color;

	m_color = c;

	/*
	KoColor v = KoColor(c);
	v.setOpacity( m_opacity );

	KoCommandHistory* history = m_part->commandHistory();
	const Q3PtrList<KoCommand>* commandList = history->commands();
	VFillCmd* command = dynamic_cast<VFillCmd*>(commandList->getLast());

	if(command == 0 || m_fillCmd == 0)
	{
		m_fillCmd = new VFillCmd( &m_part->document(), VFill(v) );
		m_part->addCommand( m_fillCmd, true );
	}
	else
	{

		Q3PtrList<VObject> VOldObjectList = command->getSelection()->objects();
		Q3PtrList<VObject> VNewObjectList = m_part->document().selection()->objects();

		if( VOldObjectList == VNewObjectList )
		{
			m_fillCmd->changeFill(VFill(v));
			m_part->repaintAllViews();
		}
		else
		{
			m_fillCmd = new VFillCmd( &m_part->document(), VFill(v) );
			m_part->addCommand( m_fillCmd, true );
		}
	}
	*/
	emit colorChanged( c );

	m_colorChooser->blockSignals(false);
}

void
KoColorDocker::mouseReleaseEvent( QMouseEvent * )
{
	//changeColor();
}

void KoColorDocker::setFillDocker()
{
	m_isStrokeDocker = false;
}

void KoColorDocker::setStrokeDocker()
{
	m_isStrokeDocker = true;
}

void KoColorDocker::update()
{
	/*
	mHSVWidget->blockSignals(true);

	int objCnt = m_part->document().selection()->objects().count();

	if( objCnt > 0 )
	{
		VObject *obj = m_part->document().selection()->objects().getFirst();

		QColor fgColor = QColor(obj->stroke()->color());
		QColor bgColor = QColor(obj->fill()->color());

		//mHSVWidget->setFgColor(fgColor);
		//mHSVWidget->setBgColor(bgColor);
	}

	mHSVWidget->blockSignals(false);
	*/
}

#include "vcolordocker.moc"

