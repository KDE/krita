/* This file is part of the KDE project
   Copyright (C) 2002, Benoit Vautrin <benoit.vautrin@free.fr>

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

#ifndef __KOCONTEXTHELPACTION_H__
#define __KOCONTEXTHELPACTION_H__

#include <qwidget.h>
#include <qbitmap.h>
#include <q3dockwindow.h>
//Added by qt3to4:
#include <QPixmap>
#include <QMouseEvent>
#include <QLabel>
#include <QTimerEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QPaintEvent>

#include <kaction.h>
#include <ktoggleaction.h>
#include <koffice_export.h>
class QPixmap;
class QLabel;
class Q3SimpleRichText;

class KoVerticalLabel : public QWidget
{
	Q_OBJECT
	
	public:
		KoVerticalLabel( QWidget* parent = 0, const char* name = 0 );
		~KoVerticalLabel();
		
	public slots:
		void setText( const QString& text );
		
	protected:
		void paintEvent( QPaintEvent* );
		
	private:
		QString m_text;
}; // KoVerticalLabel

class KoHelpNavButton : public QWidget
{
	Q_OBJECT
	
	public:
		enum NavDirection {
			Up,
			Down
		};

		KoHelpNavButton( NavDirection d, QWidget* parent );

	signals:
		void pressed();
		void released();

	protected:
		void paintEvent( QPaintEvent* );
		void enterEvent( QEvent* );
		void leaveEvent( QEvent* );

	private:
		QBitmap      m_bitmap;
		bool         m_pressed;
}; // KoHelpNavButton

class KoTinyButton : public QWidget
{
	Q_OBJECT
	
	public:
		enum Action {
			Close,
			Sticky
		};

		KoTinyButton( Action a, QWidget* parent );

	signals:
		void clicked();
		void toggled( bool );

	protected:
		void paintEvent( QPaintEvent* );
		void mousePressEvent( QMouseEvent* );
		void mouseReleaseEvent( QMouseEvent* );

	private:
		QBitmap      m_bitmap;
		bool         m_pressed;
		Action       m_action;
		bool         m_toggled;
}; // KoTinyButton

class KoHelpView : public QWidget
{
	Q_OBJECT

	public:
		KoHelpView( QWidget* parent );
		~KoHelpView();

		void setText( const QString& text );
		bool eventFilter( QObject* watched, QEvent* e );

	signals:
		void linkClicked( const QString& link );

	protected:
		virtual void mousePressEvent( QMouseEvent* e );
		virtual void mouseReleaseEvent( QMouseEvent* e );
		virtual void mouseMoveEvent( QMouseEvent* e );
		virtual void paintEvent( QPaintEvent* e );

	private:
		Q3SimpleRichText* currentText;
		QString currentAnchor;
}; // KoHelpView

class KoHelpWidget : public QWidget
{
	Q_OBJECT

	public:
		KoHelpWidget( QString help, QWidget* parent );

		void setText( QString text );
		void timerEvent( QTimerEvent* );
		void updateButtons();

	signals:
		void linkClicked( const QString& link );

	public slots:
		void scrollUp();
		void scrollDown();
		void startScrollingUp();
		void startScrollingDown();
		void stopScrolling();

	protected:
		void resizeEvent( QResizeEvent* );

	private:
		int              m_ypos;
		bool             m_scrollDown;
		QWidget*         m_helpViewport;
		KoHelpView*      m_helpView;
		KoHelpNavButton* m_upButton;
		KoHelpNavButton* m_downButton;
}; // KoHelpWidget

/**
 * KoContextHelpPopup is the popup displayed by ContextHelpAction.
 */
class KoContextHelpPopup : public QWidget
{
	Q_OBJECT

	public:
		KoContextHelpPopup( QWidget* parent = 0 );
		~KoContextHelpPopup();

	public slots:
		void setContextHelp( const QString& title, const QString& text, const QPixmap* icon = 0 );
		void setSticky( bool sticky ) { m_isSticky = sticky; }

	protected:
		virtual void mousePressEvent( QMouseEvent* );
		virtual void mouseMoveEvent( QMouseEvent* );
		virtual void resizeEvent( QResizeEvent* );
		virtual void paintEvent( QPaintEvent* );
		virtual void windowActivationChange( bool );
		virtual void keyPressEvent ( QKeyEvent* );
		virtual void keyReleaseEvent ( QKeyEvent* );

	signals:
		void wantsToBeClosed();
		/**
		 * Connect to this signal to receive the href value of the links clicked.
		 */
		void linkClicked( const QString& link );

	private:
		KoHelpWidget*    m_helpViewer;
		KoVerticalLabel* m_helpTitle;
		QLabel*          m_helpIcon;
		KoTinyButton*    m_close;
		KoTinyButton*    m_sticky;

		QPoint           m_mousePos;
		bool             m_isSticky;
}; // KoContextHelpPopup

/**
 * KoContextHelpAction provides a easy to use context help system.
 *
 * This action displays on demand a context help in a popup.
 * The context help is set by the updateHelp slot.
 */
class KOFFICEUI_EXPORT KoContextHelpAction : public KToggleAction
{
	Q_OBJECT

	public:
		KoContextHelpAction( KActionCollection* parent, QWidget* parent = 0 );
		virtual ~KoContextHelpAction();

	public slots:
		void updateHelp( const QString& title, const QString& text, const QPixmap* icon = 0 );
		void closePopup();

	signals:
		/**
		 * Connect to this signal to receive the href value of the links clicked.
		 */
		void linkClicked( const QString& link );

	private:
		KoContextHelpPopup* m_popup;
}; // KoContextHelpAction

class KoContextHelpWidget : public QWidget
{
	Q_OBJECT

	public:
		KoContextHelpWidget( QWidget* parent = 0, const char* name = 0 );
		~KoContextHelpWidget();

	public slots:
		void setContextHelp( const QString& title, const QString& text, const QPixmap* icon = 0 );

	signals:
		/**
		 * Connect to this signal to receive the href value of the links clicked.
		 */
		void linkClicked( const QString& link );

	private:
		KoHelpWidget*    m_helpViewer;
		KoVerticalLabel* m_helpTitle;
		QLabel*          m_helpIcon;
}; // KoContextHelpWidget

class KoContextHelpDocker : public Q3DockWindow
{
	Q_OBJECT

	public:
		KoContextHelpDocker( QWidget* parent = 0, const char* name = 0 );
		~KoContextHelpDocker();

	public slots:
		void setContextHelp( const QString& title, const QString& text, const QPixmap* icon = 0 );

	signals:
		/**
		 * Connect to this signal to receive the href value of the links clicked.
		 */
		void linkClicked( const QString& link );

	private:
		KoHelpWidget*    m_helpViewer;
		KoVerticalLabel* m_helpTitle;
		QLabel*          m_helpIcon;
}; // KoContextHelpDocker

#endif /* __KOCONTEXTHELPACTION_H__ */
