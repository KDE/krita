/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2005 Martin Ellis <martin.ellis@kdemail.net>

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

#ifndef paragdia_h
#define paragdia_h

#include <kpagedialog.h>
#include <QStringList>
#include <q3ptrlist.h>
#include <q3stylesheet.h>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <KoRuler.h>
#include <KoUnit.h>
#include <KoUnitWidgets.h>
#include <q3dict.h>
#include <QLineEdit>
#include "KoParagLayout.h"
#include "KoParagCounter.h"
#include <knuminput.h>
#include <q3groupbox.h>
#include <koffice_export.h>

class QListWidget;
class KoTextZoomHandler;
class KButtonBox;
class KPushButton;
class KColorButton;
class KoTextDocument;
class KoBorderPreview;
class KoStylePreview;
class KPagePreview2;
class KPagePreview;
class KoSpinBox;
class Q3ButtonGroup;
class QCheckBox;
class QComboBox;
class Q3GroupBox;
class QLabel;
class Q3ListBox;
class QPushButton;
class QRadioButton;
class QWidget;
class Q3WidgetStack;
class KIntNumInput;
class KDoubleNumInput;
class KComboBox;
class Q3VBoxLayout;
class KoUnitDoubleSpinBox;
class KoParagDecorationTab; // From KoParagDecorationTab.ui

/**
 * This is the base class for any widget [usually used in a tab]
 * that handles modifying a part of a KoParagLayout.
 * Used by the paragraph dialog (this file) and by the style editor.
 */
class KOTEXT_EXPORT KoParagLayoutWidget : public QWidget
{
    Q_OBJECT
public:
    // Constructor: flag (PD_SOMETHING) and parent widget
    KoParagLayoutWidget( int flag, QWidget * parent )
        : QWidget( parent ), m_flag( flag )
    {
    }
    virtual ~KoParagLayoutWidget() {}

    // Display settings, from the paragLayout
    virtual void display( const KoParagLayout & lay ) = 0;

    // Save the settings, into the paragLayout
    // This is only used by the stylist, not by paragdia (which needs undo/redo, applying partially etc.)
    virtual void save( KoParagLayout & lay ) = 0;

    // Return true if the settings where modified
    // ## maybe too global, but how to do it differently? We'll see if we need this.
    //virtual bool isModified() = 0;

    /**  return the (i18n-ed) name of the tab */
    virtual QString tabName() = 0;

    // Return the part of the paraglayout that this widget cares about
    int flag() const { return m_flag; }

private:
    int m_flag;
};

/**
 * The widget for editing idents and spacings (tab 1)
 */
class KOTEXT_EXPORT KoIndentSpacingWidget : public KoParagLayoutWidget
{
    Q_OBJECT
public:
    KoIndentSpacingWidget( KoUnit::Unit unit, double _frameWidth, QWidget * parent );
    virtual ~KoIndentSpacingWidget() {}

    virtual void display( const KoParagLayout & lay );
    virtual void save( KoParagLayout & lay );
    //virtual bool isModified();
    virtual QString tabName();

    double leftIndent() const;
    double rightIndent() const;
    double firstLineIndent() const;
    double spaceBeforeParag() const;
    double spaceAfterParag() const;
    double lineSpacing() const;
    KoParagLayout::SpacingType lineSpacingType() const;
private slots:
    void leftChanged( double );
    void rightChanged( double );
    void firstChanged( double );
    void spacingActivated( int );
    void spacingChanged( double );
    void spacingChanged( int );
    void beforeChanged( double );
    void afterChanged( double );
private:
    void updateLineSpacing( KoParagLayout::SpacingType _type );

    Q3WidgetStack *sSpacingStack;
    KIntNumInput *eSpacingPercent;
    KoUnitDoubleSpinBox *eBefore, *eAfter, *eSpacing;
    KoUnitDoubleSpinBox *eLeft, *eRight, *eFirstLine;
    QComboBox *cSpacing;
    KPagePreview *prev1;
    KoUnit::Unit m_unit;
};

/**
 * The widget for editing paragraph alignment (tab 2)
 */
