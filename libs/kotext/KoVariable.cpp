/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2005 David Faure <faure@kde.org>

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

#include "KoVariable.h"
#include "KoVariable.moc"
#include "KoTextZoomHandler.h"
#include "TimeFormatWidget.h"
#include "DateFormatWidget.h"
#include "KoTextCommand.h"
#include "KoTextObject.h"
#include "KoTextParag.h"
#include "KoOasisContext.h"
#include <KoOasisSettings.h>

#include <KoDocumentInfo.h>
#include <KoOasisStyles.h>
#include <KoXmlWriter.h>
#include <KoDocument.h>
#include <KoXmlNS.h>
#include <KoDom.h>

#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kdialogbase.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kinstance.h>
#include <kcalendarsystem.h>
#include <kseparatoraction.h>
#include <ktoggleaction.h>
#include <kaboutdata.h>

#include <QStringList>
#include <QComboBox>
#include <q3valuelist.h>

#include <QRadioButton>
//Added by qt3to4:
#include <QString>
#include <Q3PtrList>

#include "IsoDuration.h"

class KoVariableSettings::KoVariableSettingPrivate
{
public:
    KoVariableSettingPrivate()
    {
        m_lastPrintingDate.setTime_t(0); // Default is 1970-01-01 midnight locale time
    }
    QDateTime m_lastPrintingDate;
    QDateTime m_creationDate;
    QDateTime m_modificationDate;
};


KoVariableSettings::KoVariableSettings()
{
    d = new KoVariableSettingPrivate;
    m_startingPageNumber = 1;
    m_displayLink = true;
    m_displayComment = true;
    m_underlineLink = true;
    m_displayFieldCode = false;
}

KoVariableSettings::~KoVariableSettings()
{
    delete d;
    d = 0;
}

QDateTime KoVariableSettings::lastPrintingDate() const
{
    return d->m_lastPrintingDate;
}

void KoVariableSettings::setLastPrintingDate( const QDateTime & _date)
{
    d->m_lastPrintingDate = _date;
}

QDateTime KoVariableSettings::creationDate() const
{
    return d->m_creationDate;
}

void KoVariableSettings::setCreationDate( const QDateTime & _date )
{
    d->m_creationDate = _date;
}

QDateTime KoVariableSettings::modificationDate() const
{
    return d->m_modificationDate;
}

void KoVariableSettings::setModificationDate( const QDateTime & _date)
{
    d->m_modificationDate = _date;
}

void KoVariableSettings::saveOasis( KoXmlWriter &settingsWriter ) const
{
    settingsWriter.startElement("config:config-item-set");
    settingsWriter.addAttribute("config:name", "configuration-variable-settings");
    settingsWriter.addConfigItem("displaylink", m_displayLink );
    settingsWriter.addConfigItem( "underlinelink", m_underlineLink);
    settingsWriter.addConfigItem( "displaycomment", m_displayComment);
    settingsWriter.addConfigItem( "displayfieldcode", m_displayFieldCode);
    // m_startingPageNumber isn't saved to OASIS. Applications must use either
    // style:page-number in the first parag of a page (see KoTextParag), or
    // style:first-page-number in style:page-layout, for spreadsheets etc.
    if ( d->m_lastPrintingDate.isValid())
        settingsWriter.addConfigItem("lastPrintingDate", d->m_lastPrintingDate.toString(Qt::ISODate));

    if ( d->m_creationDate.isValid())
        settingsWriter.addConfigItem("creationDate", d->m_creationDate.toString(Qt::ISODate));

    if ( d->m_modificationDate.isValid())
        settingsWriter.addConfigItem("modificationDate", d->m_modificationDate.toString(Qt::ISODate));

    settingsWriter.endElement(); // config:config-item-set
}

void KoVariableSettings::loadOasis(const KoOasisSettings&settingsDoc)
{
    KoOasisSettings::Items configurationSettings = settingsDoc.itemSet( "configuration-variable-settings" );
    if ( !configurationSettings.isNull() )
    {
        m_displayLink = configurationSettings.parseConfigItemBool( "displaylink", true );
        m_underlineLink = configurationSettings.parseConfigItemBool( "underlinelink", true );
        m_displayComment = configurationSettings.parseConfigItemBool( "displaycomment", true );
        m_displayFieldCode = configurationSettings.parseConfigItemBool( "displayfieldcode", false );

        QString str = configurationSettings.parseConfigItemString( "lastPrintingDate" );
        if ( !str.isEmpty() )
            d->m_lastPrintingDate = QDateTime::fromString( str, Qt::ISODate );
        else
            d->m_lastPrintingDate.setTime_t(0); // 1970-01-01 00:00:00.000 locale time

        str = configurationSettings.parseConfigItemString( "creationDate" );
        if ( !str.isEmpty() ) {
            d->m_creationDate = QDateTime::fromString( str, Qt::ISODate );
        }

        str = configurationSettings.parseConfigItemString( "modificationDate" );
        if ( !str.isEmpty() )
            d->m_modificationDate = QDateTime::fromString( str, Qt::ISODate );

        // m_startingPageNumber isn't loaded from OASIS here. KWTextParag::loadOasis does it.
    }
}

void KoVariableSettings::save( QDomElement &parentElem )
{
     QDomElement elem = parentElem.ownerDocument().createElement( "VARIABLESETTINGS" );
     parentElem.appendChild( elem );
    if(m_startingPageNumber!=1)
    {
        elem.setAttribute( "startingPageNumber", m_startingPageNumber );
    }
    elem.setAttribute("displaylink",(int)m_displayLink);
    elem.setAttribute("underlinelink",(int)m_underlineLink);
    elem.setAttribute("displaycomment",(int)m_displayComment);
    elem.setAttribute("displayfieldcode", (int)m_displayFieldCode);

    if ( d->m_lastPrintingDate.isValid())
        elem.setAttribute("lastPrintingDate", d->m_lastPrintingDate.toString(Qt::ISODate));

    if ( d->m_creationDate.isValid())
        elem.setAttribute("creationDate", d->m_creationDate.toString(Qt::ISODate));

    if ( d->m_modificationDate.isValid())
        elem.setAttribute("modificationDate", d->m_modificationDate.toString(Qt::ISODate));
}

void KoVariableSettings::load( QDomElement &elem )
{
    QDomElement e = elem.namedItem( "VARIABLESETTINGS" ).toElement();
    if (!e.isNull())
    {
        if(e.hasAttribute("startingPageNumber"))
            m_startingPageNumber = e.attribute("startingPageNumber").toInt();
        if(e.hasAttribute("displaylink"))
            m_displayLink=(bool)e.attribute("displaylink").toInt();
        if(e.hasAttribute("underlinelink"))
            m_underlineLink=(bool)e.attribute("underlinelink").toInt();
        if(e.hasAttribute("displaycomment"))
            m_displayComment=(bool)e.attribute("displaycomment").toInt();
        if (e.hasAttribute("displayfieldcode"))
            m_displayFieldCode=(bool)e.attribute("displayfieldcode").toInt();

        if (e.hasAttribute("lastPrintingDate"))
            d->m_lastPrintingDate = QDateTime::fromString( e.attribute( "lastPrintingDate" ), Qt::ISODate );
        else
            d->m_lastPrintingDate.setTime_t(0); // 1970-01-01 00:00:00.000 locale time

        if (e.hasAttribute("creationDate")) {
            d->m_creationDate = QDateTime::fromString( e.attribute( "creationDate" ), Qt::ISODate );
        }

        if (e.hasAttribute("modificationDate"))
            d->m_modificationDate = QDateTime::fromString( e.attribute( "modificationDate" ), Qt::ISODate );
    }
}

KoVariableDateFormat::KoVariableDateFormat() : KoVariableFormat()
{
}

QString KoVariableDateFormat::convert( const QVariant& data ) const
{
    if ( data.type() != QVariant::Date && data.type() != QVariant::DateTime )
    {
        kWarning(32500)<<" Error in KoVariableDateFormat::convert. Value is a "
                      << data.typeName() << "(" << data.type() << ")" << endl;
        // dateTime will be invalid, then set to 1970-01-01
    }
    QDateTime dateTime ( data.toDateTime() );
    if ( !dateTime.isValid() )
        return i18n("No date set"); // e.g. old KWord documents

    if (m_strFormat.toLower() == "locale" || m_strFormat.isEmpty())
        return KGlobal::locale()->formatDate( dateTime.date(), false );
    else if ( m_strFormat.toLower() == "localeshort" )
        return KGlobal::locale()->formatDate( dateTime.date(), true );
    else if ( m_strFormat.toLower() == "localedatetime" )
        return KGlobal::locale()->formatDateTime( dateTime, false );
    else if ( m_strFormat.toLower() == "localedatetimeshort" )
        return KGlobal::locale()->formatDateTime( dateTime, true );

    QString tmp ( dateTime.toString(m_strFormat) );
    const int month = dateTime.date().month();
    tmp.replace("PPPP", KGlobal::locale()->calendar()->monthNamePossessive(month, false)); //long possessive month name
    tmp.replace("PPP",  KGlobal::locale()->calendar()->monthNamePossessive(month, true));  //short possessive month name
    return tmp;
}

QString KoVariableDateFormat::key() const
{
    return getKey( m_strFormat );
}

QString KoVariableDateFormat::getKey( const QString& props ) const
{
    return QString("DATE") + props.toUtf8();
}

void KoVariableDateFormat::load( const QString &key )
{
    QString params( key.mid( 4 ) ); // skip "DATE"
    if ( !params.isEmpty() )
    {
        if (params[0] == '1' || params[0] == '0') // old m_bShort crap
            params = params.mid(1); // skip it
        m_strFormat = params;
    }
}

// Used by KoVariableFormatCollection::popupActionList(), to apply all formats
// to the current data, in the popup menu.
QStringList KoVariableDateFormat::staticFormatPropsList()
{
    QStringList listDateFormat;
    listDateFormat<<"locale";
    listDateFormat<<"localeshort";
    listDateFormat<<"localedatetime";
    listDateFormat<<"localedatetimeshort";
    listDateFormat<<"dd/MM/yy";
    listDateFormat<<"dd/MM/yyyy";
    listDateFormat<<"MMM dd,yy";
    listDateFormat<<"MMM dd,yyyy";
    listDateFormat<<"dd.MMM.yyyy";
    listDateFormat<<"MMMM dd, yyyy";
    listDateFormat<<"ddd, MMM dd,yy";
    listDateFormat<<"dddd, MMM dd,yy";
    listDateFormat<<"MM-dd";
    listDateFormat<<"yyyy-MM-dd";
    listDateFormat<<"dd/yy";
    listDateFormat<<"MMMM";
    listDateFormat<<"yyyy-MM-dd hh:mm";
    listDateFormat<<"dd.MMM.yyyy hh:mm";
    listDateFormat<<"MMM dd,yyyy h:mm AP";
    listDateFormat<<"yyyy-MM-ddThh:mm:ss"; // ISO 8601
    return listDateFormat;
}

// Used by dateformatwidget_impl
// TODO: shouldn't it apply the formats to the value, like the popupmenu does?
QStringList KoVariableDateFormat::staticTranslatedFormatPropsList()
{
    QStringList listDateFormat;
    listDateFormat<<i18n("Locale date format");
    listDateFormat<<i18n("Short locale date format");
    listDateFormat<<i18n("Locale date & time format");
    listDateFormat<<i18n("Short locale date & time format");
    listDateFormat<<"dd/MM/yy";
    listDateFormat<<"dd/MM/yyyy";
    listDateFormat<<"MMM dd,yy";
    listDateFormat<<"MMM dd,yyyy";
    listDateFormat<<"dd.MMM.yyyy";
    listDateFormat<<"MMMM dd, yyyy";
    listDateFormat<<"ddd, MMM dd,yy";
    listDateFormat<<"dddd, MMM dd,yy";
    listDateFormat<<"MM-dd";
    listDateFormat<<"yyyy-MM-dd";
    listDateFormat<<"dd/yy";
    listDateFormat<<"MMMM";
    listDateFormat<<"yyyy-MM-dd hh:mm";
    listDateFormat<<"dd.MMM.yyyy hh:mm";
    listDateFormat<<"MMM dd,yyyy h:mm AP";
    listDateFormat<<"yyyy-MM-ddThh:mm:ss"; // ISO 8601
    return listDateFormat;
}

