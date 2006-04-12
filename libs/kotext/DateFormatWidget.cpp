#include "ui_timedateformatwidget.h"
#include "DateFormatWidget.h"
#include "DateFormatWidget.moc"
#include <qdatetime.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <kglobal.h>
#include <klocale.h>
#include <qlineedit.h>
#include <kdebug.h>
#include <knuminput.h>
#include <KoVariable.h>

DateFormatWidget::DateFormatWidget( QWidget* parent )
    : QWidget( parent ),
      m_ui( new Ui_TimeDateFormatWidgetPrototype )
{
    m_ui->setupUi( this );
    QStringList listDateFormat = KoVariableDateFormat::staticTranslatedFormatPropsList();
    m_ui->combo1->addItems(listDateFormat);

    m_ui->combo2->addItem( i18n( "Day"));
    m_ui->combo2->addItem( i18n( "Day (2 digits)"));
    m_ui->combo2->addItem( i18n( "Day (abbreviated name)"));
    m_ui->combo2->addItem( i18n( "Day (long name)"));
    m_ui->combo2->addItem( i18n( "Month" ) );
    m_ui->combo2->addItem( i18n( "Month (2 digits)" ) );
    m_ui->combo2->addItem( i18n( "Month (abbreviated name)" ) );
    m_ui->combo2->addItem( i18n( "Month (long name)" ) );
    m_ui->combo2->addItem( i18n( "Month (possessive abbreviated name)" ) );
    m_ui->combo2->addItem( i18n( "Month (possessive long name)" ) );
    m_ui->combo2->addItem( i18n( "Year (2 digits)" ) );
    m_ui->combo2->addItem( i18n( "Year (4 digits)" ) );

    m_ui->combo2->addItem( i18n( "Hour" ) );
    m_ui->combo2->addItem( i18n( "Hour (2 digits)" ) );
    m_ui->combo2->addItem( i18n( "Minute" ) );
    m_ui->combo2->addItem( i18n( "Minute (2 digits)" ) );
    m_ui->combo2->addItem( i18n( "Second" ) );
    m_ui->combo2->addItem( i18n( "Second (2 digits)" ) );
    m_ui->combo2->addItem( i18n( "Millisecond (3 digits)" ) );
    m_ui->combo2->addItem( i18n( "am/pm" ) );
    m_ui->combo2->addItem( i18n( "AM/PM" ) );

    m_ui->combo2->setCurrentIndex( 0 );

    m_ui->label_correction->setText(i18n("Correct in Days"));

    connect( m_ui->CheckBox1, SIGNAL(toggled ( bool )),this,SLOT(slotPersonalizeChanged(bool)));
    connect( m_ui->combo1, SIGNAL(activated ( const QString & )), this, SLOT(slotDefaultValueChanged(const QString &)));
    connect( m_ui->combo1, SIGNAL(textChanged ( const QString & )), this, SLOT(slotDefaultValueChanged(const QString &)));
    connect( m_ui->combo2, SIGNAL(activated( int ) ), this, SLOT( comboActivated() ) );
    connect( m_ui->KIntNumInput1, SIGNAL(valueChanged(int)), this, SLOT( slotOffsetChanged(int)));

    slotPersonalizeChanged(false);
}

DateFormatWidget::~DateFormatWidget()
{
    delete m_ui;
}

// public slots

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
    m_ui->combo2->setEnabled(b);
    m_ui->TextLabel1->setEnabled(b);
    m_ui->combo1->setEditable(b);
    updateLabel();

}

void DateFormatWidget::comboActivated()
{
    QString string=m_ui->combo2->currentText();
    if(string==i18n( "Day"))
        m_ui->combo1->lineEdit()->insert("d");
    else if(string==i18n( "Day (2 digits)"))
        m_ui->combo1->lineEdit()->insert("dd");
    else if(string==i18n( "Day (abbreviated name)"))
        m_ui->combo1->lineEdit()->insert("ddd");
    else if(string==i18n( "Day (long name)"))
        m_ui->combo1->lineEdit()->insert("dddd");
    else if(string==i18n( "Month" ) )
        m_ui->combo1->lineEdit()->insert("M");
    else if(string==i18n( "Month (2 digits)" ) )
        m_ui->combo1->lineEdit()->insert("MM");
    else if(string==i18n( "Month (abbreviated name)" ) )
        m_ui->combo1->lineEdit()->insert("MMM");
    else if(string==i18n( "Month (long name)" ) )
        m_ui->combo1->lineEdit()->insert("MMMM");
    else if(string==i18n( "Month (possessive abbreviated name)" ) )
        m_ui->combo1->lineEdit()->insert("PPP");
    else if(string==i18n( "Month (possessive long name)" ) )
        m_ui->combo1->lineEdit()->insert("PPPP");
    else if(string==i18n( "Year (2 digits)" ) )
        m_ui->combo1->lineEdit()->insert("yy");
    else if(string==i18n( "Year (4 digits)" ) )
        m_ui->combo1->lineEdit()->insert("yyyy");

    else if(string==i18n("Hour"))
        m_ui->combo1->lineEdit()->insert("h");
    else if(string==i18n("Hour (2 digits)"))
        m_ui->combo1->lineEdit()->insert("hh");
    else if(string==i18n("Minute"))
        m_ui->combo1->lineEdit()->insert("m");
    else if(string==i18n("Minute (2 digits)"))
        m_ui->combo1->lineEdit()->insert("mm");
    else if(string==i18n("Second"))
        m_ui->combo1->lineEdit()->insert("s");
    else if(string==i18n("Second (2 digits)"))
        m_ui->combo1->lineEdit()->insert("ss");
    else if(string==i18n("Millisecond (3 digits)"))
        m_ui->combo1->lineEdit()->insert("zzz");
    else if(string==i18n("AM/PM"))
        m_ui->combo1->lineEdit()->insert("AP");
    else if(string==i18n("am/pm"))
        m_ui->combo1->lineEdit()->insert("ap");

    updateLabel();
    m_ui->combo1->setFocus();
}

/*
 * public slot
 */
void DateFormatWidget::updateLabel()
{
    KoVariableDateFormat format;
    format.setFormatProperties( resultString() );
    QDateTime ct = QDateTime::currentDateTime().addDays( correctValue() );
    m_ui->label->setText( format.convert( ct ) );
}

QString DateFormatWidget::resultString()
{
    const QString lookup(m_ui->combo1->currentText());
    const QStringList listTranslated( KoVariableDateFormat::staticTranslatedFormatPropsList() );
    const int index = listTranslated.indexOf(lookup);
    if (index==-1)
        return lookup; // Either costum or non-locale

    // We have now a locale format, so we must "translate" it back;

    // Lookup untranslated format
    const QStringList listRaw( KoVariableDateFormat::staticFormatPropsList() );
    Q_ASSERT( index < listRaw.count() );
    return listRaw.at( index );
}

int DateFormatWidget::correctValue()
{
    return m_ui->KIntNumInput1->value();
}

QComboBox *DateFormatWidget::combo1()
{
		return m_ui->combo1;
}

QComboBox *DateFormatWidget::combo2()
{
	return m_ui->combo2;
}
		
