/*
 *  preferencesdlg.h - part of KImageShop
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __preferencesdlg_h__
#define __preferencesdlg_h__

#include <qwidget.h>

#include <kdialogbase.h>

class QLineEdit;
class QCheckBox;
class KURLRequester;


/**
 *  "General"-tab for preferences dialog
 */
class GeneralTab : public QWidget
{
  Q_OBJECT

public:

  GeneralTab( QWidget *_parent = 0, const char *_name = 0 );

  bool saveOnExit();

private:

  QCheckBox *m_saveOnExit;
};

/**
 *  "Directories"-tab for preferences dialog
 */
class DirectoriesTab : public QWidget
{
  Q_OBJECT

public:

    DirectoriesTab( QWidget *_parent = 0, const char *_name = 0 );

private slots:

    void slotRequesterClicked( KURLRequester * );

private:

    KURLRequester *m_pLineEdit, *m_pGimpGradients;
};

/* jwc - undo-redo not working yet - still we can show the 
options and keep them in mind.  Harmless as the actions don't
do anything yet */

class UndoRedoTab : public QWidget
{
    Q_OBJECT

public:

    UndoRedoTab( QWidget *_parent = 0, const char *_name = 0 );
};


/**
 *  Preferences dialog of KImageShop
 */
class PreferencesDialog : public KDialogBase
{
  Q_OBJECT

public:

    static void editPreferences();

protected:

    PreferencesDialog( QWidget *_parent = 0, const char *_name = 0 );
    ~PreferencesDialog();

private:

    GeneralTab     *m_general;
    DirectoriesTab *m_directories;
    UndoRedoTab    *m_undoRedo; 
};

#endif
