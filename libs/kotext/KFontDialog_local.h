/*
    $Id: kfontdialog.h,v 1.71 2004/10/10 10:27:49 bhards Exp $

    Requires the Qt widget libraries, available at no cost at
    http://www.troll.no

    Copyright (C) 1997 Bernd Johannes Wuebben <wuebben@kde.org>
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>
    Copyright (c) 1999 Mario Weilguni <mweilguni@kde.org>

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

/*This local copy has been added to be able to hide the preview pane of the font chooser. A patch has been made in kdelibs since 3.5. This local copy can be disposed of when we require kdelibs 3.5 and higher*/

#ifndef _K_FONT_DIALOG_LOCAL_H_
#define _K_FONT_DIALOG_LOCAL_H_

#include <QLineEdit>
#include <q3button.h>
//Added by qt3to4:
#include <QLabel>
#include <kdialogbase.h>

#include <koffice_export.h>

class QComboBox;
class QCheckBox;
class QFont;
class Q3GroupBox;
class QLabel;
class QStringList;
class KListBox;
class KIntNumInput;
/**
 * @short A font selection widget.
 *
 * While KFontChooser_local as an ordinary widget can be embedded in
 * custom dialogs and therefore is very flexible, in most cases
 * it is preferable to use the convenience functions in
 * KFontDialog_local.
 *
 * @author Preston Brown <pbrown@kde.org>, Bernd Wuebben <wuebben@kde.org>
 * @version $Id: kfontdialog.h,v 1.71 2004/10/10 10:27:49 bhards Exp $
 */
class KOTEXT_EXPORT KFontChooser_local : public QWidget
{
  Q_OBJECT
  Q_PROPERTY( QFont font READ font WRITE setFont )

public:
  /**
   *  @li @p FamilyList - Identifies the family (leftmost) list.
   *  @li @p StyleList -  Identifies the style (center) list.
   *  @li @p SizeList -   Identifies the size (rightmost) list.
   */
  enum FontColumn { FamilyList=0x01, StyleList=0x02, SizeList=0x04};

  /**
   *  @li @p FontDiffFamily - Identifies a requested change in the font family.
   *  @li @p FontDiffStyle -  Identifies a requested change in the font style.
   *  @li @p FontDiffSize -   Identifies a requested change in the font size.
   */
  enum FontDiff { FontDiffFamily=0x01, FontDiffStyle=0x02, FontDiffSize=0x04 };

  /**
   * Constructs a font picker widget.
   * It normally comes up with all font families present on the system; the
   * getFont method below does allow some more fine-tuning of the selection of fonts
   * that will be displayed in the dialog.
   * <p>Consider the following code snippet;
   * \code
   *    QStringList list;
   *    KFontChooser_local::getFontList(list,SmoothScalableFonts);
   *    KFontChooser_local chooseFont = new KFontChooser_local(0, "FontList", false, list);
   * \endcode
   * <p>
   * The above creates a font chooser dialog with only SmoothScaleble fonts.
   *
   * @param parent The parent widget.
   * @param name The widget name.
   * @param onlyFixed Only display fonts which have fixed-width
   *        character sizes.
   * @param fontList A list of fonts to display, in XLFD format.  If
   *        no list is formatted, the internal KDE font list is used.
   *        If that has not been created, X is queried, and all fonts
   *        available on the system are displayed.
   * @param diff Display the difference version dialog.
   *        See KFontDialog_local::getFontDiff().
   * @param makeFrame Draws a frame with titles around the contents.
   * @param visibleListSize The minimum number of visible entries in the
   *        fontlists.
   * @param sizeIsRelativeState If not zero the widget will show a
   *        checkbox where the user may choose whether the font size
   *        is to be interpreted as relative size.
   *        Initial state of this checkbox will be set according to
   *        *sizeIsRelativeState, user choice may be retrieved by
   *        calling sizeIsRelative().
   */
  KFontChooser_local(QWidget *parent = 0L, const char *name = 0L,
	       bool onlyFixed = false,
	       const QStringList &fontList = QStringList(),
	       bool makeFrame = true, int visibleListSize=8,
               bool diff = false, Qt::CheckState *sizeIsRelativeState = 0L );

  /**
   * Destructs the font chooser.
   */
  virtual ~KFontChooser_local();