////

KoVariableTimeFormat::KoVariableTimeFormat() : KoVariableFormat()
{
}

void KoVariableTimeFormat::load( const QString &key )
{
    QString params( key.mid( 4 ) );
    if ( !params.isEmpty() )
	m_strFormat = params;
}

QString KoVariableTimeFormat::convert( const QVariant & time ) const
{
    if ( time.type() != QVariant::Time )
    {
        kDebug(32500)<<" Error in KoVariableTimeFormat::convert. Value is a "
                      << time.typeName() << "(" << time.type() << ")" << endl;
        return QString::null;
    }

    if( m_strFormat.toLower() == "locale" || m_strFormat.isEmpty() )
	return KGlobal::locale()->formatTime( time.toTime() );
    return time.toTime().toString(m_strFormat);
}

QString KoVariableTimeFormat::key() const
{
    return getKey( m_strFormat );
}

QString KoVariableTimeFormat::getKey( const QString& props ) const
{
    return QString("TIME") + props.toUtf8();
}

// Used by KoVariableFormatCollection::popupActionList(), to apply all formats
// to the current data, in the popup menu.
QStringList KoVariableTimeFormat::staticFormatPropsList()
{
    QStringList listTimeFormat;
    listTimeFormat<<"locale";
    listTimeFormat<<"hh:mm";
    listTimeFormat<<"hh:mm:ss";
    listTimeFormat<<"hh:mm AP";
    listTimeFormat<<"hh:mm:ss AP";
    listTimeFormat<<"mm:ss.zzz";
    return listTimeFormat;
}

// Used by timeformatwidget_impl
QStringList KoVariableTimeFormat::staticTranslatedFormatPropsList()
{
    QStringList listTimeFormat;
    listTimeFormat<<i18n("Locale format");
    listTimeFormat<<"hh:mm";
    listTimeFormat<<"hh:mm:ss";
    listTimeFormat<<"hh:mm AP";
    listTimeFormat<<"hh:mm:ss AP";
    listTimeFormat<<"mm:ss.zzz";
    return listTimeFormat;
}

////

QString KoVariableStringFormat::convert( const QVariant & string ) const
{
    if ( string.type() != QVariant::String )
    {
        kDebug(32500)<<" Error in KoVariableStringFormat::convert. Value is a " << string.typeName() << endl;
        return QString::null;
    }

    return string.toString();
}

QString KoVariableStringFormat::key() const
{
    return getKey( QString::null );
    // TODO prefix & suffix
}

QString KoVariableStringFormat::getKey( const QString& props ) const
{
    return QString("STRING") + props.toUtf8();
}

////

QString KoVariableNumberFormat::convert( const QVariant &value ) const
{
    if ( value.type() != QVariant::Int )
    {
        kDebug(32500)<<" Error in KoVariableNumberFormat::convert. Value is a " << value.typeName() << endl;
        return QString::null;
    }

    return QString::number( value.toInt() );
}

QString KoVariableNumberFormat::key() const
{
    return getKey(QString::null);
}

QString KoVariableNumberFormat::getKey( const QString& props ) const
{
    return QString("NUMB") + props.toUtf8();
}

////

KoVariableFormatCollection::KoVariableFormatCollection()
{
    m_dict.setAutoDelete( true );
}

KoVariableFormat * KoVariableFormatCollection::format( const QString &key )
{
    KoVariableFormat *f = m_dict[ key.toLatin1() ];
    if (f)
        return f;
    else
        return createFormat( key );
}

KoVariableFormat * KoVariableFormatCollection::createFormat( const QString &key )
{
    kDebug(32500) << "KoVariableFormatCollection: creating format for key=" << key << endl;
    KoVariableFormat * format = 0L;
    // The first 4 chars identify the class
    QString type = key.left(4);
    if ( type == "DATE" )
        format = new KoVariableDateFormat();
    else if ( type == "TIME" )
        format = new KoVariableTimeFormat();
    else if ( type == "NUMB" ) // this type of programming makes me numb ;)
        format = new KoVariableNumberFormat();
    else if ( type == "STRI" )
        format = new KoVariableStringFormat();

    if ( format )
    {
        format->load( key );
        m_dict.insert( format->key().toLatin1()/* not 'key', it could be incomplete */, format );
    }
    return format;
}

/******************************************************************/
/* Class:       KoVariableCollection                              */
/******************************************************************/
KoVariableCollection::KoVariableCollection(KoVariableSettings *_settings, KoVariableFormatCollection *formatCollection)
{
    m_variableSettings = _settings;
    m_varSelected = 0L;
    m_formatCollection = formatCollection;
}

KoVariableCollection::~KoVariableCollection()
{
    delete m_variableSettings;
}

void KoVariableCollection::clear()
{
    variables.clear();
    varValues.clear();
    m_varSelected = 0;
}

void KoVariableCollection::registerVariable( KoVariable *var )
{
    if ( !var )
        return;
    variables.append( var );
}

void KoVariableCollection::unregisterVariable( KoVariable *var )
{
    variables.take( variables.findRef( var ) );
}

Q3ValueList<KoVariable *> KoVariableCollection::recalcVariables(int type)
{
    Q3ValueList<KoVariable *> modifiedVariables;
    Q3PtrListIterator<KoVariable> it( variables );
    for ( ; it.current() ; ++it )
    {
        KoVariable* variable = it.current();
        if ( variable->isDeleted() )
            continue;
        if ( variable->type() == type || type == VT_ALL )
        {
            QVariant oldValue = variable->varValue();
            variable->recalc();
            if(variable->height == 0)
                variable->resize();
            if ( variable->varValue() != oldValue )
                modifiedVariables.append( variable );
            KoTextParag * parag = variable->paragraph();
            if ( parag )
            {
                //kDebug(32500) << "KoDoc::recalcVariables -> invalidating parag " << parag->paragId() << endl;
                parag->invalidate( 0 );
                parag->setChanged( true );
            }
        }
    }
#if 0
    // TODO pass list of textdocuments as argument
    // Or even better, call emitRepaintChanged on all modified textobjects
    if( !modifiedVariables.isEmpty() )
        emit repaintVariable();
#endif
    return modifiedVariables;
}


void KoVariableCollection::setVariableValue( const QString &name, const QString &value )
{
    varValues[ name ] = value;
}

QString KoVariableCollection::getVariableValue( const QString &name ) const
{
    if ( !varValues.contains( name ) )
        return i18n( "No value" );
    return varValues[ name ];
}

bool KoVariableCollection::customVariableExist(const QString &varname) const
{
    return varValues.contains( varname );
}

void KoVariableCollection::setVariableSelected(KoVariable * var)
{
    m_varSelected=var;
}

// TODO change to QValueList<KAction *>, but only once plugActionList takes that
QList<KAction*> KoVariableCollection::popupActionList() const
{
    QList<KAction*> listAction;
    // Insert list of actions that change the subtype
    const QStringList subTypeList = m_varSelected->subTypeList();
    kDebug() << k_funcinfo << "current subtype=" << m_varSelected->subType() << endl;
    QStringList::ConstIterator it = subTypeList.begin();
    for ( int i = 0; it != subTypeList.end() ; ++it, ++i )
    {
        if ( !(*it).isEmpty() ) // in case of removed subtypes or placeholders
        {
            // We store the subtype number as the action name
            QString name; name.setNum(i);
            KToggleAction * act = new KToggleAction( *it, KShortcut(), 0, name );
            connect( act, SIGNAL(activated()), this, SLOT(slotChangeSubType()) );
            if ( i == m_varSelected->subType() )
                act->setChecked( true );
            //m_subTextMap.insert( act, i );
            listAction.append( act );
        }
    }
    // Insert list of actions that change the format properties
    KoVariableFormat* format = m_varSelected->variableFormat();
    QString currentFormat = format->formatProperties();

    const QStringList list = format->formatPropsList();
    it = list.begin();
    for ( int i = 0; it != list.end() ; ++it, ++i )
    {
        if( i == 0 ) // first item, and list not empty
            listAction.append( new KSeparatorAction() );

        if ( !(*it).isEmpty() ) // in case of removed subtypes or placeholders
        {
            format->setFormatProperties( *it ); // temporary change
            QString text = format->convert( m_varSelected->varValue() );
            // We store the raw format as the action name
            KToggleAction * act = new KToggleAction(text, KShortcut(), 0, (*it).toUtf8());
            connect( act, SIGNAL(activated()), this, SLOT(slotChangeFormat()) );
            if ( (*it) == currentFormat )
                act->setChecked( true );
            listAction.append( act );
        }
    }

    // Restore current format
    format->setFormatProperties( currentFormat );
    return listAction;
}

void KoVariableCollection::slotChangeSubType()
{
    KAction * act = (KAction *)(sender());
    int menuNumber = act->objectName().toInt();
    int newSubType = m_varSelected->variableSubType(menuNumber);
    kDebug(32500) << "slotChangeSubType: menuNumber=" << menuNumber << " newSubType=" << newSubType << endl;
    if ( m_varSelected->subType() != newSubType )
    {
        KoChangeVariableSubType *cmd=new KoChangeVariableSubType(
            m_varSelected->subType(), newSubType, m_varSelected );
        cmd->execute();
        m_varSelected->textDocument()->emitNewCommand(cmd);
    }
}

void KoVariableCollection::slotChangeFormat()
{
    KAction * act = (KAction *)(sender());
    QString newFormat = act->objectName();
    QString oldFormat = m_varSelected->variableFormat()->formatProperties();
    if (oldFormat != newFormat )
    {
        KCommand *cmd=new KoChangeVariableFormatProperties(
            oldFormat, newFormat, m_varSelected );
        cmd->execute();
        m_varSelected->textDocument()->emitNewCommand(cmd);
    }
}

KoVariable * KoVariableCollection::createVariable( int type, short int subtype, KoVariableFormatCollection * coll, KoVariableFormat *varFormat,KoTextDocument *textdoc, KoDocument * doc, int _correct, bool _forceDefaultFormat, bool /*loadFootNote*/ )
{
    Q_ASSERT( coll == m_formatCollection ); // why do we need a parameter ?!?
    QString string;
    QStringList stringList;
    if ( varFormat == 0L )
    {
        // Get the default format for this variable (this method is only called in the interactive case, not when loading)
        switch ( type ) {
        case VT_DATE:
        case VT_DATE_VAR_KWORD10:  // compatibility with kword 1.0
        {
            if ( _forceDefaultFormat )
                varFormat = coll->format( KoDateVariable::defaultFormat() );
            else
            {
                QString result = KoDateVariable::formatStr(_correct);
                if ( result.isNull() )//we cancel insert variable
                    return 0L;
                varFormat = coll->format( result );
            }
            break;
        }
        case VT_TIME:
        case VT_TIME_VAR_KWORD10:  // compatibility with kword 1.0
        {
            if ( _forceDefaultFormat )
                varFormat = coll->format( KoTimeVariable::defaultFormat() );
            else
            {
                QString result = KoTimeVariable::formatStr(_correct);
                if ( result.isNull() )//we cancel insert variable
                    return 0L;
                varFormat = coll->format( result );
            }
            break;
        }
        case VT_PGNUM:
            varFormat = coll->format( "NUMBER" );
            break;
        case VT_FIELD:
        case VT_CUSTOM:
        case VT_MAILMERGE:
        case VT_LINK:
        case VT_NOTE:
            varFormat = coll->format( "STRING" );
            break;
        case VT_FOOTNOTE: // this is a KWord-specific variable
            kError() << "Footnote type not handled in KoVariableCollection: VT_FOOTNOTE" << endl;
            return 0L;
        case VT_STATISTIC:
            kError() << " Statistic type not handled in KoVariableCollection: VT_STATISTIC" << endl;
            return 0L;
        }
    }
    Q_ASSERT( varFormat );
    if ( varFormat == 0L ) // still 0 ? Impossible!
        return 0L ;

    kDebug(32500) << "Creating variable. Format=" << varFormat->key() << " type=" << type << endl;
    KoVariable * var = 0L;
    switch ( type ) {
        case VT_DATE:
        case VT_DATE_VAR_KWORD10:  // compatibility with kword 1.0
            var = new KoDateVariable( textdoc, subtype, varFormat, this, _correct );
            break;
        case VT_TIME:
        case VT_TIME_VAR_KWORD10:  // compatibility with kword 1.0
            var = new KoTimeVariable( textdoc, subtype, varFormat, this, _correct );
            break;
        case VT_PGNUM:
            kError() << "VT_PGNUM must be handled by the application's reimplementation of KoVariableCollection::createVariable" << endl;
            //var = new KoPageVariable( textdoc, subtype, varFormat, this );
            break;
        case VT_FIELD:
            var = new KoFieldVariable( textdoc, subtype, varFormat,this,doc );
            break;
        case VT_CUSTOM:
            var = new KoCustomVariable( textdoc, QString::null, varFormat, this);
            break;
        case VT_MAILMERGE:
            var = new KoMailMergeVariable( textdoc, QString::null, varFormat ,this);
            break;
        case VT_LINK:
            var = new KoLinkVariable( textdoc,QString::null, QString::null, varFormat ,this);
            break;
        case VT_NOTE:
            var = new KoNoteVariable( textdoc, QString::null, varFormat ,this);
            break;
    }
    Q_ASSERT( var );
    return var;
}