class KOTEXT_EXPORT KoParagAlignWidget : public KoParagLayoutWidget
{
    Q_OBJECT
public:
    KoParagAlignWidget( bool breakLine, QWidget * parent );
    virtual ~KoParagAlignWidget() {}

    virtual void display( const KoParagLayout & lay );
    virtual void save( KoParagLayout & lay );
    //virtual bool isModified();
    virtual QString tabName();

    int pageBreaking() const;
    int align() const;

protected slots:
    void alignLeft();
    void alignCenter();
    void alignRight();
    void alignJustify();

protected:
    void clearAligns();

private:
    QRadioButton *rLeft, *rCenter, *rRight, *rJustify;
    QCheckBox *cKeepLinesTogether, *cHardBreakBefore, *cHardBreakAfter;
    KPagePreview2 *prev2;
};

/**
 * The widget for editing paragraph borders (tab 3)
 */
class KOTEXT_EXPORT KoParagDecorationWidget : public KoParagLayoutWidget
{
    Q_OBJECT
public:
    KoParagDecorationWidget( QWidget * parent );
    virtual ~KoParagDecorationWidget() {}

    virtual void display( const KoParagLayout & lay );
    virtual void save( KoParagLayout & lay );
    //virtual bool isModified();
    virtual QString tabName();

    // Borders
    KoBorder leftBorder() const { return m_leftBorder; }
    KoBorder rightBorder() const { return m_rightBorder; }
    KoBorder topBorder() const { return m_topBorder; }
    KoBorder bottomBorder() const { return m_bottomBorder; }
    bool joinBorder() const { return m_joinBorder; }

    // Background color
    QColor backgroundColor() const;

protected slots:
    void brdLeftToggled( bool );
    void brdRightToggled( bool );
    void brdTopToggled( bool );
    void brdBottomToggled( bool );
    void brdJoinToggled( bool );
    void slotPressEvent(QMouseEvent *_ev);

protected:
    void updateBorders();

private:
    // GUI
    KoBorderPreview *wPreview;
    //! Widget defined in .ui file
    KoParagDecorationTab *wDeco;
    //! Get the currently selected border style
    KoBorder::BorderStyle curBorderStyle() const;
    //! Get the currently selected border width
    unsigned int curBorderWidth() const;
    //! Get the currently selected border color
    QColor curBorderColor() const;

    // Borders
    KoBorder m_leftBorder, m_rightBorder, m_topBorder, m_bottomBorder;
    bool m_joinBorder;

    /**
     * Update a border that has been clicked in the border preview.
     *
     * If there is no border already at the clicked @p position, then
     * create a border at that position using the selected width,color and
     * style.
     *
     * If the clicked border is already on, and it is different to the
     * selected width, color or style, then update it to match the
     * current selection.  If it's the same, then remove the border.
     *
     * Used by slotPressEvent.
     * @param border the border to be updated
     * @param position the position of the border to be updated
     * @param corresponding the toggle button corresponding to the border
     */
    void clickedBorderPreview( KoBorder& border, KoBorder::BorderType position,
			       KPushButton *corresponding );

    //! Set a given border according to the values selected in the GUI
    void updateBorder( KoBorder& border );
    //! Check whether a border is the same as that selected in the GUI
    bool borderChanged( const KoBorder& border );

};


class KOTEXT_EXPORT KoCounterStyleWidget : public QWidget
{
    Q_OBJECT
public:
    KoCounterStyleWidget( bool displayDepth= true, bool onlyStyleTypeLetter = false, bool disableAll=false, QWidget* parent = 0 );

    class StyleRepresenter {
        public:
            StyleRepresenter (const QString name, KoParagCounter::Style style, bool bullet=false) {
                m_name=name;
                m_style=style;
                m_bullet=bullet;
            }
            QString name() const { return m_name; }
            KoParagCounter::Style style() const { return m_style; }
            bool isBullet() const { return m_bullet; }

        private:
            QString m_name;
            KoParagCounter::Style m_style;
            bool m_bullet;
    };