  /**
   * Enables or disable a font column in the chooser.
   *
   * Use this
   * function if your application does not need or supports all font
   * properties.
   *
   * @param column Specify the columns. An or'ed combination of
   *        @p FamilyList, @p StyleList and @p SizeList is possible.
   * @param state If @p false the columns are disabled.
   */
  void enableColumn( int column, bool state );

  /**
   * Sets the currently selected font in the chooser.
   *
   * @param font The font to select.
   * @param onlyFixed Readjust the font list to display only fixed
   *        width fonts if @p true, or vice-versa.
   */
  void setFont( const QFont &font, bool onlyFixed = false );

  /**
   * @return The bitmask corresponding to the attributes the user
   *         wishes to change.
   */
  int fontDiffFlags();

  /**
   * @return The currently selected font in the chooser.
   */
  QFont font() const { return selFont; }

  /**
   * Sets the color to use in the preview.
   */
  void setColor( const QColor & col );

  /**
   * @return The color currently used in the preview (default: the text
   *         color of the active color group)
   */
  QColor color() const;

  /**
   * Sets the background color to use in the preview.
   */
  void setBackgroundColor( const QColor & col );

  /**
   * @return The background color currently used in the preview (default:
   *         the base color of the active colorgroup)
   */
  QColor backgroundColor() const;

  /**
   * Sets the state of the checkbox indicating whether the font size
   * is to be interpreted as relative size.
   * NOTE: If parameter sizeIsRelative was not set in the constructor
   *       of the widget this setting will be ignored.
   */
  void setSizeIsRelative( Qt::CheckState relative );

  /**
   * @return Whether the font size is to be interpreted as relative size
   *         (default: QButton:Off)
   */
  Qt::CheckState sizeIsRelative() const;


  /**
   * @return The current text in the sample text input area.
   */
  QString sampleText() const { return sampleEdit->text(); }

  /**
   * Sets the sample text.
   *
   * Normally you should not change this
   * text, but it can be better to do this if the default text is
   * too large for the edit area when using the default font of your
   * application.
   *
   * @param text The new sample text. The current will be removed.
   */
  void setSampleText( const QString &text )
  {
    sampleEdit->setText( text );
  }

  /**
   * Shows or hides the sample text box.
   *
   * @param visible Set it to true to show the box, to false to hide it.
   */
  void setSampleBoxVisible( bool visible )
  {
    ( visible ) ? sampleEdit->show() : sampleEdit->hide();
  }

  /**
   * Converts a QFont into the corresponding X Logical Font
   * Description (XLFD).
   *
   * @param theFont The font to convert.
   * @return A string representing the given font in XLFD format.
   */
  static QString getXLFD( const QFont &theFont )
    { return theFont.rawName(); }

  /**
   * The selection criteria for the font families shown in the dialog.
   *  @li @p FixedWidthFont when included only fixed-width fonts are returned.
   *        The fonts where the width of every character is equal.
   *  @li @p ScalableFont when included only scalable fonts are returned;
   *        certain configurations allow bitmap fonts to remain unscaled and
   *        thus these fonts have limited number of sizes.
   *  @li @p SmoothScalableFont when included only return smooth scalable fonts.
   *        this will return only non-bitmap fonts which are scalable to any size requested.
   *        Setting this option to true will mean the "scalable" flag is irrelavant.
   */
  enum FontListCriteria { FixedWidthFonts=0x01, ScalableFonts=0x02, SmoothScalableFonts=0x04 };

  /**
   * Creates a list of font strings.
   *
   * @param list The list is returned here.
   * @param fontListCriteria should contain all the restrictions for font selection as OR-ed values
   *        @see KFontChooser_local::FontListCriteria for the individual values
   */
  static void getFontList( QStringList &list, uint fontListCriteria);

  /**
   *
   * Fill the font family list of the KFontChooser
   *
   * @param list The font list.
   */
  void setFamilyList( QStringList list );