KoVariable* KoVariableCollection::loadOasisField( KoTextDocument* textdoc, const QDomElement& tag, KoOasisContext& context )
{
    const QString localName( tag.localName() );
    const bool isTextNS = tag.namespaceURI() == KoXmlNS::text;
    QString key;
    int type = -1;
    if ( isTextNS )
    {
        if ( localName.endsWith( "date" ) || localName.endsWith( "time" ) )
        {
            QString dataStyleName = tag.attributeNS( KoXmlNS::style, "data-style-name", QString::null );
            QString dateFormat = "locale";
            const KoOasisStyles::DataFormatsMap& map = context.oasisStyles().dataFormats();
            KoOasisStyles::DataFormatsMap::const_iterator it = map.find( dataStyleName );
            if ( it != map.end() )
                dateFormat = (*it).formatStr;

            // Only text:time is a pure time (the data behind is only h/m/s)
            // ### FIXME: not true, a time can have a date too (reason: for MS Word (already from long ago) time and date are the same thing. But for OO the correction is not in the same unit for time and date.)
            // Whereas print-time/creation-time etc. are actually related to a date/time value.
            if ( localName == "time" )
            {
                type = VT_TIME;
                key = "TIME" + dateFormat;
            }
            else
            {
                type = VT_DATE;
                key = "DATE" + dateFormat;
            }
        }
        else if (localName == "page-number" || localName == "page-count" )
        {
            type = VT_PGNUM;
            key = "NUMBER";
        }
        else if (localName == "chapter")
        {
            type = VT_PGNUM;
            key = "STRING";
        }
        else if (localName == "file-name")
        {
            type = VT_FIELD;
            key = "STRING";
        }
        else if (localName == "author-name"
                 || localName == "author-initials"
                 || localName == "subject"
                 || localName == "title"
                 || localName == "description"
                 || localName == "keywords")
        {
            type = VT_FIELD;
            key = "STRING";
        }
        else if ( localName.startsWith( "sender-" )
                  && localName != "sender-firstname" // not supported
                  && localName != "sender-lastname" // not supported
                  && localName != "sender-initials" // not supported
            )
        {
            type = VT_FIELD;
            key = "STRING";
        }
        else if ( localName == "variable-set"
                  || localName == "user-defined"
                  || localName == "user-field-get" )
        {
            key = "STRING";
            type = VT_CUSTOM;
        }
        else
            return 0L;
    }
    else if ( tag.namespaceURI() == KoXmlNS::office && localName == "annotation" )
    {
        type = VT_NOTE;
        key = "NUMBER";
    }
    else
    {
        // Not an error. It's simply not a variable tag (the caller doesn't check for that)
        return 0;
    }
// TODO localName == "page-variable-get", "initial-creator" and many more
// TODO VT_MAILMERGE

    return loadOasisFieldCreateVariable( textdoc, tag, context, key, type );
}

KoVariable* KoVariableCollection::loadOasisFieldCreateVariable( KoTextDocument* textdoc, const QDomElement& tag, KoOasisContext& context, const QString &key, int type )
{
    KoVariableFormat * varFormat = key.isEmpty() ? 0 : m_formatCollection->format( key.toLatin1() );
    // If varFormat is 0 (no key specified), the default format will be used.

    KoVariable* var = createVariable( type, -1, m_formatCollection, varFormat, textdoc, context.koDocument(), 0 /*correct*/, true );
    var->loadOasis( tag, context );
    return var;
}

/******************************************************************/
/* Class: KoVariable                                              */
/******************************************************************/
KoVariable::KoVariable( KoTextDocument *textdoc, KoVariableFormat *varFormat, KoVariableCollection *_varColl)
    : KoTextCustomItem( textdoc )
{
    //d = new Private;
    m_varColl=_varColl;
    m_varFormat = varFormat;
    m_varColl->registerVariable( this );
    m_ascent = 0;
}

KoVariable::~KoVariable()
{
    //kDebug(32500) << "KoVariable::~KoVariable " << this << endl;
    m_varColl->unregisterVariable( this );
    //delete d;
}

QStringList KoVariable::subTypeList()
{
    return QStringList();
}

void KoVariable::resize()
{
    if ( m_deleted )
        return;
    KoTextFormat *fmt = format();
    QFontMetrics fm = fmt->refFontMetrics();
    QString txt = text();

    width = 0;
     // size at 100%
    for ( int i = 0 ; i < (int)txt.length() ; ++i )
        width += fm.width( txt[i] ); // was fm.charWidth(txt,i), but see drawCustomItemHelper...
    // zoom to LU
    width = qRound( KoTextZoomHandler::ptToLayoutUnitPt( width ) );
    height = fmt->height();
    m_ascent = fmt->ascent();
    //kDebug(32500) << "KoVariable::resize text=" << txt << " width=" << width << " height=" << height << " ascent=" << m_ascent << endl;
}

void KoVariable::recalcAndRepaint()
{
    recalc();
    KoTextParag * parag = paragraph();
    if ( parag )
    {
        //kDebug(32500) << "KoVariable::recalcAndRepaint -> invalidating parag " << parag->paragId() << endl;
        parag->invalidate( 0 );
        parag->setChanged( true );
    }
    textDocument()->emitRepaintChanged();
}

QString KoVariable::fieldCode()
{
    return i18n("Variable");
}

QString KoVariable::text(bool realValue)
{
    KoTextFormat *fmt = format();
    QString str;
    if (m_varColl->variableSetting()->displayFieldCode()&&!realValue)
        str = fieldCode();
    else
        str = m_varFormat->convert( m_varValue );

    return fmt->displayedString( str);
}

void KoVariable::drawCustomItem( QPainter* p, int x, int y, int wpix, int hpix, int ascentpix, int /*cx*/, int /*cy*/, int /*cw*/, int /*ch*/, const QColorGroup& cg, bool selected, int offset, bool drawingShadow )
{
    KoTextFormat * fmt = format();
    KoTextZoomHandler * zh = textDocument()->paintingZoomHandler();
    QFont font( fmt->screenFont( zh ) );
    drawCustomItemHelper( p, x, y, wpix, hpix, ascentpix, cg, selected, offset, fmt, font, fmt->color(), drawingShadow );
}

void KoVariable::drawCustomItemHelper( QPainter* p, int x, int y, int wpix, int hpix, int ascentpix, const QColorGroup& cg, bool selected, int offset, KoTextFormat* fmt, const QFont& font, QColor textColor, bool drawingShadow )
{
    // Important: the y value already includes the difference between the parag baseline
    // and the char's own baseline (ascent) (see paintDefault in korichtext.cpp)
    // So we just draw the text there. But we need the baseline for drawFontEffects...
    KoTextZoomHandler * zh = textDocument()->paintingZoomHandler();

    p->save();

    if ( fmt->textBackgroundColor().isValid() )
        p->fillRect( x, y, wpix, hpix, fmt->textBackgroundColor() );

    if ( drawingShadow ) // Use shadow color if drawing a shadow
    {
        textColor = fmt->shadowColor();
        p->setPen( textColor );
    }
    else if ( selected )
    {
        textColor = cg.color( QColorGroup::HighlightedText );
        p->setPen( QPen( textColor ) );
        p->fillRect( x, y, wpix, hpix, cg.color( QColorGroup::Highlight ) );
    }
    else if ( textDocument() && textDocument()->drawFormattingChars()
              && p->device()->devType() != QInternal::Printer )
    {
        textColor = cg.color( QColorGroup::Highlight );
        p->setPen( QPen ( textColor, 0, Qt::DotLine ) );
        p->drawRect( x, y, wpix, hpix );
    }
    else {
        if ( !textColor.isValid() ) // Resolve the color at this point
            textColor = KoTextFormat::defaultTextColor( p );
        p->setPen( QPen( textColor ) );
    }

    p->setFont( font ); // already done by KoTextCustomItem::draw but someone might
                        // change the font passed to drawCustomItemHelper (e.g. KoLinkVariable)
    QString str = text();
    KoTextParag::drawFontEffects( p, fmt, zh, font, textColor, x, ascentpix, wpix, y, hpix, str[0] );
    int posY = y + ascentpix + offset;
    if ( fmt->vAlign() == KoTextFormat::AlignSubScript )
        posY +=p->fontMetrics().height() / 6;
    if ( fmt->vAlign() != KoTextFormat::AlignSuperScript )
        posY -= fmt->offsetFromBaseLine();
    else if ( fmt->offsetFromBaseLine() < 0 )
        posY -= 2*fmt->offsetFromBaseLine();

    //p->drawText( x, posY, str );
    // We can't just drawText, it wouldn't use the same kerning as the one
    // that resize() planned for [which is zoom-independent].
    // We need to do the layout using layout units instead, so for simplicity
    // I just draw every char individually (whereas KoTextFormatter/KoTextParag
    // detect runs of text that can be drawn together)
    const int len = str.length();
    int xLU = zh->pixelToLayoutUnitX( x );
    QFontMetrics fm = fmt->refFontMetrics();
    for ( int i = 0; i < len; ++i )
    {
        const QChar ch = str[i];
        p->drawText( x, posY, ch );
        // Do like KoTextFormatter: do the layout in layout units.
        xLU += KoTextZoomHandler::ptToLayoutUnitPt( fm.width( ch ) );
        // And then compute the X position in pixels from the layout unit X.
        x = zh->layoutUnitToPixelX( xLU );
    }

    p->restore();
}

void KoVariable::save( QDomElement &parentElem )
{
    //kDebug(32500) << "KoVariable::save" << endl;
    QDomElement variableElem = parentElem.ownerDocument().createElement( "VARIABLE" );
    parentElem.appendChild( variableElem );
    QDomElement typeElem = parentElem.ownerDocument().createElement( "TYPE" );
    variableElem.appendChild( typeElem );
    typeElem.setAttribute( "type", static_cast<int>( type() ) );

    //// Of course, saving the key is ugly. We'll drop this when
    //// switching to the OO format.
    typeElem.setAttribute( "key", m_varFormat->key() );
    typeElem.setAttribute( "text", text(true) );
    if ( correctValue() != 0)
        typeElem.setAttribute( "correct", correctValue() );
    saveVariable( variableElem );
}

void KoVariable::load( QDomElement & )
{
}


void KoVariable::loadOasis( const QDomElement &/*elem*/, KoOasisContext& /*context*/ )
{
    // nothing to do here, reimplemented in subclasses (make it pure virtual?)
}

void KoVariable::saveOasis( KoXmlWriter& /*writer*/, KoSavingContext& /*context*/ ) const
{
}