    static void makeCounterRepresenterList( Q3PtrList<StyleRepresenter>& stylesList , bool onlyStyleTypeLetter = false );
    void fillStyleCombo(KoParagCounter::Numbering type = KoParagCounter::NUM_LIST);
    void display( const KoParagLayout & lay );
    void changeKWSpinboxType(KoParagCounter::Style st);
    const KoParagCounter & counter() const { return m_counter; }
    void setCounter( const KoParagCounter& counter );

public slots:
    void numTypeChanged( int nType );

signals:
    void sig_startChanged( int );
    void sig_restartChanged(bool);
    void sig_depthChanged(int);
    void sig_displayLevelsChanged(int);
    void sig_suffixChanged(const QString &);
    void sig_prefixChanged(const QString &);
    void sig_numTypeChanged( int );
    void sig_alignmentChanged( int );
    void changeCustomBullet( const QString & , QChar );
    void changeStyle( KoParagCounter::Style );
protected slots:
    void startChanged(int i) {m_counter.setStartNumber(i);emit sig_startChanged(i);}
    void restartChanged(bool b) {m_counter.setRestartCounter(b);emit sig_restartChanged(b);}
    void depthChanged(int i) {m_counter.setDepth(i);emit sig_depthChanged(i);}
    void displayLevelsChanged(int i) {m_counter.setDisplayLevels(i);emit sig_displayLevelsChanged(i);}
    void alignmentChanged(const QString& s);
    void suffixChanged(const QString & txt) {m_counter.setSuffix(txt);emit sig_suffixChanged(txt); }
    void prefixChanged(const QString & txt) {m_counter.setPrefix(txt);emit sig_prefixChanged(txt); }

    void numStyleChanged();
    void selectCustomBullet();

protected:
    void displayStyle( KoParagCounter::Style style );

private:
    Q3GroupBox *gStyle;
    Q3PtrList <StyleRepresenter> stylesList;
    QListWidget *lstStyle;
    KoParagCounter m_counter;
    QLineEdit *sSuffix, *sPrefix;
    QPushButton *bCustom;
    KoSpinBox *spnStart;
    QSpinBox *spnDepth;
    QSpinBox *spnDisplayLevels;
    QLabel *lStart;
    QLabel *lCustom;
    QCheckBox *cbRestart;
    KComboBox *cbAlignment;
    QLabel *lAlignment;
    unsigned int styleBuffer;
    bool noSignals;
};

/**
 * The widget for editing counters (bullets & numbering) (tab 4)
 */
class KOTEXT_EXPORT KoParagCounterWidget : public KoParagLayoutWidget
{
    Q_OBJECT
public:

    KoParagCounterWidget( bool disableAll=false, QWidget * parent = 0 );
    virtual ~KoParagCounterWidget() {}

    virtual void display( const KoParagLayout & lay );
    virtual void save( KoParagLayout & lay );
    //virtual bool isModified();
    virtual QString tabName();

    const KoParagCounter & counter() const { return m_counter; }

protected slots:
    //void selectCustomBullet();
    //void numStyleChanged(); // selected another style from the combobox
    void numTypeChanged( int );  // selected another type radiobutton.

    void suffixChanged(const QString & txt) {m_counter.setSuffix(txt); updatePreview(); }
    void prefixChanged(const QString & txt) {m_counter.setPrefix(txt); updatePreview();}
    void startChanged(int i) {m_counter.setStartNumber(i); updatePreview();}
    void restartChanged(bool b) {m_counter.setRestartCounter(b); }
    void depthChanged(int i) {m_counter.setDepth(i); updatePreview();}
    void displayLevelsChanged(int i) {m_counter.setDisplayLevels(i); updatePreview();}
    void alignmentChanged(int i) {m_counter.setAlignment(i); updatePreview();}
    void slotChangeCustomBullet( const QString & f, QChar c);
    void styleChanged (KoParagCounter::Style st );

private:
    void updatePreview();

    Q3ButtonGroup *gNumbering;
    KoParagCounter m_counter;
    KoStylePreview *preview;
    KoCounterStyleWidget *m_styleWidget;
    unsigned int styleBuffer;
    bool noSignals;
};

