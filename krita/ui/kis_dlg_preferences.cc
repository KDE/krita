/*
 *  preferencesdlg.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
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

#include <qvbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcursor.h>
#include <qpixmap.h>
#include <qbitmap.h>

#include <klocale.h>
#include <knuminput.h>
#include <kfiledialog.h>
#include <kurlrequester.h>
#include <klineedit.h>

#include "kis_cursor.h"
#include "kis_config.h"
#include "kis_dlg_preferences.h"

GeneralTab::GeneralTab( QWidget *_parent, const char *_name )
	: QWidget( _parent, _name )
{
	// Layout
	QGridLayout* grid = new QGridLayout( this, 3, 1, KDialog::marginHint(), KDialog::spacingHint());

// 	// checkbutton
// 	m_saveOnExit 
// 		= new QCheckBox( i18n( "Save and restore dialog geometries" ), this );
// 	grid->addWidget( m_saveOnExit, 0, 0 );

	QLabel* label;
	label = new QLabel(this, i18n("Cursor shape"), this);
	grid -> addWidget(label, 0, 0);
       
	m_cmbCursorShape = new QComboBox(this);
// 	m_cmbCursorShape -> insertItem(*KisCursor::brushCursor().bitmap(), "Tool icon");
// 	m_cmbCursorShape -> insertItem(*KisCursor::crossCursor().bitmap(), "Crosshair");
// 	m_cmbCursorShape -> insertItem(*KisCursor::arrowCursor().bitmap(), "Arrow");
// 	m_cmbCursorShape -> insertItem("Brush shape");
	m_cmbCursorShape -> insertItem("Tool icon");
	m_cmbCursorShape -> insertItem("Crosshair");
	m_cmbCursorShape -> insertItem("Arrow");
	
	KisConfig cfg;
	m_cmbCursorShape -> setCurrentItem(cfg.defCursorStyle());

	grid -> addWidget(m_cmbCursorShape, 1, 0);



	// only for testing it
	/* KIntNumInput* i = new KIntNumInput( "a", 1, 100, 1, 1, 
	   QString::null, 10, true, this ); */
        
// 	KIntNumInput* i = new KIntNumInput(1, this, 10, "a");
// 	i->setRange(1, 100, 1);
// 	grid->addWidget( i, 1, 0 );

	

	grid->setRowStretch( 2, 1 );
}


bool GeneralTab::saveOnExit()
{
	return m_saveOnExit->isChecked();
}

enumCursorStyle GeneralTab::cursorStyle() 
{
	return (enumCursorStyle)m_cmbCursorShape -> currentItem();
}


DirectoriesTab::DirectoriesTab( QWidget *_parent, const char *_name )
	: QWidget( _parent, _name )
{
	QLabel* label;

	// Layout
	QGridLayout* grid = new QGridLayout( this, 5, 1, KDialog::marginHint(), KDialog::spacingHint());

	// Inputline
	m_pLineEdit = new KURLRequester( this, "tempDir" );
	connect( m_pLineEdit, SIGNAL( openFileDialog( KURLRequester * )),
		 SLOT( slotRequesterClicked( KURLRequester * )));
	grid->addWidget( m_pLineEdit, 1, 0 );

	// Label
	label = new QLabel( this, i18n( "Directory for temporary files:" ) , this );
	grid->addWidget( label, 0, 0 );

	// Inputline
	m_pGimpGradients = new KURLRequester( this, "gimpGradientDir" );
	connect( m_pLineEdit, SIGNAL( openFileDialog( KURLRequester * )),
		 SLOT( slotRequesterClicked( KURLRequester * )));
	grid->addWidget( m_pGimpGradients, 3, 0 );

	// Label
	label = new QLabel( this, i18n( "Directory of GIMP gradients:" ) , this );
	grid->addWidget( label, 2, 0 );

	grid->setRowStretch( 4, 1 );
}

// delayed KURLRequester configuration to avoid reading directories right
// on dialog construction
void DirectoriesTab::slotRequesterClicked( KURLRequester *requester )
{
	// currently, all KURLRequesters are in directory mode
	requester->fileDialog()->setMode(KFile::Directory);
}


UndoRedoTab::UndoRedoTab( QWidget *_parent, const char *_name  )
	: QWidget( _parent, _name )
{
	// Layout
	QGridLayout* grid = new QGridLayout( this, 3, 1, KDialog::marginHint(), KDialog::spacingHint());

	QLabel *label;

	label = new QLabel( i18n( "Undo depth totally" ), this );
	grid->addWidget( label, 0, 0 );

	label = new QLabel( i18n( "Undo depth in memory" ), this );
	grid->addWidget( label, 1, 0 );

	grid->setRowStretch( 2, 1 );
}


PreferencesDialog::PreferencesDialog( QWidget* parent, const char* name )
	: KDialogBase( IconList, i18n("Preferences"), Ok | Cancel | Help | Default | Apply, Ok, parent, name, true, true )
{
	QVBox *vbox;

	vbox = addVBoxPage( i18n( "General") );
	m_general = new GeneralTab( vbox );

	vbox = addVBoxPage( i18n( "Directories") );
	m_directories = new DirectoriesTab( vbox );

	vbox = addVBoxPage( i18n( "Undo/Redo") );
	m_undoRedo = new UndoRedoTab( vbox );
}

PreferencesDialog::~PreferencesDialog()
{
}

void PreferencesDialog::editPreferences()
{
	PreferencesDialog* dialog;

	dialog = new PreferencesDialog();
	if( dialog->exec() == Accepted )
	{
 		KisConfig cfg;
 		cfg.defCursorStyle(dialog -> m_general -> cursorStyle());
	}
}

#include "kis_dlg_preferences.moc"