void KoVariable::setVariableFormat( KoVariableFormat *_varFormat )
{
    // TODO if ( _varFormat ) _varFormat->deref();
    m_varFormat = _varFormat;
    // TODO m_varFormat->ref();
}

#define addText( text, newFormat ) { \
        if ( !text.isEmpty() ) \
        { \
            newFormat +=text; \
            text=""; \
        } \
}

QString KoVariable::convertKlocaleToQDateTimeFormat( const QString & _format )
{
    QString newFormat;
    QString format( _format );
    QString text;
    do
    {
        if ( format.startsWith( "%Y" ) )
        {
            addText( text, newFormat );
            newFormat+="yyyy";
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%y" ) )
        {
            addText( text, newFormat );
            newFormat+="yyyy";

            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%n" ) )
        {
            addText( text, newFormat );
            newFormat+="M";
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%m" ) )
        {
            addText( text, newFormat );
            newFormat+="MM";
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%e" ) )
        {
            addText( text, newFormat );
            newFormat+="d";
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%d" ) )
        {
            addText( text, newFormat );
            newFormat+="dd";
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%b" ) )
        {
            addText( text, newFormat );
            newFormat+="MMM";
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%B" ) )
        {
            addText( text, newFormat );
            newFormat+="MMMM";
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%a" ) )
        {
            addText( text, newFormat );
            newFormat+="ddd";

            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%A" ) )
        {
            addText( text, newFormat );
            newFormat+="dddd";
            format = format.remove( 0, 2 );
        }
        if ( format.startsWith( "%H" ) ) //hh
        {
            //hour in 24h
            addText( text, newFormat );
            newFormat+="hh";
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%k" ) )//h
        {
            addText( text, newFormat );
            newFormat+="h";
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%I" ) )// ?????
        {
            addText( text, newFormat );
            //TODO hour in 12h
        }
        else if ( format.startsWith( "%l" ) )
        {
            addText( text, newFormat );
            //TODO hour in 12h with 1 digit
        }
        else if ( format.startsWith( "%M" ) )// mm
        {
            addText( text, newFormat );
            newFormat+="mm";
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%S" ) ) //ss
        {
            addText( text, newFormat );
            newFormat+="ss";
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%p" ) )
        {
            //TODO am or pm
            addText( text, newFormat );
            newFormat+="ap";
            format = format.remove( 0, 2 );
        }

        else
        {
            text += format[0];
            format = format.remove( 0, 1 );
        }
    }
    while ( format.length() > 0 );
    addText( text, format );
    return format;
}


/******************************************************************/
/* Class: KoDateVariable                                          */
/******************************************************************/
KoDateVariable::KoDateVariable( KoTextDocument *textdoc, short int subtype, KoVariableFormat *_varFormat, KoVariableCollection *_varColl, int _correctDate)
    : KoVariable( textdoc, _varFormat,_varColl ), m_subtype( subtype ), m_correctDate( _correctDate)
{
}

QString KoDateVariable::fieldCode()
{
    if ( m_subtype == VST_DATE_FIX )
        return i18n("Date (Fixed)");
    else if ( m_subtype == VST_DATE_CURRENT)
        return i18n("Date");
    else if ( m_subtype == VST_DATE_LAST_PRINTING)
        return i18n("Last Printing");
    else if ( m_subtype == VST_DATE_CREATE_FILE )
        return i18n( "File Creation");
    else if ( m_subtype == VST_DATE_MODIFY_FILE )
        return i18n( "File Modification");
    else
        return i18n("Date");
}

void KoDateVariable::resize()
{
    KoTextFormat * fmt = format();
    QString oldLanguage;
    if ( !fmt->language().isEmpty())
    {
         oldLanguage=KGlobal::locale()->language();
         bool changeLanguage = KGlobal::locale()->setLanguage( fmt->language() );
         KoVariable::resize();
         if ( changeLanguage )
             KGlobal::locale()->setLanguage( oldLanguage );
    }
    else
        KoVariable::resize();
}

void KoDateVariable::recalc()
{
    if ( m_subtype == VST_DATE_CURRENT )
        m_varValue = QDateTime::currentDateTime().addDays(m_correctDate);
    else if ( m_subtype == VST_DATE_LAST_PRINTING )
        m_varValue = m_varColl->variableSetting()->lastPrintingDate();
    else if ( m_subtype == VST_DATE_CREATE_FILE )
        m_varValue = m_varColl->variableSetting()->creationDate();
    else if ( m_subtype == VST_DATE_MODIFY_FILE )
        m_varValue = m_varColl->variableSetting()->modificationDate();
    else
    {
        // Only if never set before (i.e. upon insertion)
        if ( m_varValue.isNull() )
            m_varValue = QDateTime::currentDateTime().addDays(m_correctDate);
    }
    resize();
}

void KoDateVariable::saveVariable( QDomElement& varElem )
{
    QDomElement elem = varElem.ownerDocument().createElement( "DATE" );
    varElem.appendChild( elem );

    QDate date = m_varValue.toDate(); // works with Date and DateTime
    date = date.addDays( -m_correctDate );//remove correctDate value otherwise value stored is bad
    elem.setAttribute( "year", date.year() );
    elem.setAttribute( "month", date.month() );
    elem.setAttribute( "day", date.day() );
    elem.setAttribute( "fix", m_subtype == VST_DATE_FIX ); // for compat
    elem.setAttribute( "correct", m_correctDate);
    elem.setAttribute( "subtype", m_subtype);
    if ( m_varValue.type() == QVariant::DateTime )
    {
        QTime time = m_varValue.toTime();
        elem.setAttribute( "hour", time.hour() );
        elem.setAttribute( "minute", time.minute() );
        elem.setAttribute( "second", time.second() );
    }
}

void KoDateVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );

    QDomElement e = elem.namedItem( "DATE" ).toElement();
    if (!e.isNull())
    {
        const bool fix = e.attribute("fix").toInt() == 1;
        if ( e.hasAttribute("correct"))
            m_correctDate = e.attribute("correct").toInt();
        if ( fix )
        {
            const int y = e.attribute("year").toInt();
            const int month = e.attribute("month").toInt();
            const int d = e.attribute("day").toInt();
            const int h = e.attribute("hour").toInt();
            const int min = e.attribute("minute").toInt();
            const int s = e.attribute("second").toInt();
            const int ms = e.attribute("msecond").toInt();
            QDate date( y, month, d );
            date = date.addDays( m_correctDate );
            const QTime time( h, min, s, ms );
            if (time.isValid())
                m_varValue = QVariant ( QDateTime( date, time ) );
            else
                m_varValue = QVariant( date );
        }
        //old date variable format
        m_subtype = fix ? VST_DATE_FIX : VST_DATE_CURRENT;
        if ( e.hasAttribute( "subtype" ))
            m_subtype = e.attribute( "subtype").toInt();
    }
}

void KoDateVariable::saveOasis( KoXmlWriter& writer, KoSavingContext& context ) const
{
    switch( m_subtype )
    {
    case VST_DATE_FIX:
    case VST_DATE_CURRENT:
        writer.startElement( "text:date" );
        if ( m_subtype == VST_DATE_FIX )
        {
            writer.addAttribute( "text:date-value", m_varValue.toDate().toString( Qt::ISODate) );
            writer.addAttribute( "text:fixed", "true" );
        }
        break;
    case VST_DATE_LAST_PRINTING:
        writer.startElement( "text:print-date" );
        break;
    case VST_DATE_CREATE_FILE:
        writer.startElement( "text:creation-date" );
        break;
    case VST_DATE_MODIFY_FILE:
        writer.startElement( "text:modification-date" );
        break;
    }
    QString value(  m_varFormat->formatProperties() );
    bool klocaleFormat = false;
    if ( value.toLower() == "locale" ||
         value.isEmpty() ||
         value.toLower() == "localeshort" ||
         value.toLower() == "localedatetime" ||
         value.toLower() == "localedatetimeshort" )
    {
        if ( value.toLower() == "locale" || value.isEmpty())
            value =  KGlobal::locale()->dateFormat();
        else if ( value.toLower() == "localeshort" )
            value = KGlobal::locale()->dateFormatShort();
        else if ( value.toLower() == "localedatetime" )
            value =  QString( "%1 %2" ).arg( KGlobal::locale()->dateFormat() ).arg( KGlobal::locale()->timeFormat() );
        else if ( value.toLower() == "localedatetimeshort" )
            value =  QString( "%1 %2" ).arg( KGlobal::locale()->dateFormatShort() ).arg( KGlobal::locale()->timeFormat() );
        klocaleFormat = true;
    }
    writer.addAttribute( "style:data-style-name", KoOasisStyles::saveOasisDateStyle(context.mainStyles(), value, klocaleFormat ) );

    if ( m_correctDate != 0 )
        writer.addAttribute( "text:date-adjust", daysToISODuration( m_correctDate ) );
    writer.endElement();
}

void KoDateVariable::loadOasis( const QDomElement &elem, KoOasisContext& /*context*/ )
{
    const QString localName( elem.localName() );
    if ( localName == "date" ) // current (or fixed) date
    {
        // Standard form of the date is in text:date-value. Example: 2004-01-21T10:57:05
        const QString dateValue = elem.attributeNS( KoXmlNS::text, "date-value", QString::null);
        QDateTime dt;
#warning "kde4 port it"
        //if ( !dateValue.isEmpty() ) // avoid QDate warning
            //dt = QDate::fromString(dateValue, Qt::ISODate);

        bool fixed = (elem.hasAttributeNS( KoXmlNS::text, "fixed") && elem.attributeNS( KoXmlNS::text, "fixed", QString::null)=="true");
        if (!dt.isValid())
            fixed = false; // OOo docs say so: not valid = current datetime
        if ( fixed )
            m_varValue = QVariant( dt );
        m_subtype = fixed ? VST_DATE_FIX : VST_DATE_CURRENT;
    }
    // For all those the value of the date will be retrieved from meta.xml
    else if ( localName.startsWith( "print" ) )
        m_subtype = VST_DATE_LAST_PRINTING;
    else if ( localName.startsWith( "creation" ) )
        m_subtype = VST_DATE_CREATE_FILE;
    else if ( localName.startsWith( "modification" ) )
        m_subtype = VST_DATE_MODIFY_FILE;
    const QString adjustStr = elem.attributeNS( KoXmlNS::text, "date-adjust", QString::null );
    if ( !adjustStr.isEmpty() )
        m_correctDate = ISODurationToDays( adjustStr );
}

QStringList KoDateVariable::actionTexts()
{
    QStringList lst;
    lst << i18n( "Current Date (fixed)" );
    lst << i18n( "Current Date (variable)" );
    lst << i18n( "Date of Last Printing" );
    lst << i18n( "Date of File Creation" );
    lst << i18n( "Date of File Modification" );
    return lst;
}

QStringList KoDateVariable::subTypeList()
{
    return KoDateVariable::actionTexts();
}

QString KoDateVariable::defaultFormat()
{
    return QString("DATE") + "locale";
}

QString KoDateVariable::formatStr(int & correct)
{
    QString string;
    QStringList stringList;
    KDialogBase* dialog=new KDialogBase(0, 0, true, i18n("Date Format"), KDialogBase::Ok|KDialogBase::Cancel);
    dialog->setWindowTitle( i18nc( "DateFormat", "Format of Date Variable" ) );
    DateFormatWidget* widget=new DateFormatWidget(dialog);
    int count=0;
    dialog->setMainWidget(widget);
    KConfig* config = KoGlobal::kofficeConfig();
    if( config->hasGroup("Date format history") )
    {
        KConfigGroup configGroup( config, "Date format history");
        const int noe=configGroup.readNumEntry("Number Of Entries", 5);
        for(int i=0;i<noe;i++)
        {
            QString num;
            num.setNum(i);
            const QString tmpString(configGroup.readEntry("Last Used"+num,QString()));
            if(tmpString.startsWith("locale"))
                continue;
            else if(stringList.contains(tmpString))
                continue;
            else if(!tmpString.isEmpty())
            {
                stringList.append(tmpString);
                count++;
            }
        }

    }
    if(!stringList.isEmpty())
    {
        widget->combo1()->addItem("---");
        widget->combo1()->addItems(stringList);
    }
    if(false) { // ### TODO: select the last used item
        QComboBox *combo= widget->combo1();
        combo->setCurrentIndex(combo->count() -1);
        widget->updateLabel();
    }

    if(dialog->exec()==QDialog::Accepted)
    {
        string = widget->resultString().toUtf8();
        correct = widget->correctValue();
    }
    else
    {
        delete dialog;
        return 0;
    }
    config->setGroup("Date format history");
    stringList.removeAll(string);
    stringList.prepend(string);
    for(int i=0;i<=count;i++)
    {
        QString num;
        num.setNum(i);
#warning "kde4: port it"
        //configGroup.writeEntry("Last Used"+num, stringList[i]);
    }
    config->sync();
    delete dialog;
    return QString("DATE") + string;
}

/******************************************************************/
/* Class: KoTimeVariable                                          */
/******************************************************************/
KoTimeVariable::KoTimeVariable( KoTextDocument *textdoc, short int subtype, KoVariableFormat *varFormat, KoVariableCollection *_varColl, int _correct)
    : KoVariable( textdoc, varFormat,_varColl ), m_subtype( subtype ), m_correctTime( _correct)
{
}

QString KoTimeVariable::fieldCode()
{
    return (m_subtype == VST_TIME_FIX)?i18n("Time (Fixed)"):i18n("Time");
}


void KoTimeVariable::resize()
{
    KoTextFormat * fmt = format();
    if ( !fmt->language().isEmpty() )
    {
        QString oldLanguage = KGlobal::locale()->language();
        bool changeLanguage = KGlobal::locale()->setLanguage( fmt->language() );
        KoVariable::resize();
        if ( changeLanguage )
            KGlobal::locale()->setLanguage( oldLanguage );
    }
    else
        KoVariable::resize();
}

void KoTimeVariable::recalc()
{
    if ( m_subtype == VST_TIME_CURRENT )
        m_varValue = QVariant( QTime::currentTime().addSecs(60*m_correctTime));
    else
    {
        // Only if never set before (i.e. upon insertion)
        if ( m_varValue.toTime().isNull() )
            m_varValue = QVariant( QTime::currentTime().addSecs(60*m_correctTime));
    }
    resize();
}


void KoTimeVariable::saveVariable( QDomElement& parentElem )
{
    QDomElement elem = parentElem.ownerDocument().createElement( "TIME" );
    parentElem.appendChild( elem );

    QTime time = m_varValue.toTime();
    time = time.addSecs(-60*m_correctTime);
    elem.setAttribute( "hour", time.hour() );
    elem.setAttribute( "minute", time.minute() );
    elem.setAttribute( "second", time.second() );
    elem.setAttribute( "msecond", time.msec() );
    elem.setAttribute( "fix", m_subtype == VST_TIME_FIX );
    elem.setAttribute( "correct", m_correctTime );
}

void KoTimeVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );

    QDomElement e = elem.namedItem( "TIME" ).toElement();
    if (!e.isNull())
    {
        int h = e.attribute("hour").toInt();
        int m = e.attribute("minute").toInt();
        int s = e.attribute("second").toInt();
        int ms = e.attribute("msecond").toInt();
        int correct = 0;
        if ( e.hasAttribute("correct"))
            correct=e.attribute("correct").toInt();
        bool fix = static_cast<bool>( e.attribute("fix").toInt() );
        if ( fix )
        {
            QTime time;
            time.setHMS( h, m, s, ms );
            time = time.addSecs( 60*m_correctTime );
            m_varValue = QVariant( time);

        }
        m_subtype = fix ? VST_TIME_FIX : VST_TIME_CURRENT;
        m_correctTime = correct;
    }
}

void KoTimeVariable::loadOasis( const QDomElement &elem, KoOasisContext& /*context*/ )
{
    const QString localName( elem.localName() );
    Q_ASSERT( localName == "time" ); // caller checked for it
    if ( localName == "time" ) // current (or fixed) time
    {
        // Use QDateTime to work around a possible problem of QTime::fromString in Qt 3.2.2
        QDateTime dt(QDateTime::fromString(elem.attributeNS( KoXmlNS::text, "time-value", QString::null), Qt::ISODate));

        bool fixed = (elem.hasAttributeNS( KoXmlNS::text, "fixed") && elem.attributeNS( KoXmlNS::text, "fixed", QString::null)=="true");
        if (!dt.isValid())
            fixed = false; // OOo docs say so: not valid = current datetime
        if ( fixed )
            m_varValue = QVariant( dt.time() );
        m_subtype = fixed ? VST_TIME_FIX : VST_TIME_CURRENT;
        QString adjustStr = elem.attributeNS( KoXmlNS::text, "time-adjust", QString::null );
        if ( !adjustStr.isEmpty() )
            m_correctTime = ISODurationToMinutes( adjustStr );
    }
}

void KoTimeVariable::saveOasis( KoXmlWriter& writer, KoSavingContext& context ) const
{
    writer.startElement( "text:time" );
    if ( m_correctTime != 0 ) {
        writer.addAttribute( "text:time-adjust", minutesToISODuration( m_correctTime ) );
    }
    if (m_subtype == VST_TIME_FIX )
    {
        writer.addAttribute( "text:fixed", "true" );
        writer.addAttribute( "text:time-value", m_varValue.toTime().toString( Qt::ISODate ) );
    }

    QString value(  m_varFormat->formatProperties() );
    bool klocaleFormat = false;
    if ( value.toLower() == "locale" )
    {
        value = KGlobal::locale()->timeFormat();
        klocaleFormat = true;
    }
    writer.addAttribute( "style:data-style-name", KoOasisStyles::saveOasisTimeStyle(context.mainStyles(), m_varFormat->formatProperties(), klocaleFormat ) );
    //writer.addTextNode( /*value*/ value displayed as texte );
    //TODO save text value
    //<text:time style:data-style-name="N43" text:time-value="2004-11-11T14:42:19" text:fixed="true">02:42:19 PM</text:time>
    writer.endElement();
}


QStringList KoTimeVariable::actionTexts()
{
    QStringList lst;
    lst << i18n( "Current Time (fixed)" );
    lst << i18n( "Current Time (variable)" );
    return lst;
}

QStringList KoTimeVariable::subTypeList()
{
    return KoTimeVariable::actionTexts();
}

QString KoTimeVariable::formatStr(int & _correct)
{
    QString string;
    QStringList stringList;
    KDialogBase* dialog=new KDialogBase(0, 0, true, i18n("Time Format"), KDialogBase::Ok|KDialogBase::Cancel);
    dialog->setWindowTitle( i18nc( "TimeFormat", "This Dialog Allows You to Set the Format of the Time Variable" ) );
    TimeFormatWidget* widget=new TimeFormatWidget(dialog);
    dialog->setMainWidget(widget);
    KConfig* config = KoGlobal::kofficeConfig();
    int count=0;
    if( config->hasGroup("Time format history") )
    {
        KConfigGroup configGroup( config, "Time format history" );
        const int noe=configGroup.readNumEntry("Number Of Entries", 5);
        for(int i=0;i<noe;i++)
        {
            QString num;
            num.setNum(i);
            QString tmpString(configGroup.readEntry("Last Used"+num,QString()));
            if(tmpString.startsWith("locale"))
                continue;
            else if(stringList.contains(tmpString))
                continue;
            else if(!tmpString.isEmpty())
            {
                stringList.append(tmpString);
                count++;
            }
        }
    }
    if(!stringList.isEmpty())
    {
        widget->combo1()->addItem("---");
        widget->combo1()->addItems(stringList);
    }
    if(false) // ### TODO: select the last used item
    {
        QComboBox *combo= widget->combo1();
        combo->setCurrentIndex( combo->count() -1 );
    }
    if(dialog->exec()==QDialog::Accepted)
    {
        string = widget->resultString().toUtf8();
        _correct = widget->correctValue();
    }
    else
    {
        delete dialog;
        return 0;
    }
    config->setGroup("Time format history");
    stringList.removeAll( string );
    stringList.prepend(string);
    for(int i=0;i<=count;i++)
    {
        QString num;
        num.setNum(i);
#warning "kde4 port it"
        //configGroup.writeEntry("Last Used"+num, stringList[i]);
    }
    config->sync();
    delete dialog;
    return QString("TIME"+string );
}

QString KoTimeVariable::defaultFormat()
{
    return QString(QString("TIME")+QString("locale") );
}


/******************************************************************/
/* Class: KoCustomVariable                                        */
/******************************************************************/
KoCustomVariable::KoCustomVariable( KoTextDocument *textdoc, const QString &name, KoVariableFormat *varFormat, KoVariableCollection *_varColl )
    : KoVariable( textdoc, varFormat,_varColl )
{
    m_varValue = QVariant( name );
}

QString KoCustomVariable::fieldCode()
{
    return i18n("Custom Variable");
}

QString KoCustomVariable::text(bool realValue)
{
    if (m_varColl->variableSetting()->displayFieldCode()&&!realValue)
        return fieldCode();
    else
        return value();
} // use a format when they are customizable



void KoCustomVariable::saveVariable( QDomElement& parentElem )
{
    QDomElement elem = parentElem.ownerDocument().createElement( "CUSTOM" );
    parentElem.appendChild( elem );
    elem.setAttribute( "name", m_varValue.toString() );
    elem.setAttribute( "value", value() );
}

void KoCustomVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );
    QDomElement e = elem.namedItem( "CUSTOM" ).toElement();
    if (!e.isNull())
    {
        m_varValue = QVariant (e.attribute( "name" ));
        setValue( e.attribute( "value" ) );
    }
}

void KoCustomVariable::loadOasis( const QDomElement &elem, KoOasisContext& /*context*/ )
{
    const QString localName( elem.localName() );
    // We treat all those the same. For OO/OpenDocument the difference is that
    // - user-field-get is related to text:user-field-decls in <body>
    // - variable-set is related to variable-decls (defined in <body>);
    //                 its value can change in the middle of the document.
    // - user-defined is related to meta:user-defined in meta.xml
    if ( localName == "variable-set"
         || localName == "user-defined"
        || localName == "user-field-get" ) {
        m_varValue = elem.attributeNS( KoXmlNS::text, "name", QString::null );
        setValue( elem.text() );
    }
}

void KoCustomVariable::saveOasis( KoXmlWriter& writer, KoSavingContext& /*context*/ ) const
{
    //TODO save value into meta:user-defined
    writer.startElement( "text:user-field-get" ); //see 6.3.6
    writer.addAttribute( "text:name", m_varValue.toString() );
    writer.addTextNode( value() );
    writer.endElement();
}

QString KoCustomVariable::value() const
{
    return m_varColl->getVariableValue( m_varValue.toString() );
}

void KoCustomVariable::setValue( const QString &v )
{
    m_varColl->setVariableValue( m_varValue.toString(), v );
}

QStringList KoCustomVariable::actionTexts()
{
    return QStringList( i18n( "Custom..." ) );
}

void KoCustomVariable::recalc()
{
    resize();
}

/******************************************************************/
/* Class: KoMailMergeVariable                                  */
/******************************************************************/
KoMailMergeVariable::KoMailMergeVariable( KoTextDocument *textdoc, const QString &name, KoVariableFormat *varFormat,KoVariableCollection *_varColl )
    : KoVariable( textdoc, varFormat, _varColl )
{
    m_varValue = QVariant ( name );
}

QString KoMailMergeVariable::fieldCode()
{
    return i18n("Mail Merge");
}

void KoMailMergeVariable::loadOasis( const QDomElement &/*elem*/, KoOasisContext& /*context*/ )
{
    // TODO
}

void KoMailMergeVariable::saveOasis( KoXmlWriter& /*writer*/, KoSavingContext& /*context*/ ) const
{
        kWarning(32500) << "Not implemented: OASIS saving of mail merge variables" << endl;
}



void KoMailMergeVariable::saveVariable( QDomElement& parentElem )
{
    QDomElement elem = parentElem.ownerDocument().createElement( "MAILMERGE" );
    parentElem.appendChild( elem );
    elem.setAttribute( "name", m_varValue.toString() );
}

void KoMailMergeVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );
    QDomElement e = elem.namedItem( "MAILMERGE" ).toElement();
    if (!e.isNull())
        m_varValue = QVariant( e.attribute( "name" ) );
}

QString KoMailMergeVariable::value() const
{
    return QString();//m_doc->getMailMergeDataBase()->getValue( m_name );
}

QString KoMailMergeVariable::text(bool /*realValue*/)
{
    // ## should use a format maybe
    QString v = value();
    if ( v == name() )
        return "<" + v + ">";
    return v;
}

QStringList KoMailMergeVariable::actionTexts()
{
    return QStringList( i18n( "&Mail Merge..." ) );
}

/******************************************************************/
/* Class: KoPageVariable                                         */
/******************************************************************/
KoPageVariable::KoPageVariable( KoTextDocument *textdoc, short int subtype, KoVariableFormat *varFormat,KoVariableCollection *_varColl )
        : KoVariable( textdoc, varFormat, _varColl ), m_subtype( subtype )
{
}

QString KoPageVariable::fieldCode()
{
    if ( m_subtype == VST_PGNUM_CURRENT )
        return i18n("Page Current Num");
    else if ( m_subtype == VST_PGNUM_TOTAL )
        return i18n("Total Page Num");
    else if ( m_subtype == VST_CURRENT_SECTION )
        return i18n("Current Section");
    else if ( m_subtype == VST_PGNUM_PREVIOUS )
        return i18n("Previous Page Number");
    else if ( m_subtype == VST_PGNUM_NEXT )
        return i18n("Next Page Number");

    else
        return i18n("Current Section");
}


void KoPageVariable::saveVariable( QDomElement& parentElem )
{
    QDomElement pgNumElem = parentElem.ownerDocument().createElement( "PGNUM" );
    parentElem.appendChild( pgNumElem );
    pgNumElem.setAttribute( "subtype", m_subtype );
    if ( m_subtype != VST_CURRENT_SECTION )
        pgNumElem.setAttribute( "value", m_varValue.toInt() );
    else
        pgNumElem.setAttribute( "value", m_varValue.toString() );
}

void KoPageVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );
    QDomElement pgNumElem = elem.namedItem( "PGNUM" ).toElement();
    if (!pgNumElem.isNull())
    {
        m_subtype = pgNumElem.attribute("subtype").toInt();
        // ### This could use the format...
        if ( m_subtype != VST_CURRENT_SECTION )
            m_varValue = QVariant(pgNumElem.attribute("value").toInt());
        else
            m_varValue = QVariant(pgNumElem.attribute("value"));
    }
}

void KoPageVariable::saveOasis( KoXmlWriter& writer, KoSavingContext& /*context*/ ) const
{
    switch( m_subtype )
    {
    case VST_PGNUM_PREVIOUS:
    case VST_PGNUM_NEXT:
    case VST_PGNUM_CURRENT:
    {
        writer.startElement( "text:page-number" );
        if ( m_subtype == VST_PGNUM_PREVIOUS )
        {
            writer.addAttribute( "text:select-page", "previous" );
        }
        else if ( m_subtype == VST_PGNUM_NEXT )
        {
            writer.addAttribute( "text:select-page", "next" );
        }
        else if ( m_subtype == VST_PGNUM_CURRENT )
        {
            writer.addAttribute( "text:select-page", "current" );
        }
        writer.addTextNode( m_varValue.toString() );
        writer.endElement();
    }
    break;
    case VST_CURRENT_SECTION:
    {
        writer.startElement( "text:chapter" );
        writer.addTextNode( m_varValue.toString() );
        writer.endElement();
    }
    break;
    case VST_PGNUM_TOTAL:
    {
        writer.startElement( "text:page-count" );
        writer.addTextNode( m_varValue.toString() );
        writer.endElement();
    }
    break;
    }
}

void KoPageVariable::loadOasis( const QDomElement &elem, KoOasisContext& /*context*/ )
{
    const QString localName( elem.localName() );
    if ( localName == "page-number" )
    {
        m_subtype = VST_PGNUM_CURRENT;

        if ( elem.hasAttributeNS( KoXmlNS::text, "select-page") )
        {
            const QString select = elem.attributeNS( KoXmlNS::text, "select-page", QString::null);
            if (select == "previous")
                m_subtype = VST_PGNUM_PREVIOUS;
            else if (select == "next")
                m_subtype = VST_PGNUM_NEXT;
        }
        // Missing: fixed, page adjustment, formatting style
        m_varValue = QVariant( elem.text().toInt() );
    }
    else if ( localName == "chapter" )
    {
        m_subtype = VST_CURRENT_SECTION;
        m_varValue = QVariant( elem.text() );
        // text:display attribute can be name, number (i.e. with prefix/suffix),
        // number-and-name, plain-number-and-name, plain-number
        // TODO: a special format class for this, so that it can be easily switched using the RMB
    }
    else if ( localName == "page-count" )
    {
        m_subtype = VST_PGNUM_TOTAL;
        m_varValue = QVariant( elem.text() );
    }
}

QStringList KoPageVariable::actionTexts()
{
    QStringList lst;
    lst << i18n( "Page Number" );
    lst << i18n( "Number of Pages" );
    lst << i18n( "Section Title" );
    lst << i18n( "Previous Page" );
    lst << i18n( "Next Page" );
    return lst;
}

QStringList KoPageVariable::subTypeList()
{
    return KoPageVariable::actionTexts();
}

void KoPageVariable::setVariableSubType( short int type )
{
    m_subtype = type;
    Q_ASSERT( m_varColl );
    KoVariableFormatCollection* fc = m_varColl->formatCollection();
    setVariableFormat((m_subtype == VST_CURRENT_SECTION) ? fc->format("STRING") : fc->format("NUMBER"));
}

/******************************************************************/
/* Class: KoFieldVariable                                         */
/******************************************************************/
KoFieldVariable::KoFieldVariable( KoTextDocument *textdoc, short int subtype, KoVariableFormat *varFormat, KoVariableCollection *_varColl ,KoDocument *_doc )
    : KoVariable( textdoc, varFormat,_varColl ), m_subtype( subtype ), m_doc(_doc)
{
}

QString KoFieldVariable::fieldCode()
{
    switch( m_subtype ) {
    case VST_FILENAME:
        return i18n("Filename");
        break;
    case VST_DIRECTORYNAME:
        return i18n("Directory Name");
        break;
    case VST_PATHFILENAME:
        return i18n("Path Filename");
        break;
    case VST_FILENAMEWITHOUTEXTENSION:
        return i18n("Filename Without Extension");
        break;
    case VST_AUTHORNAME:
        return i18n("Author Name");
        break;
    case VST_EMAIL:
        return i18n("Email");
        break;
    case VST_COMPANYNAME:
        return i18n("Company Name");
        break;
    case VST_TELEPHONE_WORK:
        return i18n("Telephone (work)");
        break;
    case VST_TELEPHONE_HOME:
        return i18n("Telephone (home)");
        break;
    case VST_FAX:
        return i18n("Fax");
        break;
    case VST_COUNTRY:
        return i18n("Country");
        break;
    case VST_POSTAL_CODE:
        return i18n("Postal Code");
        break;
    case VST_CITY:
        return i18n("City");
        break;
    case VST_STREET:
        return i18n("Street");
        break;
    case VST_AUTHORTITLE:
        return i18n("Author Title");
        break;
    case VST_TITLE:
        return i18n("Title");
        break;
    case VST_SUBJECT:
        return i18n("Subject");
        break;
    case VST_ABSTRACT:
        return i18n("Abstract");
        break;
    case VST_KEYWORDS:
        return i18n("Keywords");
        break;
    case VST_INITIAL:
        return i18n("Initials");
        break;
    }
    return i18n("Field");
}

QString KoFieldVariable::text(bool realValue)
{
    if (m_varColl->variableSetting()->displayFieldCode()&&!realValue)
        return fieldCode();
    else
        return value();
} // use a format when they are customizable


void KoFieldVariable::saveVariable( QDomElement& parentElem )
{
    //kDebug(32500) << "KoFieldVariable::saveVariable" << endl;
    QDomElement elem = parentElem.ownerDocument().createElement( "FIELD" );
    parentElem.appendChild( elem );
    elem.setAttribute( "subtype", m_subtype );
    elem.setAttribute( "value", m_varValue.toString() );
}

void KoFieldVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );
    QDomElement e = elem.namedItem( "FIELD" ).toElement();
    if (!e.isNull())
    {
        m_subtype = e.attribute( "subtype" ).toInt();
        if ( m_subtype == VST_NONE )
            kWarning() << "Field subtype of -1 found in the file !" << endl;
        m_varValue = QVariant( e.attribute( "value" ) );
    } else
        kWarning() << "FIELD element not found !" << endl;
}

void KoFieldVariable::loadOasis( const QDomElement &elem, KoOasisContext& /*context*/ )
{
    const QString localName( elem.localName() );
    if ( localName == "file-name" ) {
        const QString display = elem.attributeNS( KoXmlNS::text, "display", QString::null );
        if (display == "path")
            m_subtype = VST_DIRECTORYNAME;
        else if (display == "name")
            m_subtype = VST_FILENAMEWITHOUTEXTENSION;
        else if (display == "name-and-extension")
            m_subtype = VST_FILENAME;
        else
            m_subtype = VST_PATHFILENAME;
    }
    else if ( localName == "author-name" )
        m_subtype = VST_AUTHORNAME;
    else if ( localName == "author-initials" )
        m_subtype = VST_INITIAL;
    else if ( localName == "subject" )
        m_subtype = VST_SUBJECT;
    else if ( localName == "title" )
        m_subtype = VST_TITLE;
    else if ( localName == "description" )
        m_subtype = VST_ABSTRACT;
    else if ( localName == "keywords" )
        m_subtype = VST_KEYWORDS;

    else if ( localName == "sender-company" )
        m_subtype = VST_COMPANYNAME;
    else if ( localName == "sender-firstname" )
        ; // ## This is different from author-name, but the notion of 'sender' is unclear...
    else if ( localName == "sender-lastname" )
        ; // ## This is different from author-name, but the notion of 'sender' is unclear...
    else if ( localName == "sender-initials" )
        ; // ## This is different from author-initials, but the notion of 'sender' is unclear...
    else if ( localName == "sender-street" )
        m_subtype = VST_STREET;
    else if ( localName == "sender-country" )
        m_subtype = VST_COUNTRY;
    else if ( localName == "sender-postal-code" )
        m_subtype = VST_POSTAL_CODE;
    else if ( localName == "sender-city" )
        m_subtype = VST_CITY;
    else if ( localName == "sender-title" )
        m_subtype = VST_AUTHORTITLE; // Small hack (it's supposed to be about the sender, not about the author)
    else if ( localName == "sender-position" )
        m_subtype = VST_AUTHORPOSITION;
    else if ( localName == "sender-phone-private" )
        m_subtype = VST_TELEPHONE_HOME;
    else if ( localName == "sender-phone-work" )
        m_subtype = VST_TELEPHONE_WORK;
    else if ( localName == "sender-fax" )
        m_subtype = VST_FAX;
    else if ( localName == "sender-email" )
        m_subtype = VST_EMAIL;

    m_varValue = QVariant( elem.text() );
}

void KoFieldVariable::saveOasis( KoXmlWriter& writer, KoSavingContext& /*context*/ ) const
{
    switch( m_subtype )
    {
    case VST_NONE:
        break;
    case VST_FILENAME:
        writer.startElement( "text:file-name" );
        writer.addAttribute( "text:display", "name-and-extension" );
        break;
    case VST_DIRECTORYNAME:
        writer.startElement( "text:file-name" );
        writer.addAttribute( "text:display", "path" );
        break;
    case VST_AUTHORNAME:
        writer.startElement( "text:author-name" );
        break;
    case VST_EMAIL:
        writer.startElement("text:sender-email" );
        break;
    case VST_COMPANYNAME:
        writer.startElement("text:sender-company" );
        break;
    case VST_PATHFILENAME:
        writer.startElement("text:display" );
        writer.addAttribute( "text:display", "pathfilename" ); // ???????? not define !
        break;
    case VST_FILENAMEWITHOUTEXTENSION:
        writer.startElement("text:display" );
        writer.addAttribute( "text:display", "name-and-extension" ); // ???????? not define !
        break;
    case VST_TELEPHONE_WORK:
        writer.startElement("text:sender-phone-work" );
        break;
    case VST_TELEPHONE_HOME:
        writer.startElement("text:sender-phone-private" );
        break;
    case VST_FAX:
        writer.startElement("text:sender-fax" );
        break;
    case VST_COUNTRY:
        writer.startElement("text:sender-country" );
        break;
    case VST_TITLE:
        writer.startElement("text:title" );
        break;
    case VST_KEYWORDS:
        writer.startElement("text:keywords" );
        break;
    case VST_SUBJECT:
        writer.startElement("text:subject" );
        break;
    case VST_ABSTRACT:
        writer.startElement("text:description" );
        break;
    case VST_POSTAL_CODE:
        writer.startElement("text:sender-postal-code" );
        break;
    case VST_CITY:
        writer.startElement("text:sender-city" );
        break;
    case VST_STREET:
        writer.startElement("text:sender-street" );
        break;
    case VST_AUTHORTITLE:
        writer.startElement("text:sender-title" );
        break;
    case VST_AUTHORPOSITION:
        writer.startElement("text:sender-position" );
        break;
    case VST_INITIAL:
        writer.startElement("text:author-initials" );
        break;
    }
    writer.addTextNode( m_varValue.toString() );
    writer.endElement();
}

void KoFieldVariable::recalc()
{
    QString value;
    switch( m_subtype ) {
        case VST_NONE:
            kWarning() << "KoFieldVariable::recalc() called with m_subtype = VST_NONE !" << endl;
            break;
        case VST_FILENAME:
            value = m_doc->url().fileName();
            break;
        case VST_DIRECTORYNAME:
            value = m_doc->url().directory();
            break;
        case VST_PATHFILENAME:
            value=m_doc->url().path();
            break;
        case VST_FILENAMEWITHOUTEXTENSION:
        {
            QString file=m_doc->url().fileName();
            int pos = file.lastIndexOf(".");
            if(pos !=-1)
                value=file.mid(0,pos);
            else
                value=file;
        }
        break;
        case VST_AUTHORNAME:
        case VST_EMAIL:
        case VST_COMPANYNAME:
        case VST_TELEPHONE_WORK:
        case VST_TELEPHONE_HOME:
        case VST_FAX:
        case VST_COUNTRY:
        case VST_POSTAL_CODE:
        case VST_CITY:
        case VST_STREET:
        case VST_AUTHORTITLE:
    case VST_AUTHORPOSITION:
        case VST_INITIAL:
        {
            KoDocumentInfo * info = m_doc->documentInfo();
            if ( !info )
                kWarning() << "Author information not found in documentInfo !" << endl;
            else
            {
                if ( m_subtype == VST_AUTHORNAME )
                    value = info->authorInfo( "creator" );
                else if ( m_subtype == VST_EMAIL )
                    value = info->authorInfo( "email" );
                else if ( m_subtype == VST_COMPANYNAME )
                    value = info->authorInfo( "company" );
                else if ( m_subtype == VST_TELEPHONE_WORK )
                    value = info->authorInfo( "telephone-work" );
                else if ( m_subtype == VST_TELEPHONE_HOME )
                    value = info->authorInfo( "telephone" );
                else if ( m_subtype == VST_FAX )
                    value = info->authorInfo( "fax" );
                else if ( m_subtype == VST_COUNTRY )
                    value = info->authorInfo( "country" );
                else if ( m_subtype == VST_POSTAL_CODE )
                    value = info->authorInfo( "postal-code" );
                else if ( m_subtype == VST_CITY )
                    value = info->authorInfo( "city" );
                else if ( m_subtype == VST_STREET )
                    value = info->authorInfo( "street" );
                else if ( m_subtype == VST_AUTHORTITLE )
                    value = info->authorInfo( "title" );
                else if ( m_subtype == VST_INITIAL )
                    value = info->authorInfo( "initial" );
                else if ( m_subtype == VST_AUTHORPOSITION )
                    value = info->authorInfo( "position" );
            }
        }
        break;
        case VST_TITLE:
        case VST_ABSTRACT:
    case VST_SUBJECT:
    case VST_KEYWORDS:
        {
            KoDocumentInfo * info = m_doc->documentInfo();

            if ( !info )
                kWarning() << "'About' page not found in documentInfo !" << endl;
            else
            {
                if ( m_subtype == VST_TITLE )
                    value = info->aboutInfo( "title" );
                else if ( m_subtype == VST_SUBJECT )
                    value = info->aboutInfo( "subject" );
                else if ( m_subtype == VST_KEYWORDS )
                    value = info->aboutInfo( "keyword" );
                else
                    value = info->aboutInfo( "comments" );
            }
        }
        break;
    }

    if (value.isEmpty())        // try the initial value
        value = m_varValue.toString();

    if (value.isEmpty())        // still empty? give up
        value = i18n("<None>");

    m_varValue = QVariant( value );

    resize();
}

QStringList KoFieldVariable::actionTexts()
{
    // NOTE: if you change here, also change fieldSubType()
    QStringList lst;
    lst << i18n( "Author Name" );
    lst << i18n( "Title" );
    lst << i18n( "Initials" );
    lst << i18n( "Position" );
    lst << i18n( "Company" );
    lst << i18n( "Email" );
    lst << i18n( "Telephone (work)");
    lst << i18n( "Telephone (private)");

    lst << i18n( "Fax");
    lst << i18n( "Street" );
    lst << i18n( "Postal Code" );
    lst << i18n( "City" );
    lst << i18n( "Country");

    lst << i18n( "Document Title" );
    lst << i18n( "Document Abstract" );
    lst << i18n( "Document Subject" );
    lst << i18n( "Document Keywords" );

    lst << i18n( "File Name" );
    lst << i18n( "File Name without Extension" );
    lst << i18n( "Directory Name" ); // is "Name" necessary ?
    lst << i18n( "Directory && File Name" );
    return lst;
}

short int KoFieldVariable::variableSubType( short int menuNumber )
{
    return fieldSubType(menuNumber);
}

KoFieldVariable::FieldSubType KoFieldVariable::fieldSubType(short int menuNumber)
{
    // NOTE: if you change here, also change actionTexts()
    FieldSubType v;
    switch (menuNumber)
    {
        case 0: v = VST_AUTHORNAME;
                break;
        case 1: v = VST_AUTHORTITLE;
                break;
        case 2: v = VST_INITIAL;
                break;
        case 3: v = VST_AUTHORPOSITION;
                break;
        case 4: v = VST_COMPANYNAME;
                break;
        case 5: v = VST_EMAIL;
                break;
        case 6: v = VST_TELEPHONE_WORK;
                break;
        case 7: v = VST_TELEPHONE_HOME;
                break;
        case 8: v = VST_FAX;
                break;
        case 9: v = VST_STREET;
                break;
        case 10: v = VST_POSTAL_CODE;
                break;
        case 11: v = VST_CITY;
                break;
        case 12: v = VST_COUNTRY;
                break;
        case 13: v = VST_TITLE;
                break;
        case 14: v = VST_ABSTRACT;
                break;
        case 15: v = VST_SUBJECT;
                break;
        case 16: v = VST_KEYWORDS;
                break;
        case 17: v = VST_FILENAME;
                break;
        case 18: v = VST_FILENAMEWITHOUTEXTENSION;
                break;
        case 19: v = VST_DIRECTORYNAME;
                break;
        case 20: v = VST_PATHFILENAME;
                break;
        default:
            v = VST_NONE;
            break;
    }
    return v;
}

QStringList KoFieldVariable::subTypeList()
{
    return KoFieldVariable::actionTexts();
}

/******************************************************************/
/* Class: KoLinkVariable                                          */
/******************************************************************/
KoLinkVariable::KoLinkVariable( KoTextDocument *textdoc, const QString & _linkName, const QString & _ulr,KoVariableFormat *varFormat,KoVariableCollection *_varColl )
    : KoVariable( textdoc, varFormat,_varColl )
    ,m_url(_ulr)
{
    m_varValue = QVariant( _linkName );
}

QString KoLinkVariable::fieldCode()
{
    return i18n("Link");
}

void KoLinkVariable::loadOasis( const QDomElement &elem, KoOasisContext& /*context*/ )
{
    if ( elem.localName() == "a" && elem.namespaceURI() == KoXmlNS::text ) {
        m_url = elem.attributeNS( KoXmlNS::xlink, "href", QString::null);
        m_varValue = QVariant(elem.text());
    }
}

void KoLinkVariable::saveOasis( KoXmlWriter& writer, KoSavingContext& /*context*/ ) const
{
    //<text:a xlink:type="simple" xlink:href="http://www.kde.org/" office:name="sdgfsdfgs">kde org wxc &lt;wxc </text:a>
    writer.startElement( "text:a" );
    writer.addAttribute( "xlink:type", "simple" );
    writer.addAttribute( "xlink:href", m_url );
    writer.addAttribute( "office:name", m_varValue.toString() );
    writer.addTextNode( m_varValue.toString() );
    writer.endElement();

}

QString KoLinkVariable::text(bool realValue)
{
    if (m_varColl->variableSetting()->displayFieldCode()&&!realValue)
        return fieldCode();
    else
        return value();
}

void KoLinkVariable::saveVariable( QDomElement& parentElem )
{
    QDomElement linkElem = parentElem.ownerDocument().createElement( "LINK" );
    parentElem.appendChild( linkElem );
    linkElem.setAttribute( "linkName", m_varValue.toString() );
    linkElem.setAttribute( "hrefName", m_url );
}

void KoLinkVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );
    QDomElement linkElem = elem.namedItem( "LINK" ).toElement();
    if (!linkElem.isNull())
    {
        m_varValue = QVariant(linkElem.attribute("linkName"));
        m_url = linkElem.attribute("hrefName");
    }
}