/**
 *
 */
class KoTabulatorsLineEdit : public KoUnitDoubleSpinBox
{
    Q_OBJECT
public:
    KoTabulatorsLineEdit( QWidget *parent, double lower, double upper, double step, double value = 0.0,
                         KoUnit::Unit unit = KoUnit::U_PT, unsigned int precision = 2 );

protected:
    virtual void keyPressEvent ( QKeyEvent * );
 signals:
    void keyReturnPressed();
};

/**
 * The widget for editing tabulators (tab 5)
 */
class KOTEXT_EXPORT KoParagTabulatorsWidget : public KoParagLayoutWidget
{
    Q_OBJECT
public:
    KoParagTabulatorsWidget( KoUnit::Unit unit, double _frameWidth, QWidget * parent );
    virtual ~KoParagTabulatorsWidget() {}

    virtual void display( const KoParagLayout & lay );
    virtual void save( KoParagLayout & lay );
    virtual QString tabName();

    KoTabulatorList tabList() const { return m_tabList; }

    void setCurrentTab( double tabPos );

protected slots:
    void slotTabValueChanged( double );
    void slotAlignCharChanged( const QString &_text );
    void newClicked();
    void deleteClicked();
    void deleteAllClicked();
    void setActiveItem(int selected);
    void updateAlign(int selected);
    void updateFilling(int selected);
    void updateWidth();

private:

    void sortLists();
    QString tabToString(const KoTabulator &tab);

    Q3VBoxLayout* editLayout;

    Q3ListBox* lstTabs;
    Q3GroupBox* gPosition;
    KoTabulatorsLineEdit* sTabPos;
    Q3ButtonGroup* bgAlign;
    QRadioButton* rAlignLeft;
    QRadioButton* rAlignCenter;
    QRadioButton* rAlignRight;
    QRadioButton* rAlignVar;
    QLineEdit* sAlignChar;
    Q3GroupBox* gTabLeader;
    QComboBox* cFilling;
    KoUnitDoubleSpinBox* eWidth;
    QPushButton* bNew;
    QPushButton* bDelete;
    QPushButton* bDeleteAll;

    KoTabulatorList m_tabList;
    KoUnit::Unit m_unit;
    double m_toplimit;
    bool noSignals;
};

/**
 * KoStylePreview. Previewing text with style :)
 * Used in the parag bullet/number tab of the parag dia,
 * and in the main tab of the stylist.
 */
class KoStylePreview : public Q3GroupBox
{
    Q_OBJECT

public:
    KoStylePreview( const QString &title, const QString &text, QWidget *parent );
    virtual ~KoStylePreview();

    /** Apply the given @p style to the preview.
     * Note that this overwrites anything done by setCounter. */
    void setStyle( KoParagStyle *style );

    /** Set the given @p counter to the preview. */
    void setCounter( const KoParagCounter & counter );

protected:
    void drawContents( QPainter *painter );

    KoTextDocument *m_textdoc;
    KoTextZoomHandler *m_zoomHandler;
};


/**
 * The complete(*) dialog for changing attributes of a paragraph
 *
 * (*) the flags (to only show parts of it) have been kept just in case
 * but are not currently used.
 */
class KOTEXT_EXPORT KoParagDia : public KPageDialog
{
    Q_OBJECT

public:
    enum { PD_SPACING = 1, PD_ALIGN = 2, PD_DECORATION = 4, PD_NUMBERING = 8,
           PD_TABS = 16 };

    /**
     * breakLine : kpresenter didn't used this attibute, kword use it.
     */
    KoParagDia( QWidget*, int flags, KoUnit::Unit unit, double _frameWidth=-1,bool breakLine=true, bool disableAll = false);
    ~KoParagDia();

    /** Flags passed to constructor */
    int getFlags()const { return m_flags; }

    /** Make a given page the current one - @p page is a flag (PD_something) value */
    void setCurrentPage( int page );

    /** Set the values to be displayed */
    void setParagLayout( const KoParagLayout & lay );

