/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef kostylist_h
#define kostylist_h

#include <kdialogbase.h>
#include <QStringList>
//Added by qt3to4:
#include <Q3GridLayout>
#include <QResizeEvent>

#include <KoParagDia.h>
#include <KoFontTab.h>
#include <KoDecorationTab.h>
#include <KoHighlightingTab.h>
#include <KoLayoutTab.h>
#include <KoLanguageTab.h>
#include <KoUnit.h>
#include <q3ptrlist.h>
#include <KoStyleCollection.h>

//class KoFontChooser;
class KoParagStyle;
class KoStyleEditor;
class KoStyleManagerTab;
class QCheckBox;
class QComboBox;
class QListWidget;
class Q3GridLayout;
class QLineEdit;
class QPushButton;
class QTabWidget;
class QWidget;
class KoTextDocument;
class KoStyleManagerPrivate;

/**
 * This class provides a dialog for editing named text styles.
 */
class KOTEXT_EXPORT KoStyleManager : public KDialogBase
{
    Q_OBJECT

public:
    enum { ShowIncludeInToc = 1 }; // bitfield for flags
    KoStyleManager( QWidget *_parent, KoUnit::Unit unit,
                    const KoStyleCollection& styleCollection,
                    const QString & activeStyleName,
                    int flags = 0 );
    virtual ~KoStyleManager();

    virtual KoParagStyle* addStyleTemplate(KoParagStyle *style)=0;
    //virtual void applyStyleChange( KoParagStyle * changedStyle, int paragLayoutChanged, int formatChanged )=0;
    virtual void applyStyleChange( KoStyleChangeDefMap changed )=0;

    virtual void removeStyleTemplate( KoParagStyle *style )=0;
    virtual void updateAllStyleLists()=0;
    virtual void updateStyleListOrder( const QStringList & list)=0;

protected:
    void updateFollowingStyle( KoParagStyle *s );
    void updateInheritStyle( KoParagStyle *s );
    void setupWidget(const KoStyleCollection & styleCollection);
    void addGeneralTab( int flags );
    void apply();
    void updateGUI();
    void updatePreview();
    void save();
    int styleIndex( int pos );

private:
    QTabWidget *m_tabs;
    QListWidget* m_stylesList;
    QLineEdit *m_nameString;
    QComboBox *m_styleCombo;
    QPushButton *m_deleteButton;
    QPushButton *m_newButton;
    QPushButton *m_moveUpButton;
    QPushButton *m_moveDownButton;
    QComboBox *m_inheritCombo;
    KoStyleManagerPrivate *d;

    KoParagStyle *m_currentStyle;
    Q3PtrList<KoParagStyle> m_origStyles;      // internal list of orig styles we have modified
    Q3PtrList<KoParagStyle> m_changedStyles;   // internal list of changed styles.
    Q3PtrList<KoStyleManagerTab> m_tabsList;
    QStringList m_styleOrder;
    int numStyles;
    bool noSignals;

protected slots:
    virtual void slotOk();
    virtual void slotApply();
    void switchStyle();
    void switchTabs();
    void addStyle();
    void deleteStyle();
    void moveUpStyle();
    void moveDownStyle();
    void renameStyle(const QString &);
protected:
    KoParagStyle * style( const QString & _name );
    void addTab( KoStyleManagerTab * tab );
    QString generateUniqueName();
};

class KOTEXT_EXPORT KoStyleManagerTab : public QWidget {
    Q_OBJECT
public:
    KoStyleManagerTab(QWidget *parent) : QWidget(parent) {};

    /** the new style which is to be displayed */
    void setStyle(KoParagStyle *style) { m_style = style; }
    /**  update the GUI from the current Style*/
    virtual void update() = 0;
    /**  return the (i18n-ed) name of the tab */
    virtual QString tabName() = 0;
    /** save the GUI to the style */
    virtual void save() = 0;
protected:
    KoParagStyle *m_style;
};

// A tab to edit parts of the parag-layout of the style
// Acts as a wrapper around KoParagLayoutWidget [which doesn't know about styles].
class KOTEXT_EXPORT KoStyleParagTab : public KoStyleManagerTab
{
    Q_OBJECT
public:
    KoStyleParagTab( QWidget * parent );

    // not a constructor parameter since 'this' is the parent of the widget
    void setWidget( KoParagLayoutWidget * widget );

    virtual void update();
    virtual void save();
    virtual QString tabName() { return m_widget->tabName(); }
protected:
    virtual void resizeEvent( QResizeEvent *e );
private:
    KoParagLayoutWidget * m_widget;
};

// The "font" tab. Maybe we should put the text color at the bottom ?
class KOTEXT_EXPORT KoStyleFontTab : public KoStyleManagerTab
{
    Q_OBJECT
public:
    KoStyleFontTab( QWidget * parent );
    ~KoStyleFontTab();
    virtual void update();
    virtual QString tabName();
    virtual void save();
private:
	KoFontTab *m_fontTab;
	KoDecorationTab *m_decorationTab;
	KoHighlightingTab *m_highlightingTab;
	KoLayoutTab *m_layoutTab;
	KoLanguageTab *m_languageTab;
};

/*
Font            simple font dia
Color           simple color dia
Spacing and Indents     paragraph spacing dia (KWParagDia)
alignments      KoParagDia alignment tab
borders         KoParagDia  borders tab
numbering       KoParagDia  tab numbering
tabulators      KoParagDia  tab tabs */

#endif