void KoLinkVariable::recalc()
{
    resize();
}

QStringList KoLinkVariable::actionTexts()
{
    return QStringList( i18n( "Link..." ) );
}


void KoLinkVariable::drawCustomItem( QPainter* p, int x, int y, int wpix, int hpix, int ascentpix, int /*cx*/, int /*cy*/, int /*cw*/, int /*ch*/, const QColorGroup& cg, bool selected, int offset, bool drawingShadow )
{
    KoTextFormat * fmt = format();
    KoTextZoomHandler * zh = textDocument()->paintingZoomHandler();

    bool displayLink = m_varColl->variableSetting()->displayLink();
    QFont font( fmt->screenFont( zh ) );
    if ( m_varColl->variableSetting()->underlineLink() )
        font.setUnderline( true );
    QColor textColor = displayLink ? cg.color( QColorGroup::Link ) : fmt->color();

    drawCustomItemHelper( p, x, y, wpix, hpix, ascentpix, cg, selected, offset, fmt, font, textColor, drawingShadow );
}


/******************************************************************/
/* Class: KoNoteVariable                                          */
/******************************************************************/
KoNoteVariable::KoNoteVariable( KoTextDocument *textdoc, const QString & _note,KoVariableFormat *varFormat,KoVariableCollection *_varColl )
    : KoVariable( textdoc, varFormat,_varColl )
    , m_createdNoteDate( QDate::currentDate() )
{
    m_varValue = QVariant( _note );
}