  /**
   * Reimplemented for internal reasons.
   */
  virtual QSize sizeHint( void ) const;

signals:
  /**
   * Emitted whenever the selected font changes.
   */
  void fontSelected( const QFont &font );

private slots:
  void toggled_checkbox();
  void family_chosen_slot(const QString&);
  void size_chosen_slot(const QString&);
  void style_chosen_slot(const QString&);
  void displaySample(const QFont &font);
  void showXLFDArea(bool);
  void size_value_slot(int);
private:
  void fillFamilyListBox(bool onlyFixedFonts = false);
  void fillSizeList();
  // This one must be static since getFontList( QStringList, char*) is so
  static void addFont( QStringList &list, const char *xfont );

  void setupDisplay();

  // pointer to an optinally supplied list of fonts to
  // inserted into the fontdialog font-family combo-box
  QStringList  fontList;

  KIntNumInput *sizeOfFont;

  QLineEdit    *sampleEdit;
  QLineEdit    *xlfdEdit;

  QLabel       *familyLabel;
  QLabel       *styleLabel;
  QCheckBox    *familyCheckbox;
  QCheckBox    *styleCheckbox;
  QCheckBox    *sizeCheckbox;
  QLabel       *sizeLabel;
  KListBox     *familyListBox;
  KListBox     *styleListBox;
  KListBox     *sizeListBox;
  QComboBox    *charsetsCombo; // BIC: remove in KDE4
  QCheckBox    *sizeIsRelativeCheckBox;

  QFont        selFont;

  QString      selectedFamily;
  QString      selectedStyle;
  int          selectedSize;
  QMap<QString, QString> currentStyles;

  bool usingFixed;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KFontChooser_localPrivate;
  KFontChooser_localPrivate *d;
};

/**
 * @short A font selection dialog.
 *
 * The KFontDialog_local provides a dialog for interactive font selection.
 * It is basically a thin wrapper around the KFontChooser_local widget,
 * which can also be used standalone. In most cases, the simplest
 * use of this class is the static method KFontDialog_local::getFont(),
 * which pops up the dialog, allows the user to select a font, and
 * returns when the dialog is closed.
 *
 * Example:
 *
 * \code
 *      QFont myFont;
 *      int result = KFontDialog_local::getFont( myFont );
 *      if ( result == KFontDialog_local::Accepted )
 *            ...
 * \endcode
 *
 * \image html kfontdialog.png "KDE Font Dialog"
 *
 * @author Preston Brown <pbrown@kde.org>, Bernd Wuebben <wuebben@kde.org>
 * @version $Id: kfontdialog.h,v 1.71 2004/10/10 10:27:49 bhards Exp $
 */
class KOTEXT_EXPORT KFontDialog_local : public KDialogBase  {
    Q_OBJECT

public:
  /**
   * Constructs a font selection dialog.
   *
   * @param parent The parent widget of the dialog, if any.
   * @param name The name of the dialog.
   * @param modal Specifies whether the dialog is modal or not.
   * @param onlyFixed only display fonts which have fixed-width
   *        character sizes.
   * @param fontlist a list of fonts to display, in XLFD format.  If
   *        no list is formatted, the internal KDE font list is used.
   *        If that has not been created, X is queried, and all fonts
   *        available on the system are displayed.
   * @param makeFrame Draws a frame with titles around the contents.
   * @param diff Display the difference version dialog. See getFontDiff().
   * @param sizeIsRelativeState If not zero the widget will show a
   *        checkbox where the user may choose whether the font size
   *        is to be interpreted as relative size.
   *        Initial state of this checkbox will be set according to
   *        *sizeIsRelativeState, user choice may be retrieved by
   *        calling sizeIsRelative().
   *
   */
  KFontDialog_local( QWidget *parent = 0L, const char *name = 0,
	       bool onlyFixed = false, bool modal = false,
	       const QStringList &fontlist = QStringList(),
	       bool makeFrame = true, bool diff = false,
               Qt::CheckState *sizeIsRelativeState = 0L );

  /**
   * Sets the currently selected font in the dialog.
   *
   * @param font The font to select.
   * @param onlyFixed readjust the font list to display only fixed
   *        width fonts if true, or vice-versa
   */
  void setFont( const QFont &font, bool onlyFixed = false )
    { chooser->setFont(font, onlyFixed); }

  /**
   * @return The currently selected font in the dialog.
   */
  QFont font() const { return chooser->font(); }

