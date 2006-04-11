#include "ui_timedateformatwidget.h"
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

TimeFormatWidget::TimeFormatWidget( QWidget* parent )
    : QWidget( parent ),
      m_ui( new Ui_TimeDateFormatWidgetPrototype )
{
    QStringList listTimeFormat = KoVariableTimeFormat::staticTranslatedFormatPropsList();
    m_ui->combo1->addItems(listTimeFormat);

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

    m_ui->label_correction->setText(i18n("Correct in Minutes"));
    connect( m_ui->CheckBox1, SIGNAL(toggled ( bool )),this,SLOT(slotPersonalizeChanged(bool)));
    connect( m_ui->combo1, SIGNAL(activated ( const QString & )), this, SLOT(slotDefaultValueChanged(const QString &)));
    connect( m_ui->combo1, SIGNAL(textChanged ( const QString & )), this, SLOT(slotDefaultValueChanged(const QString &)));
    connect( m_ui->KIntNumInput1, SIGNAL(valueChanged(int)), this, SLOT( slotOffsetChanged(int)));
    slotPersonalizeChanged(false);
}

TimeFormatWidget::~TimeFormatWidget()
{
    delete m_ui;
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
    m_ui->combo2->setEnabled(b);
    m_ui->combo1->setEditable(b);
    m_ui->TextLabel1->setEnabled(b);
    updateLabel();

}

void TimeFormatWidget::comboActivated()
{
    QString string=m_ui->combo2->currentText();
    if(string==i18n("Hour"))
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
void TimeFormatWidget::updateLabel()
{
    KoVariableTimeFormat format;
    format.setFormatProperties( resultString() );

    QTime ct = QTime::currentTime().addSecs(correctValue()); // ### TODO: dialog says correct in *minutes*
    m_ui->label->setText( format.convert( ct ) );
}

QString TimeFormatWidget::resultString() const
{
    const QString lookup(m_ui->combo1->currentText());
    const QStringList listTranslated( KoVariableTimeFormat::staticTranslatedFormatPropsList() );
    const int index = listTranslated.indexOf(lookup);
    if (index==-1)
        return (lookup); // Either costum or non-locale

    // We have now a locale format, so we must "translate" it back;

    // Lookup untranslated format
    const QStringList listRaw( KoVariableTimeFormat::staticFormatPropsList() );
    Q_ASSERT( index < listRaw.count() );
    return listRaw.at( index );
}

int TimeFormatWidget::correctValue() const
{
    return m_ui->KIntNumInput1->value()*60;
}