QString KoNoteVariable::fieldCode()
{
    return i18n("Note");
}

QString KoNoteVariable::createdNote() const
{
    return KGlobal::locale()->formatDate( m_createdNoteDate, false );
}

void KoNoteVariable::loadOasis( const QDomElement &elem, KoOasisContext& /*context*/ )
{
    const QString localName = elem.localName();
    QString note;
    if ( localName == "annotation" && elem.namespaceURI() == KoXmlNS::office )
    {
        QDomElement date = KoDom::namedItemNS( elem, KoXmlNS::dc, "date" );
        m_createdNoteDate = QDate::fromString( date.text(), Qt::ISODate );
        QDomNode text = KoDom::namedItemNS( elem, KoXmlNS::text, "p" );
        for ( ; !text.isNull(); text = text.nextSibling() )
        {
            if ( text.isElement() )
            {
                QDomElement t = text.toElement();
                note += t.text() + "\n";
            }
        }
    }
    m_varValue = QVariant( note  );
}

void KoNoteVariable::saveOasis( KoXmlWriter& writer, KoSavingContext& /*context*/ ) const
{
//    <office:annotation><dc:date>2004-11-10</dc:date><text:p/><text:p>---- 10/11/2004, 16:18 ----</text:p><text:p>dfgsdfsdfg</text:p><text:p>---- 10/11/2004, 16:18 ----</text:p><text:p/><text:p>---- 10/11/2004, 16:18 ----</text:p><text:p>gs</text:p><text:p>---- 10/11/2004, 16:18 ----</text:p><text:p>fg</text:p></office:annotation>
    writer.startElement( "office:annotation" );
    writer.startElement( "dc:date" );
    writer.addTextNode( m_createdNoteDate.toString(Qt::ISODate) );
    writer.endElement();
    QStringList text = m_varValue.toString().split( "\n" );
    for ( QStringList::Iterator it = text.begin(); it != text.end(); ++it ) {
        writer.startElement( "text:p" );
        writer.addTextNode( *it );
        writer.endElement();
    }
    writer.endElement();
}

