/*

    Requires the Qt widget libraries, available at no cost at
    http://www.troll.no

    Copyright (C) 1996 Bernd Johannes Wuebben  <wuebben@kde.org>
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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <qcombobox.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qfont.h>
#include <q3groupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qscrollbar.h>
#include <qstringlist.h>
#include <qfontdatabase.h>
#include <q3whatsthis.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3ValueList>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include <kapplication.h>
#include <kcharsets.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <qlineedit.h>
#include <klistbox.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <knuminput.h>

#include <koffice_export.h>

#include "KFontDialog_local.h"
#include "KFontDialog_local.moc"

static int minimumListWidth( const Q3ListBox *list )
{
  int w=0;
  for( uint i=0; i<list->count(); i++ )
  {
    int itemWidth = list->item(i)->width(list);
    w = qMax(w,itemWidth);
  }
  if( w == 0 ) { w = 40; }
  w += list->frameWidth() * 2;
  w += list->verticalScrollBar()->sizeHint().width();
  return w;
}

static int minimumListHeight( const Q3ListBox *list, int numVisibleEntry )
{
  int w = list->count() > 0 ? list->item(0)->height(list) :
    list->fontMetrics().lineSpacing();

  if( w < 0 ) { w = 10; }
  if( numVisibleEntry <= 0 ) { numVisibleEntry = 4; }
  return ( w * numVisibleEntry + 2 * list->frameWidth() );
}

class KFontChooser_local::KFontChooser_localPrivate
{
public:
    KFontChooser_localPrivate()
        { m_palette.setColor(QPalette::Active, QColorGroup::Text, Qt::black);
          m_palette.setColor(QPalette::Active, QColorGroup::Base, Qt::white); }
    QPalette m_palette;
};

KFontChooser_local::KFontChooser_local(QWidget *parent, const char *name,
			   bool onlyFixed, const QStringList &fontList,
			   bool makeFrame, int visibleListSize, bool diff,
                           QCheckBox::ToggleState *sizeIsRelativeState )
  : QWidget(parent, name), usingFixed(onlyFixed)
{
  charsetsCombo = 0;

  QString mainWhatsThisText =
    i18n( "Here you can choose the font to be used." );
  Q3WhatsThis::add( this, mainWhatsThisText );

  d = new KFontChooser_localPrivate;
  Q3VBoxLayout *topLayout = new Q3VBoxLayout( this, 0, KDialog::spacingHint() );
  int checkBoxGap = KDialog::spacingHint() / 2;

  QWidget *page;
  Q3GridLayout *gridLayout;
  int row = 0;
  if( makeFrame )
  {
    page = new Q3GroupBox( i18n("Requested Font"), this );
    topLayout->addWidget(page);
    gridLayout = new Q3GridLayout( page, 5, 3, KDialog::marginHint(), KDialog::spacingHint() );
    gridLayout->addRowSpacing( 0, fontMetrics().lineSpacing() );
    row = 1;
  }
  else
  {
    page = new QWidget( this );
    topLayout->addWidget(page);
    gridLayout = new Q3GridLayout( page, 4, 3, 0, KDialog::spacingHint() );
  }

  //
  // first, create the labels across the top
  //
  Q3HBoxLayout *familyLayout = new Q3HBoxLayout();
  familyLayout->addSpacing( checkBoxGap );
  if (diff) {
    familyCheckbox = new QCheckBox(i18n("Font"), page);
    connect(familyCheckbox, SIGNAL(toggled(bool)), SLOT(toggled_checkbox()));
    familyLayout->addWidget(familyCheckbox, 0, Qt::AlignLeft);
    QString familyCBToolTipText =
      i18n("Change font family?");
    QString familyCBWhatsThisText =
      i18n("Enable this checkbox to change the font family settings.");
    Q3WhatsThis::add( familyCheckbox, familyCBWhatsThisText );
    QToolTip::add(   familyCheckbox, familyCBToolTipText );
    familyLabel = 0;
  } else {
    familyCheckbox = 0;
    familyLabel = new QLabel( i18n("Font:"), page, "familyLabel" );
    familyLayout->addWidget(familyLabel, 1, Qt::AlignLeft);
  }
  gridLayout->addLayout(familyLayout, row, 0 );

  Q3HBoxLayout *styleLayout = new Q3HBoxLayout();
  if (diff) {
     styleCheckbox = new QCheckBox(i18n("Font style"), page);
     connect(styleCheckbox, SIGNAL(toggled(bool)), SLOT(toggled_checkbox()));
     styleLayout->addWidget(styleCheckbox, 0, Qt::AlignLeft);
    QString styleCBToolTipText =
      i18n("Change font style?");
    QString styleCBWhatsThisText =
      i18n("Enable this checkbox to change the font style settings.");
    Q3WhatsThis::add( styleCheckbox, styleCBWhatsThisText );
    QToolTip::add(   styleCheckbox, styleCBToolTipText );
    styleLabel = 0;
  } else {
    styleCheckbox = 0;
    styleLabel = new QLabel( i18n("Font style:"), page, "styleLabel");
    styleLayout->addWidget(styleLabel, 1, Qt::AlignLeft);
  }
  styleLayout->addSpacing( checkBoxGap );
  gridLayout->addLayout(styleLayout, row, 1 );

  Q3HBoxLayout *sizeLayout = new Q3HBoxLayout();
  if (diff) {
    sizeCheckbox = new QCheckBox(i18n("Size"),page);
    connect(sizeCheckbox, SIGNAL(toggled(bool)), SLOT(toggled_checkbox()));
    sizeLayout->addWidget(sizeCheckbox, 0, Qt::AlignLeft);
    QString sizeCBToolTipText =
      i18n("Change font size?");
    QString sizeCBWhatsThisText =
      i18n("Enable this checkbox to change the font size settings.");
    Q3WhatsThis::add( sizeCheckbox, sizeCBWhatsThisText );
    QToolTip::add(   sizeCheckbox, sizeCBToolTipText );
    sizeLabel = 0;
  } else {
    sizeCheckbox = 0;
    sizeLabel = new QLabel( i18n("Size:"), page, "sizeLabel");
    sizeLayout->addWidget(sizeLabel, 1, Qt::AlignLeft);
  }
  sizeLayout->addSpacing( checkBoxGap );
  sizeLayout->addSpacing( checkBoxGap ); // prevent label from eating border
  gridLayout->addLayout(sizeLayout, row, 2 );

  row ++;

  //
  // now create the actual boxes that hold the info
  //
  familyListBox = new KListBox( page, "familyListBox");
  familyListBox->setEnabled( !diff );
  gridLayout->addWidget( familyListBox, row, 0 );
  QString fontFamilyWhatsThisText =
    i18n("Here you can choose the font family to be used." );
  Q3WhatsThis::add( familyListBox, fontFamilyWhatsThisText );
  Q3WhatsThis::add(diff?(QWidget *) familyCheckbox:(QWidget *) familyLabel, fontFamilyWhatsThisText );
  connect(familyListBox, SIGNAL(highlighted(const QString &)),
	  SLOT(family_chosen_slot(const QString &)));
  if(!fontList.isEmpty())
  {
    familyListBox->insertStringList(fontList);
  }
  else
  {
    fillFamilyListBox(onlyFixed);
  }

  familyListBox->setMinimumWidth( minimumListWidth( familyListBox ) );
  familyListBox->setMinimumHeight(
    minimumListHeight( familyListBox, visibleListSize  ) );

  styleListBox = new KListBox( page, "styleListBox");
  styleListBox->setEnabled( !diff );
  gridLayout->addWidget(styleListBox, row, 1);
  QString fontStyleWhatsThisText =
    i18n("Here you can choose the font style to be used." );
  Q3WhatsThis::add( styleListBox, fontStyleWhatsThisText );
  Q3WhatsThis::add(diff?(QWidget *)styleCheckbox:(QWidget *)styleLabel, fontFamilyWhatsThisText );
  styleListBox->insertItem(i18n("Regular"));
  styleListBox->insertItem(i18n("Italic"));
  styleListBox->insertItem(i18n("Bold"));
  styleListBox->insertItem(i18n("Bold Italic"));
  styleListBox->setMinimumWidth( minimumListWidth( styleListBox ) );
  styleListBox->setMinimumHeight(
    minimumListHeight( styleListBox, visibleListSize  ) );

  connect(styleListBox, SIGNAL(highlighted(const QString &)),
	  SLOT(style_chosen_slot(const QString &)));


  sizeListBox = new KListBox( page, "sizeListBox");
  sizeOfFont = new KIntNumInput( page );
  sizeOfFont->setObjectName("sizeOfFont");
  sizeOfFont->setMinimum(4);

  sizeListBox->setEnabled( !diff );
  sizeOfFont->setEnabled( !diff );
  if( sizeIsRelativeState ) {
    QString sizeIsRelativeCBText =
      i18n("Relative");
    QString sizeIsRelativeCBToolTipText =
      i18n("Font size<br><i>fixed</i> or <i>relative</i><br>to environment");
    QString sizeIsRelativeCBWhatsThisText =
      i18n("Here you can switch between fixed font size and font size "
           "to be calculated dynamically and adjusted to changing "
           "environment (e.g. widget dimensions, paper size)." );
    sizeIsRelativeCheckBox = new QCheckBox( sizeIsRelativeCBText,
                                            page,
                                           "sizeIsRelativeCheckBox" );
    sizeIsRelativeCheckBox->setTristate( diff );
    Q3GridLayout *sizeLayout2 = new Q3GridLayout( 3,2, KDialog::spacingHint()/2, "sizeLayout2" );
    gridLayout->addLayout(sizeLayout2, row, 2);
    sizeLayout2->setColStretch( 1, 1 ); // to prevent text from eating the right border
    sizeLayout2->addMultiCellWidget( sizeOfFont, 0, 0, 0, 1);
    sizeLayout2->addMultiCellWidget(sizeListBox, 1,1, 0,1);
    sizeLayout2->addWidget(sizeIsRelativeCheckBox, 2, 0, Qt::AlignLeft);
    Q3WhatsThis::add( sizeIsRelativeCheckBox, sizeIsRelativeCBWhatsThisText );
    QToolTip::add(   sizeIsRelativeCheckBox, sizeIsRelativeCBToolTipText );
  }
  else {
    sizeIsRelativeCheckBox = 0L;
    Q3GridLayout *sizeLayout2 = new Q3GridLayout( 2,1, KDialog::spacingHint()/2, "sizeLayout2" );
    gridLayout->addLayout(sizeLayout2, row, 2);
    sizeLayout2->addWidget( sizeOfFont, 0, 0);
    sizeLayout2->addMultiCellWidget(sizeListBox, 1,1, 0,0);
  }
  QString fontSizeWhatsThisText =
    i18n("Here you can choose the font size to be used." );
  Q3WhatsThis::add( sizeListBox, fontSizeWhatsThisText );
  Q3WhatsThis::add( diff?(QWidget *)sizeCheckbox:(QWidget *)sizeLabel, fontSizeWhatsThisText );

  fillSizeList();
  sizeListBox->setMinimumWidth( minimumListWidth(sizeListBox) +
    sizeListBox->fontMetrics().maxWidth() );
  sizeListBox->setMinimumHeight(
    minimumListHeight( sizeListBox, visibleListSize  ) );

  connect( sizeOfFont, SIGNAL( valueChanged(int) ),
           SLOT(size_value_slot(int)));

  connect( sizeListBox, SIGNAL(highlighted(const QString&)),
	   SLOT(size_chosen_slot(const QString&)) );
  sizeListBox->setSelected(sizeListBox->findItem(QString::number(10)), true); // default to 10pt.

  row ++;

  row ++;
  sampleEdit = new QLineEdit( page, "sampleEdit");
  QFont tmpFont( KGlobalSettings::generalFont().family(), 64, QFont::Black );
  sampleEdit->setFont(tmpFont);
  sampleEdit->setText(i18n("The Quick Brown Fox Jumps Over The Lazy Dog"));
  sampleEdit->setMinimumHeight( sampleEdit->fontMetrics().lineSpacing() );
  sampleEdit->setAlignment(Qt::AlignCenter);
  gridLayout->addMultiCellWidget(sampleEdit, 4, 4, 0, 2);
  QString sampleEditWhatsThisText =
    i18n("This sample text illustrates the current settings. "
         "You may edit it to test special characters." );
  Q3WhatsThis::add( sampleEdit, sampleEditWhatsThisText );
  connect(this, SIGNAL(fontSelected(const QFont &)),
	  SLOT(displaySample(const QFont &)));

  Q3VBoxLayout *vbox;
  if( makeFrame )
  {
    page = new Q3GroupBox( i18n("Actual Font"), this );
    topLayout->addWidget(page);
    vbox = new Q3VBoxLayout( page, KDialog::spacingHint() );
    vbox->addSpacing( fontMetrics().lineSpacing() );
  }
  else
  {
    page = new QWidget( this );
    topLayout->addWidget(page);
    vbox = new Q3VBoxLayout( page, 0, KDialog::spacingHint() );
    QLabel *label = new QLabel( i18n("Actual Font"), page );
    vbox->addWidget( label );
  }

  xlfdEdit = new QLineEdit( page, "xlfdEdit" );
  vbox->addWidget( xlfdEdit );

  // lets initialize the display if possible
  setFont( KGlobalSettings::generalFont(), usingFixed );
  // check or uncheck or gray out the "relative" checkbox
  if( sizeIsRelativeState && sizeIsRelativeCheckBox )
    setSizeIsRelative( *sizeIsRelativeState );

  KConfig *config = KGlobal::config();
  KConfigGroup group(config, QString::fromLatin1("General"));
  showXLFDArea(group.readBoolEntry(QString::fromLatin1("fontSelectorShowXLFD"), false));
}

KFontChooser_local::~KFontChooser_local()
{
  delete d;
}

void KFontChooser_local::fillSizeList() {
  if(! sizeListBox) return; //assertion.

  static const int c[] =
  {
    4,  5,  6,  7,
    8,  9,  10, 11,
    12, 13, 14, 15,
    16, 17, 18, 19,
    20, 22, 24, 26,
    28, 32, 48, 64,
    0
  };
  for(int i = 0; c[i]; ++i)
  {
    sizeListBox->insertItem(QString::number(c[i]));
  }
}

void KFontChooser_local::setColor( const QColor & col )
{
  d->m_palette.setColor( QPalette::Active, QColorGroup::Text, col );
  QPalette pal = sampleEdit->palette();
  pal.setColor( QPalette::Active, QColorGroup::Text, col );
  sampleEdit->setPalette( pal );
}

QColor KFontChooser_local::color() const
{
  return d->m_palette.color( QPalette::Active, QColorGroup::Text );
}

void KFontChooser_local::setBackgroundColor( const QColor & col )
{
  d->m_palette.setColor( QPalette::Active, QColorGroup::Base, col );
  QPalette pal = sampleEdit->palette();
  pal.setColor( QPalette::Active, QColorGroup::Base, col );
  sampleEdit->setPalette( pal );
}

QColor KFontChooser_local::backgroundColor() const
{
  return d->m_palette.color( QPalette::Active, QColorGroup::Base );
}

void KFontChooser_local::setSizeIsRelative( QCheckBox::ToggleState relative )
{
  // check or uncheck or gray out the "relative" checkbox
  if( sizeIsRelativeCheckBox ) {
    if( QCheckBox::NoChange == relative )
      sizeIsRelativeCheckBox->setNoChange();
    else
      sizeIsRelativeCheckBox->setChecked(  QCheckBox::On == relative );
  }
}

QCheckBox::ToggleState KFontChooser_local::sizeIsRelative() const
{
  return sizeIsRelativeCheckBox
       ? sizeIsRelativeCheckBox->state()
       : QCheckBox::NoChange;
}

QSize KFontChooser_local::sizeHint( void ) const
{
  return minimumSizeHint();
}


void KFontChooser_local::enableColumn( int column, bool state )
{
  if( column & FamilyList )
  {
    familyListBox->setEnabled(state);
  }
  if( column & StyleList )
  {
    styleListBox->setEnabled(state);
  }
  if( column & SizeList )
  {
    sizeListBox->setEnabled(state);
  }
}


void KFontChooser_local::setFont( const QFont& aFont, bool onlyFixed )
{
  selFont = aFont;
  selectedSize=aFont.pointSize();
  if (selectedSize == -1)
     selectedSize = QFontInfo(aFont).pointSize();

  if( onlyFixed != usingFixed)
  {
    usingFixed = onlyFixed;
    fillFamilyListBox(usingFixed);
  }
  setupDisplay();
  displaySample(selFont);
}


int KFontChooser_local::fontDiffFlags() {
   int diffFlags = 0;
   if (familyCheckbox && styleCheckbox && sizeCheckbox) {
      diffFlags = (int)(familyCheckbox->isChecked() ? FontDiffFamily : 0)
                | (int)( styleCheckbox->isChecked() ? FontDiffStyle  : 0)
                | (int)(  sizeCheckbox->isChecked() ? FontDiffSize   : 0);
   }
   return diffFlags;
}

void KFontChooser_local::toggled_checkbox()
{
  familyListBox->setEnabled( familyCheckbox->isChecked() );
  styleListBox->setEnabled( styleCheckbox->isChecked() );
  sizeListBox->setEnabled( sizeCheckbox->isChecked() );
  sizeOfFont->setEnabled( sizeCheckbox->isChecked() );
}

void KFontChooser_local::family_chosen_slot(const QString& family)
{
    QString currentFamily;
    if (family.isEmpty())
       currentFamily = familyListBox->currentText();
    else
       currentFamily = family;


    QFontDatabase dbase;
    QStringList styles = QStringList(dbase.styles(currentFamily));
    styleListBox->clear();
    currentStyles.clear();
    for ( QStringList::Iterator it = styles.begin(); it != styles.end(); ++it ) {
        QString style = *it;
        int pos = style.find("Plain");
        if(pos >=0) style = style.replace(pos,5,i18n("Regular"));
        pos = style.find("Normal");
        if(pos >=0) style = style.replace(pos,6,i18n("Regular"));
        pos = style.find("Oblique");
        if(pos >=0) style = style.replace(pos,7,i18n("Italic"));
        if(!styleListBox->findItem(style)) {
            styleListBox->insertItem(i18n(style.toUtf8()));
            currentStyles.insert(i18n(style.toUtf8()), *it);
        }
    }
    if(styleListBox->count()==0) {
        styleListBox->insertItem(i18n("Regular"));
        currentStyles.insert(i18n("Regular"), "Normal");
    }

    styleListBox->blockSignals(true);
    Q3ListBoxItem *item = styleListBox->findItem(selectedStyle);
    if (item)
       styleListBox->setSelected(styleListBox->findItem(selectedStyle), true);
    else
       styleListBox->setSelected(0, true);
    styleListBox->blockSignals(false);

    style_chosen_slot(QString::null);

    if (!family.isEmpty() )
	    selectedFamily = family;

}

void KFontChooser_local::size_chosen_slot(const QString& size){

  selectedSize=size.toInt();
  sizeOfFont->setValue(selectedSize);
  selFont.setPointSize(selectedSize);
  emit fontSelected(selFont);
}

void KFontChooser_local::size_value_slot(int val) {
  selFont.setPointSize(val);
  emit fontSelected(selFont);
}

void KFontChooser_local::style_chosen_slot(const QString& style)
{
    QString currentStyle;
    if (style.isEmpty())
       currentStyle = styleListBox->currentText();
    else
       currentStyle = style;

    int diff=0; // the difference between the font size requested and what we can show.

    sizeListBox->clear();
    QFontDatabase dbase;
    if(dbase.isSmoothlyScalable(familyListBox->currentText(), currentStyles[currentStyle])) {  // is vector font
        //sampleEdit->setPaletteBackgroundPixmap( VectorPixmap ); // TODO
        fillSizeList();
    } else {                                // is bitmap font.
        //sampleEdit->setPaletteBackgroundPixmap( BitmapPixmap ); // TODO
        Q3ValueList<int> sizes = dbase.smoothSizes(familyListBox->currentText(), currentStyles[currentStyle]);
        if(sizes.count() > 0) {
            Q3ValueList<int>::iterator it;
            diff=1000;
            for ( it = sizes.begin(); it != sizes.end(); ++it ) {
                if(*it <= selectedSize || diff > *it - selectedSize) diff = selectedSize - *it;
                sizeListBox->insertItem(QString::number(*it));
            }
        } else // there are times QT does not provide the list..
            fillSizeList();
    }
    sizeListBox->blockSignals(true);
    sizeListBox->setSelected(sizeListBox->findItem(QString::number(selectedSize)), true);
    sizeListBox->blockSignals(false);
    sizeListBox->ensureCurrentVisible();

    //kDebug() << "Showing: " << familyListBox->currentText() << ", " << currentStyles[currentStyle] << ", " << selectedSize-diff << endl;
    selFont = dbase.font(familyListBox->currentText(), currentStyles[currentStyle], selectedSize-diff);
    emit fontSelected(selFont);
    if (!style.isEmpty())
        selectedStyle = style;
}

void KFontChooser_local::displaySample(const QFont& font)
{
  sampleEdit->setFont(font);
  sampleEdit->setCursorPosition(0);
  xlfdEdit->setText(font.rawName());
  xlfdEdit->setCursorPosition(0);

  //QFontInfo a = QFontInfo(font);
  //kDebug() << "font: " << a.family () << ", " << a.pointSize () << endl;
  //kDebug() << "      (" << font.toString() << ")\n";
}

void KFontChooser_local::setupDisplay()
{
  // Calling familyListBox->setCurrentItem() causes the value of selFont
  // to change, so we save the family, style and size beforehand.
  QString family = selFont.family().lower();
  int style = (selFont.bold() ? 2 : 0) + (selFont.italic() ? 1 : 0);
  int size = selFont.pointSize();
  if (size == -1)
     size = QFontInfo(selFont).pointSize();
  QString sizeStr = QString::number(size);

  int numEntries, i;

  numEntries = familyListBox->count();
  for (i = 0; i < numEntries; i++) {
    if (family == familyListBox->text(i).lower()) {
      familyListBox->setCurrentItem(i);
      break;
    }
  }

  // 1st Fallback
  if ( (i == numEntries) )
  {
    if (family.contains('['))
    {
      family = family.left(family.find('[')).trimmed();
      for (i = 0; i < numEntries; i++) {
        if (family == familyListBox->text(i).lower()) {
          familyListBox->setCurrentItem(i);
          break;
        }
      }
    }
  }

  // 2nd Fallback
  if ( (i == numEntries) )
  {
    QString fallback = family+" [";
    for (i = 0; i < numEntries; i++) {
      if (familyListBox->text(i).lower().startsWith(fallback)) {
        familyListBox->setCurrentItem(i);
        break;
      }
    }
  }

  // 3rd Fallback
  if ( (i == numEntries) )
  {
    for (i = 0; i < numEntries; i++) {
      if (familyListBox->text(i).lower().startsWith(family)) {
        familyListBox->setCurrentItem(i);
        break;
      }
    }
  }

  // Fall back in case nothing matched. Otherwise, diff doesn't work
  if ( i == numEntries )
    familyListBox->setCurrentItem( 0 );

  styleListBox->setCurrentItem(style);

  numEntries = sizeListBox->count();
  for (i = 0; i < numEntries; i++){
    if (sizeStr == sizeListBox->text(i)) {
      sizeListBox->setCurrentItem(i);
      break;
    }
  }

  sizeOfFont->setValue(size);
}


void KFontChooser_local::getFontList( QStringList &list, uint fontListCriteria)
{
  QFontDatabase dbase;
  QStringList lstSys(dbase.families());

  // if we have criteria; then check fonts before adding
  if (fontListCriteria)
  {
    QStringList lstFonts;
    for (QStringList::Iterator it = lstSys.begin(); it != lstSys.end(); ++it)
    {
        if ((fontListCriteria & FixedWidthFonts) > 0 && !dbase.isFixedPitch(*it)) continue;
        if (((fontListCriteria & (SmoothScalableFonts | ScalableFonts)) == ScalableFonts) &&
                !dbase.isBitmapScalable(*it)) continue;
        if ((fontListCriteria & SmoothScalableFonts) > 0 && !dbase.isSmoothlyScalable(*it)) continue;
        lstFonts.append(*it);
    }

    if((fontListCriteria & FixedWidthFonts) > 0) {
        // Fallback.. if there are no fixed fonts found, it's probably a
        // bug in the font server or Qt.  In this case, just use 'fixed'
        if (lstFonts.count() == 0)
          lstFonts.append("fixed");
    }

    lstSys = lstFonts;
  }

  lstSys.sort();

  list = lstSys;
}

void KFontChooser_local::addFont( QStringList &list, const char *xfont )
{
  const char *ptr = strchr( xfont, '-' );
  if ( !ptr )
    return;

  ptr = strchr( ptr + 1, '-' );
  if ( !ptr )
    return;

  QString font = QString::fromLatin1(ptr + 1);

  int pos;
  if ( ( pos = font.find( '-' ) ) > 0 ) {
    font.truncate( pos );

    if ( font.find( QString::fromLatin1("open look"), 0, false ) >= 0 )
      return;

    QStringList::Iterator it = list.begin();

    for ( ; it != list.end(); ++it )
      if ( *it == font )
	return;
    list.append( font );
  }
}

void KFontChooser_local::fillFamilyListBox(bool onlyFixedFonts)
{
  QStringList fontList;
  getFontList(fontList, onlyFixedFonts?FixedWidthFonts:0);
  familyListBox->clear();
  familyListBox->insertStringList(fontList);
}

void KFontChooser_local::setFamilyList( QStringList list )
{
  familyListBox->blockSignals( true );
  familyListBox->clear();
  familyListBox->insertStringList( list );
  setFont( KGlobalSettings::generalFont(), usingFixed );
  familyListBox->blockSignals( false );
}

void KFontChooser_local::showXLFDArea(bool show)
{
  if( show )
  {
    xlfdEdit->parentWidget()->show();
  }
  else
  {
    xlfdEdit->parentWidget()->hide();
  }
}

///////////////////////////////////////////////////////////////////////////////

KFontDialog_local::KFontDialog_local( QWidget *parent, const char* name,
			  bool onlyFixed, bool modal,
			  const QStringList &fontList, bool makeFrame, bool diff,
                          QCheckBox::ToggleState *sizeIsRelativeState )
  : KDialogBase( parent, name, modal, i18n("Select Font"), Ok|Cancel, Ok )
{
  chooser = new KFontChooser_local( this, "fontChooser",
                              onlyFixed, fontList, makeFrame, 8,
                              diff, sizeIsRelativeState );
  setMainWidget(chooser);
}


int KFontDialog_local::getFontDiff( QFont &theFont, int &diffFlags, bool onlyFixed,
                             QWidget *parent, bool makeFrame,
                             QCheckBox::ToggleState *sizeIsRelativeState )
{
  KFontDialog_local dlg( parent, "Font Selector", onlyFixed, true, QStringList(),
		   makeFrame, true, sizeIsRelativeState );
  dlg.setFont( theFont, onlyFixed );

  int result = dlg.exec();
  if( result == Accepted )
  {
    theFont = dlg.chooser->font();
    diffFlags = dlg.chooser->fontDiffFlags();
    if( sizeIsRelativeState )
      *sizeIsRelativeState = dlg.chooser->sizeIsRelative();
  }
  return result;
}

int KFontDialog_local::getFont( QFont &theFont, bool onlyFixed,
                          QWidget *parent, bool makeFrame,
                          QCheckBox::ToggleState *sizeIsRelativeState )
{
  KFontDialog_local dlg( parent, "Font Selector", onlyFixed, true, QStringList(),
		   makeFrame, false, sizeIsRelativeState );
  dlg.setFont( theFont, onlyFixed );

  int result = dlg.exec();
  if( result == Accepted )
  {
    theFont = dlg.chooser->font();
    if( sizeIsRelativeState )
      *sizeIsRelativeState = dlg.chooser->sizeIsRelative();
  }
  return result;
}


int KFontDialog_local::getFontAndText( QFont &theFont, QString &theString,
				 bool onlyFixed, QWidget *parent,
				 bool makeFrame,
                                 QCheckBox::ToggleState *sizeIsRelativeState )
{
  KFontDialog_local dlg( parent, "Font and Text Selector", onlyFixed, true,
		   QStringList(), makeFrame, false, sizeIsRelativeState );
  dlg.setFont( theFont, onlyFixed );

  int result = dlg.exec();
  if( result == Accepted )
  {
    theFont   = dlg.chooser->font();
    theString = dlg.chooser->sampleText();
    if( sizeIsRelativeState )
      *sizeIsRelativeState = dlg.chooser->sizeIsRelative();
  }
  return result;
}

void KFontChooser_local::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KFontDialog_local::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }
