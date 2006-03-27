#include "timedateformatwidget.h"
#include "DateFormatWidget.h"
#include "DateFormatWidget.moc"
#include <qdatetime.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <kglobal.h>
#include <klocale.h>
#include <qlineedit.h>
#include <kdebug.h>
#include <knuminput.h>
#include <KoVariable.h>

/*
 *  Constructs a DateFormatWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'fl'
 */
DateFormatWidget::DateFormatWidget( QWidget* parent,  const char* name, Qt::WFlags fl )
    : TimeDateFormatWidgetPrototype( parent, name, fl )
{
    setCaption( i18n( "DateFormat", "Format of Date Variable" ) );
    QStringList listDateFormat = KoVariableDateFormat::staticTranslatedFormatPropsList();
    combo1->insertStringList(listDateFormat);

    combo2->insertItem( i18n( "Day"));
    combo2->insertItem( i18n( "Day (2 digits)"));
    combo2->insertItem( i18n( "Day (abbreviated name)"));
    combo2->insertItem( i18n( "Day (long name)"));
    combo2->insertItem( i18n( "Month" ) );
    combo2->insertItem( i18n( "Month (2 digits)" ) );
    combo2->insertItem( i18n( "Month (abbreviated name)" ) );
    combo2->insertItem( i18n( "Month (long name)" ) );
    combo2->insertItem( i18n( "Month (possessive abbreviated name)" ) );
    combo2->insertItem( i18n( "Month (possessive long name)" ) );
    combo2->insertItem( i18n( "Year (2 digits)" ) );
    combo2->insertItem( i18n( "Year (4 digits)" ) );

    combo2->insertItem( i18n( "Hour" ) );
    combo2->insertItem( i18n( "Hour (2 digits)" ) );
    combo2->insertItem( i18n( "Minute" ) );
    combo2->insertItem( i18n( "Minute (2 digits)" ) );
    combo2->insertItem( i18n( "Second" ) );
    combo2->insertItem( i18n( "Second (2 digits)" ) );
    combo2->insertItem( i18n( "Millisecond (3 digits)" ) );
    combo2->insertItem( i18n( "am/pm" ) );
    combo2->insertItem( i18n( "AM/PM" ) );

    combo2->setCurrentItem( 0 );

    label_correction->setText(i18n("Correct in Days"));

    connect( CheckBox1, SIGNAL(toggled ( bool )),this,SLOT(slotPersonalizeChanged(bool)));
    connect( combo1, SIGNAL(activated ( const QString & )), this, SLOT(slotDefaultValueChanged(const QString &)));
    connect( combo1, SIGNAL(textChanged ( const QString & )), this, SLOT(slotDefaultValueChanged(const QString &)));
    connect( KIntNumInput1, SIGNAL(valueChanged(int)), this, SLOT( slotOffsetChanged(int)));
    slotPersonalizeChanged(false);
}

/*
 *  Destroys the object and frees any allocated resources
 */
DateFormatWidget::~DateFormatWidget()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 * public slot
 */
void DateFormatWidget::slotDefaultValueChanged(const QString & )
{
    updateLabel();
}

void DateFormatWidget::slotOffsetChanged(int)
{
    updateLabel();
}

void DateFormatWidget::slotPersonalizeChanged(bool b)
{
    combo2->setEnabled(b);
    TextLabel1->setEnabled(b);
    combo1->setEditable(b);
    updateLabel();

}

void DateFormatWidget::comboActivated()
{
    QString string=combo2->currentText();
    if(string==i18n( "Day"))
        combo1->lineEdit()->insert("d");
    else if(string==i18n( "Day (2 digits)"))
        combo1->lineEdit()->insert("dd");
    else if(string==i18n( "Day (abbreviated name)"))
        combo1->lineEdit()->insert("ddd");
    else if(string==i18n( "Day (long name)"))
        combo1->lineEdit()->insert("dddd");
    else if(string==i18n( "Month" ) )
        combo1->lineEdit()->insert("M");
    else if(string==i18n( "Month (2 digits)" ) )
        combo1->lineEdit()->insert("MM");
    else if(string==i18n( "Month (abbreviated name)" ) )
        combo1->lineEdit()->insert("MMM");
    else if(string==i18n( "Month (long name)" ) )
        combo1->lineEdit()->insert("MMMM");
    else if(string==i18n( "Month (possessive abbreviated name)" ) )
        combo1->lineEdit()->insert("PPP");
    else if(string==i18n( "Month (possessive long name)" ) )
        combo1->lineEdit()->insert("PPPP");
    else if(string==i18n( "Year (2 digits)" ) )
        combo1->lineEdit()->insert("yy");
    else if(string==i18n( "Year (4 digits)" ) )
        combo1->lineEdit()->insert("yyyy");

    else if(string==i18n("Hour"))
        combo1->lineEdit()->insert("h");
    else if(string==i18n("Hour (2 digits)"))
        combo1->lineEdit()->insert("hh");
    else if(string==i18n("Minute"))
        combo1->lineEdit()->insert("m");
    else if(string==i18n("Minute (2 digits)"))
        combo1->lineEdit()->insert("mm");
    else if(string==i18n("Second"))
        combo1->lineEdit()->insert("s");
    else if(string==i18n("Second (2 digits)"))
        combo1->lineEdit()->insert("ss");
    else if(string==i18n("Millisecond (3 digits)"))
        combo1->lineEdit()->insert("zzz");
    else if(string==i18n("AM/PM"))
        combo1->lineEdit()->insert("AP");
    else if(string==i18n("am/pm"))
        combo1->lineEdit()->insert("ap");

    updateLabel();
    combo1->setFocus();
}

/*
 * public slot
 */
void DateFormatWidget::updateLabel()
{
    KoVariableDateFormat format;
    format.setFormatProperties( resultString() );
    QDateTime ct = QDateTime::currentDateTime().addDays( correctValue() );
    label->setText( format.convert( ct ) );
}

QString DateFormatWidget::resultString()
{
    const QString lookup(combo1->currentText());
    const QStringList listTranslated( KoVariableDateFormat::staticTranslatedFormatPropsList() );
    const int index = listTranslated.findIndex(lookup);
    if (index==-1)
        return (lookup); // Either costum or non-locale

    // We have now a locale format, so we must "translate" it back;

    // Lookup untranslated format
    const QStringList listRaw( KoVariableDateFormat::staticFormatPropsList() );

    QStringList::ConstIterator it( listRaw.at(index) );
    Q_ASSERT( it != listRaw.end() );
    if ( it != listRaw.end() )
        return *it;
    kError(32500) << "Internal error: could not find correcponding date format: " << lookup << endl;
    return QString::null; // Something is wrong, give back a default
}

int DateFormatWidget::correctValue()
{
    return KIntNumInput1->value();
}