    // Get values (in pt) - tab 1
    double leftIndent() const { return m_indentSpacingWidget->leftIndent(); }
    double rightIndent() const { return m_indentSpacingWidget->rightIndent(); }
    double firstLineIndent() const { return m_indentSpacingWidget->firstLineIndent(); }
    double spaceBeforeParag() const { return m_indentSpacingWidget->spaceBeforeParag(); }
    double spaceAfterParag() const { return m_indentSpacingWidget->spaceAfterParag(); }
    double lineSpacing() const { return m_indentSpacingWidget->lineSpacing(); }
    KoParagLayout::SpacingType lineSpacingType() const{ return m_indentSpacingWidget->lineSpacingType(); }

    // tab 2
    int align() const { return m_alignWidget->align(); }
    int pageBreaking() const { return m_alignWidget->pageBreaking(); }

    // tab 3
    KoBorder leftBorder() const { return m_decorationsWidget->leftBorder(); }
    KoBorder rightBorder() const { return m_decorationsWidget->rightBorder(); }
    KoBorder topBorder() const { return m_decorationsWidget->topBorder(); }
    KoBorder bottomBorder() const { return m_decorationsWidget->bottomBorder(); }
    bool joinBorder() const { return m_decorationsWidget->joinBorder(); }

    // tab 4
    const KoParagCounter & counter() const { return m_counterWidget->counter(); }

    // tab 5
    KoTabulatorList tabListTabulator() const { return m_tabulatorsWidget->tabList(); }
    KoParagTabulatorsWidget * tabulatorsWidget() const { return m_tabulatorsWidget; }

    QColor backgroundColor() const { return m_decorationsWidget->backgroundColor(); }

    // Support for "what has changed?"
    bool isAlignChanged() const {return oldLayout.alignment!=align();}
    bool isLineSpacingChanged() const {
	return (oldLayout.lineSpacingValue() !=lineSpacing() ||
                oldLayout.lineSpacingType != lineSpacingType());
    }
    bool isLeftMarginChanged() const {
	return oldLayout.margins[Q3StyleSheetItem::MarginLeft]!=leftIndent();
    }
    bool isRightMarginChanged() const {
	return oldLayout.margins[Q3StyleSheetItem::MarginRight]!=rightIndent();
    }
    bool isFirstLineChanged() const {
	return oldLayout.margins[ Q3StyleSheetItem::MarginFirstLine]!=firstLineIndent();
    }
    bool isSpaceBeforeChanged() const {
	return oldLayout.margins[Q3StyleSheetItem::MarginTop]!=spaceBeforeParag();
    }
    bool isSpaceAfterChanged() const {
	return oldLayout.margins[Q3StyleSheetItem::MarginBottom]!=spaceAfterParag();
    }
    bool isPageBreakingChanged() const {
	return oldLayout.pageBreaking!=pageBreaking();
    }
    bool isCounterChanged() const;
    bool isBorderChanged() const {
	return (oldLayout.leftBorder!=leftBorder() ||
                oldLayout.rightBorder!=rightBorder() ||
                oldLayout.topBorder!=topBorder() ||
                oldLayout.bottomBorder!=bottomBorder() );
    }
    bool isJoinBorderChanged() const { return oldLayout.joinBorder!=joinBorder(); }
    bool listTabulatorChanged() const {return oldLayout.tabList()!=tabListTabulator();}
    bool isBackgroundColorChanged() const { return oldLayout.backgroundColor != backgroundColor(); }

    KoParagLayout paragLayout() const;
    /// @return the set of flags which were changed
    int changedFlags() const;

protected slots:
    void slotReset();
    virtual void slotOk();
    virtual void slotApply();
signals:
     void applyParagStyle();

private:
    KoIndentSpacingWidget * m_indentSpacingWidget;
    KoParagAlignWidget * m_alignWidget;
    KoParagDecorationWidget * m_decorationsWidget;
    KoParagCounterWidget * m_counterWidget;
    KoParagTabulatorsWidget * m_tabulatorsWidget;
    KPageWidgetItem *m_p1, *m_p2, *m_p3, *m_p4, *m_p5;
    int m_flags;
    KoParagLayout oldLayout;
};

#endif
