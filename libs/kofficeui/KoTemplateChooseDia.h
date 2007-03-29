/*
   This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright 2000, 2001 Werner Trobin <trobin@kde.org>
   Copyright 2002, 2003 Thomas Nagy <tnagy@eleve.emn.fr>
   Copyright 2004, 2007 David Faure <faure@kde.org>

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

#include <kpagedialog.h>
#include <kicondialog.h>
#include <kofficeui_export.h>

class QPixmap;
class QByteArray;
class KoTemplateTree;
class KoTemplateGroup;
class QGridLayout;
class KComponentData;

// KoTCD : KoTemplateChooseDia
class KoTCDIconViewItem;

/**
 * Our reimplementation of KIconCanvas used within the template-chooser dialog.
 * @internal
 */
class KoTCDIconCanvas : public KIconCanvas
{
    Q_OBJECT
public:
    explicit KoTCDIconCanvas( QWidget *parent = 0, const char *name = 0L )
	    : KIconCanvas( parent ) { Q_UNUSED(name) }

	bool isCurrentValid() { return currentItem(); }
	QListWidgetItem * load(KoTemplateGroup *group, const QString& name, const KComponentData &instance);

protected:
    virtual void keyPressEvent( QKeyEvent *e );
};

/// @internal
class KoTCDIconViewItem : public QListWidgetItem
{
    public:
	KoTCDIconViewItem(QListWidget *parent=0)
	    : QListWidgetItem ( parent )
	    {}

    explicit KoTCDIconViewItem(QListWidget *parent=0, const QString &text=0, const QPixmap &icon=0,
                      const QString &descr=0, const QString &fullname=0)
	    : QListWidgetItem(text, parent)
	    {
            setIcon(icon);
            m_descr = descr;
            m_full = fullname;
	    }

	QString getDescr() const { return m_descr; }
	QString getFName() const { return m_full; }

    private :
	QString m_descr;
	QString m_full;

};

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
class KOFFICEUI_EXPORT KoTemplateChooseDia : public KPageDialog
{
    Q_OBJECT

public:
    /**
     * The Dialog returns one of these values depending
     * on the input of the user.
     * Cancel = The user pressed 'Cancel'
     * Template = The user selected a template
     * Empty = The user selected "Empty document"
     */
    enum ReturnType { Cancel, Template, Empty };
    /**
     * To configure the dialog you have to use this enum.
     * OnlyTemplates = Show only the templates
     * This is the only supported option in KOffice 2.
     */
    enum DialogType { OnlyTemplates };

    ~KoTemplateChooseDia();

    /**
     * This is the static method you'll normally use to show the
     * dialog.
     *
     * @param instance the KComponentData of your app
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
    static ReturnType choose(const KComponentData &instance, QString &file,
                             const DialogType &dialogType,
                             const QByteArray& templateType,
                             QWidget* parent);

private:
    /// Ditto, with extraNativeMimeTypes added
    static ReturnType choose(const KComponentData &instance, QString &file,
                             const QByteArray &format,
                             const QString &nativeName,
                             const QStringList& extraNativeMimeTypes,
                             const DialogType &dialogType,
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
     * @param instance the KComponentData of your app
     * @param format is the mimetype of the app (e.g. application/x-kspread)
     * @param nativeName is the name of your app (e.g KSpread)
     * @param dialogType the type of the dialog
     * @param templateType the template type of your application (see kword or
     *        kpresenter for details)
     *
     * @return The return type (see above)
     */
    KoTemplateChooseDia(QWidget *parent, const char *name, const KComponentData &instance,
                        const QByteArray &format,
                        const QString &nativeName,
                        const QStringList &extraNativeMimeTypes,
                        const DialogType &dialogType,
                        const QByteArray& templateType="");

private:
    KoTemplateChooseDiaPrivate * const d;

    QString descriptionText(const QString &name, const QString &description);
    void setupDialog();
    void setupTemplateDialog(QWidget * widgetbase, QGridLayout * layout);
    bool collectInfo();

private slots:

    void chosen(QListWidgetItem *);
    void currentChanged( QListWidgetItem * );
    void recentSelected( QListWidgetItem * );
};

#endif

