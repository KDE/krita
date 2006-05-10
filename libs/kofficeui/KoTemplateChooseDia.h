/*
   This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   2000, 2001 Werner Trobin <trobin@kde.org>
   2002, 2003 Thomas Nagy <tnagy@eleve.emn.fr>
   2004 David Faure <faure@kde.org>

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

#ifndef koTemplateChooseDia_h
#define koTemplateChooseDia_h

#include <kdialogbase.h>
#include <kicondialog.h>
#include <k3iconview.h>
#include <koffice_export.h>
//Added by qt3to4:
#include <QPixmap>
#include <QHideEvent>
#include <QKeyEvent>
#include <QByteArray>

// KoTCD : KoTemplateChooseDia

class KoTCDIconViewItem;
class KoTemplateTree;
class KoTemplateGroup;
class QGridLayout;

/**
 * Our reimplementation of KIconCanvas used within the template-chooser dialog.
 * @internal
 */
class KoTCDIconCanvas : public KIconCanvas
{
    Q_OBJECT
    public:
	KoTCDIconCanvas( QWidget *parent = 0, const char *name = 0L )
	    : KIconCanvas( parent ) {}

	bool isCurrentValid() { return currentItem(); }
	Q3IconViewItem * load(KoTemplateGroup *group, const QString& name, KInstance* instance);

    protected:
	virtual void keyPressEvent( QKeyEvent *e ) {
	    if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter )
		e->ignore();
	    else
		KIconCanvas::keyPressEvent( e );
	}
};

/// @internal
class KoTCDIconViewItem : public K3IconViewItem
{
    public:
	KoTCDIconViewItem(Q3IconView *parent=0)
	    : K3IconViewItem ( parent )
	    {}

	KoTCDIconViewItem(Q3IconView *parent=0, const QString &text=0, const QPixmap &icon=0,
                      const QString &descr=0, const QString &fullname=0)
	    : K3IconViewItem(parent, text, icon)
	    {
            m_descr = descr;
            m_full = fullname;
	    }

	QString getDescr() const { return m_descr; }
	QString getFName() const { return m_full; }

    private :
	QString m_descr;
	QString m_full;

};

#include <kfileiconview.h>
#include <QLabel>
/**
 * Our reimplementation of KFileIconView used as the "recent files" view
 * within the template-chooser dialog.
 * @internal
 */
class KoTCDRecentFilesIconView : public KFileIconView {
    Q_OBJECT
    public:
	KoTCDRecentFilesIconView( QWidget* parent, const char* name ) :
		KFileIconView( parent, name ), toolTip(0)
	{
	    connect( this, SIGNAL( onItem( Q3IconViewItem * ) ),
                     SLOT( showToolTip( Q3IconViewItem * ) ) );
	    connect( this, SIGNAL( onViewport() ),
                     SLOT( removeToolTip() ) );
	}
        virtual ~KoTCDRecentFilesIconView();
    protected:
        /**
         * Reimplemented to remove an eventual tooltip
         */
        virtual void hideEvent( QHideEvent * );

    private slots:
        void showToolTip( Q3IconViewItem* );
        void removeToolTip();
    private:
        QLabel* toolTip;
};

class KInstance;
class KoTemplateChooseDiaPrivate;

/**
 *  This class is used to show the template dialog
 *  on startup. Unless you need something special, you should use the static
 *  method choose().
 *
 *  @short The template choose dialog
 *  @author Reginald Stadlbauer <reggie@kde.org>
 *  @author Werner Trobin <trobin@kde.org>
 */
class KOFFICEUI_EXPORT KoTemplateChooseDia : public KDialogBase
{
    Q_OBJECT

public:
    /**
     * The Dialog returns one of these values depending
     * on the input of the user.
     * Cancel = The user pressed 'Cancel'
     * Template = The user selected a template
     * File = The user has chosen a file
     * Empty = The user selected "Empty document"
     */
    enum ReturnType { Cancel, Template, File, Empty };
    /**
     * To configure the dialog you have to use this enum.
     * Everything = Show templates and the rest of the dialog
     * OnlyTemplates = Show only the templates
     * NoTemplates = Just guess :)
     */
    enum DialogType { Everything, OnlyTemplates, NoTemplates };

    ~KoTemplateChooseDia();

    /**
     * This is the static method you'll normally use to show the
     * dialog.
     *
     * @param instance the KInstance of your app
     * The native mimetype is retrieved from the (desktop file of) that instance.
     * @param file this is the filename which is returned to your app
     * More precisely, it's a url (to give to KUrl) if ReturnType is File
     * and it's a path (to open directly) if ReturnType is Template
     *
     * @param dialogType the type of the dialog
     * @param templateType the template type of your application (see kword or
     *        kpresenter for details)
     * @param parent pointer to parent widget
     * @return The return type (see above)
     */
    static ReturnType choose(KInstance* instance, QString &file,
                             const DialogType &dialogType,
                             const QByteArray& templateType,
                             QWidget* parent);

private:
    /// Ditto, with extraNativeMimeTypes added
    static ReturnType choose(KInstance* instance, QString &file,
                             const QByteArray &format,
                             const QString &nativeName,
                             const QStringList& extraNativeMimeTypes,
                             const DialogType &dialogType=Everything,
                             const QByteArray& templateType="",
                             QWidget* parent = 0);
public:

    /**
     * Method to get the current template
     */
    QString getTemplate() const;
    /**
     * Method to get the "full" template (path+template)
     */
    QString getFullTemplate() const;
    /**
     * The ReturnType (call this one after exec())
     */
    ReturnType getReturnType() const;
    /**
     * The dialogType - normally you won't need this one
     */
    DialogType getDialogType() const;

protected slots:
    /**
     * Activated when the Ok button has been clicked.
     */
    virtual void slotOk();

private:
    /**
     *
     * @param parent parent the parent of the dialog
     * @param name the Qt internal name
     * @param instance the KInstance of your app
     * @param format is the mimetype of the app (e.g. application/x-kspread)
     * @param nativeName is the name of your app (e.g KSpread)
     * @param dialogType the type of the dialog
     * @param templateType the template type of your application (see kword or
     *        kpresenter for details)
     *
     * @return The return type (see above)
     */
    KoTemplateChooseDia(QWidget *parent, const char *name, KInstance* instance,
                        const QByteArray &format,
                        const QString &nativeName,
                        const QStringList &extraNativeMimeTypes,
                        const DialogType &dialogType=Everything,
                        const QByteArray& templateType="");

private:
    KoTemplateChooseDiaPrivate *d;

    QString descriptionText(const QString &name, const QString &description);
    void setupDialog();
    void setupTemplateDialog(QWidget * widgetbase, QGridLayout * layout);
    void setupFileDialog(QWidget * widgetbase, QGridLayout * layout);
    void setupRecentDialog(QWidget * widgetbase, QGridLayout * layout);
    bool collectInfo();
    bool noStartupDlg() const;

private slots:

    void chosen(Q3IconViewItem *);
    void currentChanged( Q3IconViewItem * );
    void recentSelected( Q3IconViewItem * );
};

#endif