void KoNoteVariable::saveVariable( QDomElement& parentElem )
{
    QDomElement linkElem = parentElem.ownerDocument().createElement( "NOTE" );
    parentElem.appendChild( linkElem );
    linkElem.setAttribute( "note", m_varValue.toString() );
}

void KoNoteVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );
    QDomElement linkElem = elem.namedItem( "NOTE" ).toElement();
    if (!linkElem.isNull())
    {
        m_varValue = QVariant(linkElem.attribute("note"));
    }
}

void KoNoteVariable::recalc()
{
    resize();
}

QStringList KoNoteVariable::actionTexts()
{
    return QStringList( i18n( "Note..." ) );
}

QString KoNoteVariable::text(bool realValue)
{
    if (m_varColl->variableSetting()->displayComment() &&
        m_varColl->variableSetting()->displayFieldCode()&&!realValue)
        return fieldCode();
    else
        //for a note return just a "space" we can look at
        //note when we "right button"
        return QString(" ");

}

void KoNoteVariable::drawCustomItem( QPainter* p, int x, int y, int wpix, int hpix, int ascentpix, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected, int offset, bool drawingShadow )
{
    if ( !m_varColl->variableSetting()->displayComment())
        return;

    KoTextFormat * fmt = format();
    //kDebug(32500) << "KoNoteVariable::drawCustomItem index=" << index() << " x=" << x << " y=" << y << endl;

    p->save();
    p->setPen( QPen( fmt->color() ) );
    if ( fmt->textBackgroundColor().isValid() )
        p->fillRect( x, y, wpix, hpix, fmt->textBackgroundColor() );
    if ( selected )
    {
        p->setPen( QPen( cg.color( QColorGroup::HighlightedText ) ) );
        p->fillRect( x, y, wpix, hpix, cg.color( QColorGroup::Highlight ) );
    }
    else if ( textDocument() && p->device()->devType() != QInternal::Printer
        && !textDocument()->dontDrawingNoteVariable())
    {
        p->fillRect( x, y, wpix, hpix, Qt::yellow);
        p->setPen( QPen( cg.color( QColorGroup::Highlight ), 0, Qt::DotLine ) );
        p->drawRect( x, y, wpix, hpix );
    }
    //call it for use drawCustomItemHelper just for draw font effect
    KoVariable::drawCustomItem( p, x, y, wpix, hpix, ascentpix, cx, cy, cw, ch, cg, selected, offset, drawingShadow );

    p->restore();
}