  /**
   * Sets the state of the checkbox indicating whether the font size
   * is to be interpreted as relative size.
   * NOTE: If parameter sizeIsRelative was not set in the constructor
   *       of the dialog this setting will be ignored.
   */
  void setSizeIsRelative( Qt::CheckState relative )
    { chooser->setSizeIsRelative( relative ); }

  /**
   * @return Whether the font size is to be interpreted as relative size
   *         (default: false)
   */
  Qt::CheckState sizeIsRelative() const
    { return chooser->sizeIsRelative(); }

  /**
   * Creates a modal font dialog, lets the user choose a font,
   * and returns when the dialog is closed.
   *
   * @param theFont a reference to the font to write the chosen font
   *        into.
   * @param onlyFixed if true, only select from fixed-width fonts.
   * @param parent Parent widget of the dialog. Specifying a widget different
   *        from 0 (Null) improves centering (looks better).
   * @param makeFrame Draws a frame with titles around the contents.
   * @param sizeIsRelativeState If not zero the widget will show a
   *        checkbox where the user may choose whether the font size
   *        is to be interpreted as relative size.
   *        Initial state of this checkbox will be set according to
   *        *sizeIsRelativeState and user choice will be returned
   *        therein.
   *
   * @return QDialog::result().
   */
  static int getFont( QFont &theFont, bool onlyFixed = false,
		      QWidget *parent = 0L, bool makeFrame = true,
                      Qt::CheckState *sizeIsRelativeState = 0L );

  /**
   * Creates a modal font difference dialog, lets the user choose a selection
   * of changes that should be made to a set of fonts, and returns when the
   * dialog is closed. Useful for choosing slight adjustments to the font set
   * when the user would otherwise have to manually edit a number of fonts.
   *
   * @param theFont a reference to the font to write the chosen font
   *        into.
   * @param diffFlags a reference to the int into which the chosen
   *        difference selection bitmask should be written.
   *        Check the returned bitmask like:
   *        \code
   *        if ( diffFlags & KFontChooser_local::FontDiffFamily )
   *            [...]
   *        if ( diffFlags & KFontChooser_local::FontDiffStyle )
   *            [...]
   *        if ( diffFlags & KFontChooser_local::FontDiffSize )
   *            [...]
   *        \endcode
   * @param onlyFixed if true, only select from fixed-width fonts.
   * @param parent Parent widget of the dialog. Specifying a widget different
   *        from 0 (Null) improves centering (looks better).
   * @param makeFrame Draws a frame with titles around the contents.
   * @param sizeIsRelativeState If not zero the widget will show a
   *        checkbox where the user may choose whether the font size
   *        is to be interpreted as relative size.
   *        Initial state of this checkbox will be set according to
   *        *sizeIsRelativeState and user choice will be returned
   *        therein.
   *
   * @returns QDialog::result().
   */
  static int getFontDiff( QFont &theFont, int &diffFlags, bool onlyFixed = false,
		      QWidget *parent = 0L, bool makeFrame = true,
                      Qt::CheckState *sizeIsRelativeState = 0L );

  /**
   * When you are not only interested in the font selected, but also
   * in the example string typed in, you can call this method.
   *
   * @param theFont a reference to the font to write the chosen font
   *        into.
   * @param theString a reference to the example text that was typed.
   * @param onlyFixed if true, only select from fixed-width fonts.
   * @param parent Parent widget of the dialog. Specifying a widget different
   *        from 0 (Null) improves centering (looks better).
   * @param makeFrame Draws a frame with titles around the contents.
   * @param sizeIsRelativeState If not zero the widget will show a
   *        checkbox where the user may choose whether the font size
   *        is to be interpreted as relative size.
   *        Initial state of this checkbox will be set according to
   *        *sizeIsRelativeState and user choice will be returned
   *        therein.
   * @return The result of the dialog.
   */
  static int getFontAndText( QFont &theFont, QString &theString,
			     bool onlyFixed = false, QWidget *parent = 0L,
			     bool makeFrame = true,
                             Qt::CheckState *sizeIsRelativeState = 0L );

signals:
  /**
   * Emitted whenever the currently selected font changes.
   * Connect to this to monitor the font as it is selected if you are
   * not running modal.
   */
  void fontSelected( const QFont &font );

protected:
  KFontChooser_local *chooser;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KFontDialog_localPrivate;
  KFontDialog_localPrivate *d;

};

#endif
