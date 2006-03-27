#include "timedateformatwidget.h"
#include "TimeFormatWidget.h"
#include "TimeFormatWidget.moc"
#include <qdatetime.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <qlineedit.h>
#include <knuminput.h>
#include <KoVariable.h>

/*
 *  Constructs a TimeFormatWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
TimeFormatWidget::TimeFormatWidget( QWidget* parent,  const char* name, Qt::WFlags fl )
    : TimeDateFormatWidgetPrototype( parent, name, fl )
{
    setCaption( i18n( "TimeFormat", "This Dialog Allows You to Set the Format of the Time Variable" ) );

    QStringList listTimeFormat = KoVariableTimeFormat::staticTranslatedFormatPropsList();
    combo1->insertStringList(listTimeFormat);

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

    label_correction->setText(i18n("Correct in Minutes"));
    connect( CheckBox1, SIGNAL(toggled ( bool )),this,SLOT(slotPersonalizeChanged(bool)));
    connect( combo1, SIGNAL(activated ( const QString & )), this, SLOT(slotDefaultValueChanged(const QString &)));
    connect( combo1, SIGNAL(textChanged ( const QString & )), this, SLOT(slotDefaultValueChanged(const QString &)));
    connect( KIntNumInput1, SIGNAL(valueChanged(int)), this, SLOT( slotOffsetChanged(int)));
    slotPersonalizeChanged(false);
}

/*
 *  Destroys the object and frees any allocated resources
 */
TimeFormatWidget::~TimeFormatWidget()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 * public slot
 */
void TimeFormatWidget::slotDefaultValueChanged(const QString & )
{
    updateLabel();
}

void TimeFormatWidget::slotOffsetChanged(int)
{
    updateLabel();
}

void TimeFormatWidget::slotPersonalizeChanged(bool b)
{
    combo2->setEnabled(b);
    combo1->setEditable(b);
    TextLabel1->setEnabled(b);
    updateLabel();

}

void TimeFormatWidget::comboActivated()
{
    QString string=combo2->currentText();
    if(string==i18n("Hour"))
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
void TimeFormatWidget::updateLabel()
{
    KoVariableTimeFormat format;
    format.setFormatProperties( resultString() );

    QTime ct = QTime::currentTime().addSecs(correctValue()); // ### TODO: dialog says correct in *minutes*
    label->setText( format.convert( ct ) );
}

QString TimeFormatWidget::resultString()
{
    const QString lookup(combo1->currentText());
    const QStringList listTranslated( KoVariableTimeFormat::staticTranslatedFormatPropsList() );
    const int index = listTranslated.findIndex(lookup);
    if (index==-1)
        return (lookup); // Either costum or non-locale

    // We have now a locale format, so we must "translate" it back;

    // Lookup untranslated format
    const QStringList listRaw( KoVariableTimeFormat::staticFormatPropsList() );

    QStringList::ConstIterator it( listRaw.at(index) );
    Q_ASSERT( it != listRaw.end() );
    if ( it != listRaw.end() )
        return *it;
    kError(32500) << "Internal error: could not find correcponding time format: " << lookup << endl;
    return QString::null; // Something is wrong, give back a default
}

int TimeFormatWidget::correctValue()
{
    return KIntNumInput1->value()*60;
}