void KoPageVariable::setSectionTitle( const QString& _title )
{
    QString title( _title );
    if ( title.isEmpty() )
    {
        title = i18n("<No title>");
    }
    m_varValue = QVariant( title );
}


// ----------------------------------------------------------------
//                   class KoStatisticVariable


bool KoStatisticVariable::m_extendedType = false;


KoStatisticVariable::KoStatisticVariable( KoTextDocument *textdoc,
					  short int subtype,
					  KoVariableFormat *varFormat,
					  KoVariableCollection *_varColl )
    : KoVariable( textdoc, varFormat, _varColl ),
      m_subtype( subtype )
{
}


QStringList KoStatisticVariable::actionTexts()
{
    QStringList lst;
    lst << i18n( "Number of Words" );
    lst << i18n( "Number of Sentences" );
    lst << i18n( "Number of Lines" );
    lst << i18n( "Number of Characters" );
    lst << i18n( "Number of Non-Whitespace Characters" );
    lst << i18n( "Number of Syllables" );
    lst << i18n( "Number of Frames" );
    lst << i18n( "Number of Embedded Objects" );
    lst << i18n( "Number of Pictures" );
    if (  m_extendedType )
        lst << i18n( "Number of Tables" );
    return lst;
}


void KoStatisticVariable::setVariableSubType( short int subtype )
{
    m_subtype = subtype;
    Q_ASSERT( m_varColl );
    KoVariableFormatCollection* fc = m_varColl->formatCollection();
    setVariableFormat(fc->format("NUMBER") );
}


QStringList KoStatisticVariable::subTypeList()
{
    return KoStatisticVariable::actionTexts();
}


void KoStatisticVariable::saveVariable( QDomElement& varElem )
{
    QDomElement  elem = varElem.ownerDocument().createElement( "STATISTIC" );
    varElem.appendChild( elem );

    elem.setAttribute( "type",  QString::number(m_subtype) );
    elem.setAttribute( "value", QString::number(m_varValue.toInt()) );
}


void KoStatisticVariable::load( QDomElement &elem )
{
    KoVariable::load( elem );

    QDomElement e = elem.namedItem( "STATISTIC" ).toElement();
    if ( !e.isNull() ) {
	// FIXME: Error handling.
	m_subtype  = e.attribute( "type" ).toInt();
	m_varValue = e.attribute( "value" ).toInt();
    }
}


void KoStatisticVariable::loadOasis( const QDomElement &elem, KoOasisContext& /*context*/ )
{
    const QString localName( elem.localName() );
    if ( localName == "object-count" )
    {
        m_subtype = VST_STATISTIC_NB_EMBEDDED;
        m_varValue = QVariant( elem.text().toInt() );
    }
    else if ( localName == "table-count" )
    {
        m_subtype = VST_STATISTIC_NB_TABLE;
        m_varValue = QVariant( elem.text().toInt() );
    }
    else if ( localName == "picture-count" )
    {
        m_subtype = VST_STATISTIC_NB_PICTURE;
        m_varValue = QVariant( elem.text().toInt() );
    }
    else if ( localName == "word-count" )
    {
        m_subtype = VST_STATISTIC_NB_WORD;
        m_varValue = QVariant( elem.text().toInt() );
    }
    else if ( localName == "character-count" )
    {
        m_subtype = VST_STATISTIC_NB_CHARACTERE;
        m_varValue = QVariant( elem.text().toInt() );
    }
    else if ( localName == "frame-count" )
    {
        m_subtype = VST_STATISTIC_NB_FRAME;
        m_varValue = QVariant( elem.text().toInt() );
    }
    else if ( localName == "line-count" )
    {
        m_subtype = VST_STATISTIC_NB_LINES;
        m_varValue = QVariant( elem.text().toInt() );
    }
    else if ( localName == "sentence-count" )
    {
        m_subtype = VST_STATISTIC_NB_SENTENCE;
        m_varValue = QVariant( elem.text().toInt() );
    }
    else if ( localName == "non-whitespace-character-count" )
    {
        m_subtype = VST_STATISTIC_NB_NON_WHITESPACE_CHARACTERE;
        m_varValue = QVariant( elem.text().toInt() );
    }
    else if ( localName == "syllable-count" )
    {
        m_subtype = VST_STATISTIC_NB_SYLLABLE;
        m_varValue = QVariant( elem.text().toInt() );
    }
    //TODO other copy
}

void KoStatisticVariable::saveOasis( KoXmlWriter& writer, KoSavingContext& /*context*/ ) const
{
    switch( m_subtype )
    {
    case VST_STATISTIC_NB_EMBEDDED:
        writer.startElement( "text:object-count" );
        writer.addTextNode( QString::number( m_varValue.toInt() ) );
        writer.endElement();
        break;
    case VST_STATISTIC_NB_TABLE:
        writer.startElement( "text:table-count" );
        writer.addTextNode( QString::number( m_varValue.toInt() ) );
        writer.endElement();
        break;
    case VST_STATISTIC_NB_PICTURE:
        writer.startElement( "text:picture-count" );
        writer.addTextNode( QString::number( m_varValue.toInt() ) );
        writer.endElement();
        break;
    case VST_STATISTIC_NB_FRAME:
        //TODO verify that it's implemented into oasis file format
        writer.startElement( "text:frame-count" );
        writer.addTextNode( QString::number( m_varValue.toInt() ) );
        writer.endElement();
        break;
    case VST_STATISTIC_NB_WORD:
        writer.startElement( "text:word-count" );
        writer.addTextNode( QString::number( m_varValue.toInt() ) );
        writer.endElement();
        break;
    case VST_STATISTIC_NB_SENTENCE:
        //TODO verify that it's implemented into oasis file format
        writer.startElement( "text:sentence-count" );
        writer.addTextNode( QString::number( m_varValue.toInt() ) );
        writer.endElement();
        break;
    case VST_STATISTIC_NB_CHARACTERE:
        writer.startElement( "text:character-count" );
        writer.addTextNode( QString::number( m_varValue.toInt() ) );
        writer.endElement();
        break;
    case VST_STATISTIC_NB_LINES:
        //TODO verify that it's implemented into oasis file format
        writer.startElement( "text:line-count" );
        writer.addTextNode( QString::number( m_varValue.toInt() ) );
        writer.endElement();
        break;
    case VST_STATISTIC_NB_NON_WHITESPACE_CHARACTERE:
        //TODO verify that it's implemented into oasis file format
        writer.startElement( "text:non-whitespace-character-count" );
        writer.addTextNode( QString::number( m_varValue.toInt() ) );
        writer.endElement();
        break;
    case VST_STATISTIC_NB_SYLLABLE:
        //TODO verify that it's implemented into oasis file format
        writer.startElement( "text:syllable-count" );
        writer.addTextNode( QString::number( m_varValue.toInt() ) );
        writer.endElement();
        break;
    }
}

QString KoStatisticVariable::fieldCode()
{
    if ( m_subtype == VST_STATISTIC_NB_FRAME )
    {
        return i18n( "Number of Frames" );
    }
    else if( m_subtype == VST_STATISTIC_NB_PICTURE )
    {
        return i18n( "Number of Pictures" );
    }
    else if( m_subtype == VST_STATISTIC_NB_TABLE )
    {
        return i18n( "Number of Tables" );
    }
    else if( m_subtype == VST_STATISTIC_NB_EMBEDDED )
    {
        return i18n( "Number of Embedded Objects" );
    }
    else if( m_subtype == VST_STATISTIC_NB_WORD )
    {
        return i18n( "Number of Words" );
    }
    else if( m_subtype == VST_STATISTIC_NB_SENTENCE )
    {
        return i18n( "Number of Sentences" );
    }
    else if( m_subtype == VST_STATISTIC_NB_LINES )
    {
        return i18n( "Number of Lines" );
    }
    else if ( m_subtype == VST_STATISTIC_NB_CHARACTERE )
    {
        return i18n( "Number of Characters" );
    }
    else if ( m_subtype == VST_STATISTIC_NB_NON_WHITESPACE_CHARACTERE )
    {
        return i18n( "Number of Non-Whitespace Characters" );
    }
    else if ( m_subtype == VST_STATISTIC_NB_SYLLABLE )
    {
        return i18n( "Number of Syllables" );
    }
    else
        return i18n( "Number of Frames" );
}


