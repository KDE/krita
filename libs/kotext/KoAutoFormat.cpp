/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
                 2001       Sven Leiber         <s.leiber@web.de>

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

#include "KoAutoFormat.h"

#include "KoTextObject.h"
#include "KoTextParag.h"
#include "KoVariable.h"
#include "KoParagCounter.h"
#include <KoDocument.h>
#include <KoSearchDia.h>
#include <KoGlobal.h>

#include <kdeversion.h>
#include <kdebug.h>
#include <klocale.h>
#include <kinstance.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kcommand.h>
//#include <KoTextFormat.h>
#include <kcompletion.h>
#include <kcalendarsystem.h>

#include <qfile.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qdom.h>
#include <qregexp.h>
//Added by qt3to4:
#include <QTextStream>
#include <Q3Frame>
#include <QMouseEvent>


KoCompletionBox::KoCompletionBox( QWidget * parent, const char * name, Qt::WFlags f)
  : QLabel(parent,name,f)
{
  setBackgroundColor(QColor("#FFFFE6"));
  setFocusPolicy(Qt::NoFocus);
  setFrameShape(Q3Frame::Box);
}

KoCompletionBox::~KoCompletionBox()
{
}

void KoCompletionBox::mousePressEvent( QMouseEvent *)
{
  hide();
}

QString& KoCompletionBox::lastWord()
{
  return m_lastWord;
}

void KoCompletionBox::setLastWord( QString const &lastword)
{
  m_lastWord = lastword;
}

KoAutoFormatEntry::KoAutoFormatEntry(const QString& replace)
    : m_replace( replace )
{
    m_formatOptions= 0L;
}

KoAutoFormatEntry::~KoAutoFormatEntry()
{
    delete m_formatOptions;
    m_formatOptions=0L;
}

KoSearchContext *KoAutoFormatEntry::formatEntryContext() const
{
    return m_formatOptions;
}

void KoAutoFormatEntry::createNewEntryContext()
{
    if ( !m_formatOptions )
    {
        m_formatOptions = new KoSearchContext();
    }
}

void KoAutoFormatEntry::setFormatEntryContext( KoSearchContext *_cont )
{
    delete m_formatOptions;
    m_formatOptions=_cont;
}

void KoAutoFormatEntry::clearFormatEntryContext( )
{
    delete m_formatOptions;
    m_formatOptions = 0L;
}


/******************************************************************/
/* Class: KoAutoFormat						  */
/******************************************************************/
KoAutoFormat::KoAutoFormat( KoDocument *_doc, KoVariableCollection *_varCollection, KoVariableFormatCollection *_varFormatCollection )
    : m_doc( _doc ),
      m_varCollection(_varCollection),
      m_varFormatCollection(_varFormatCollection),
      m_autoFormatLanguage( QString::null),
      m_configRead( false ),
      m_convertUpperCase( false ), m_convertUpperUpper( false ),
      m_advancedAutoCorrect( true ),
      m_autoDetectUrl( false ),
      m_ignoreDoubleSpace( false ),
      m_removeSpaceBeginEndLine( false ),
      m_useBulletStyle(false),
      m_autoChangeFormat(false),
      m_autoReplaceNumber(false),
      m_useAutoNumberStyle(false),
      m_completion(false),
      m_toolTipCompletion(false),
      m_completionAppendSpace(false),
      m_addCompletionWord(true),
      m_includeTwoUpperLetterException(false),
      m_includeAbbreviation(false),
      m_ignoreUpperCase(false),
      m_bAutoFormatActive(true),
      m_bAutoSuperScript( false ),
      m_bAutoCorrectionWithFormat( false ),
      m_bCapitalizeNameOfDays( false ),
      m_wordInserted( false ),
      m_bulletStyle(),
      m_typographicSimpleQuotes(),
      m_typographicDoubleQuotes(),
      m_typographicDefaultDoubleQuotes(),
      m_typographicDefaultSimpleQuotes(),
      m_listCompletion( new KCompletion ),
      m_entries(17,false),
      m_allLanguages(17,false),
      m_superScriptEntries(),
      m_upperCaseExceptions(),
      m_twoUpperLetterException(),
      m_maxFindLength( 0 ),
      m_minCompletionWordLength( 5 ),
      m_nbMaxCompletionWord( 500 ),
      m_countMaxWords(0),
      m_completionBox(0),
      m_keyCompletionAction( Enter )

{
    //load once this list not each time that we "readConfig"
    loadListOfWordCompletion();
    m_listCompletion->setIgnoreCase( true );
    updateMaxWords();
    KLocale klocale(m_doc->instance()->instanceName());
    for (int i = 1; i <=7; i++)
    {
        m_cacheNameOfDays.append(klocale.calendar()->weekDayName( i ).lower());
    }
}

KoAutoFormat::KoAutoFormat( const KoAutoFormat& format )
    : m_doc( format.m_doc ),
      m_varCollection( format.m_varCollection ),
      m_varFormatCollection( format.m_varFormatCollection ),
      m_autoFormatLanguage( format.m_autoFormatLanguage),
      m_configRead( format.m_configRead ),
      m_convertUpperCase( format.m_convertUpperCase ),
      m_convertUpperUpper( format.m_convertUpperUpper ),
      m_advancedAutoCorrect( format.m_advancedAutoCorrect ),
      m_autoDetectUrl( format.m_autoDetectUrl ),
      m_ignoreDoubleSpace( format.m_ignoreDoubleSpace ),
      m_removeSpaceBeginEndLine( format.m_removeSpaceBeginEndLine ),
      m_useBulletStyle( format.m_useBulletStyle ),
      m_autoChangeFormat( format.m_autoChangeFormat ),
      m_autoReplaceNumber( format.m_autoReplaceNumber ),
      m_useAutoNumberStyle( format.m_useAutoNumberStyle ),
      m_completion( format.m_completion ),
      m_toolTipCompletion( format.m_toolTipCompletion),
      m_completionAppendSpace( format.m_completionAppendSpace ),
      m_addCompletionWord( format.m_addCompletionWord ),
      m_includeTwoUpperLetterException( format.m_includeTwoUpperLetterException ),
      m_includeAbbreviation( format.m_includeAbbreviation ),
      m_ignoreUpperCase( format.m_ignoreUpperCase ),
      m_bAutoFormatActive( format.m_bAutoFormatActive ),
      m_bAutoSuperScript( format.m_bAutoSuperScript ),
      m_bAutoCorrectionWithFormat( format.m_bAutoCorrectionWithFormat),
      m_bCapitalizeNameOfDays( format.m_bCapitalizeNameOfDays),
      m_bulletStyle( format.m_bulletStyle ),
      m_typographicSimpleQuotes( format.m_typographicSimpleQuotes ),
      m_typographicDoubleQuotes( format.m_typographicDoubleQuotes ),
      m_typographicDefaultDoubleQuotes( format.m_typographicDefaultDoubleQuotes),
      m_typographicDefaultSimpleQuotes( format.m_typographicDefaultSimpleQuotes),
      m_listCompletion( 0L ), // don't copy it!
      m_entries(17,false ),//don't copy it.
      m_allLanguages(17,false), //don't copy it
      m_superScriptEntries ( format.m_superScriptEntries ),
      m_upperCaseExceptions( format.m_upperCaseExceptions ),
      m_twoUpperLetterException( format.m_twoUpperLetterException ),
      m_maxFindLength( format.m_maxFindLength ),
      m_minCompletionWordLength( format.m_minCompletionWordLength ),
      m_nbMaxCompletionWord( format.m_nbMaxCompletionWord ),
      m_cacheNameOfDays( format.m_cacheNameOfDays),
      m_completionBox(0),
      m_keyCompletionAction( format.m_keyCompletionAction )
{
    //m_listCompletion=new KCompletion();
    //m_listCompletion->setItems( autoFormat.listCompletion() );
    //copyAutoFormatEntries( autoFormat );
}

KoAutoFormat::~KoAutoFormat()
{
    delete m_listCompletion;
    m_entries.setAutoDelete( true );
    m_entries.clear();
    m_allLanguages.setAutoDelete( true );
    m_allLanguages.clear();
}

void KoAutoFormat::updateMaxWords()
{
    QStringList list = m_listCompletion->items();
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
    {
        QString tmp = *it;
        int maxword = 1;

        for (int i=0; i < (int)tmp.length(); i++)
                if ( tmp.at(i).isSpace() || tmp.at(i).isPunct() )
                        maxword++;
        if (maxword >  m_countMaxWords )
                m_countMaxWords = maxword;
    }
    kDebug() << "m_countMaxWords: " << m_countMaxWords << endl;
}

void KoAutoFormat::loadListOfWordCompletion()
{
    KConfig* config = KoGlobal::kofficeConfig();
    KConfigGroup configGroup( config, "Completion Word" );
    m_listCompletion->insertItems(configGroup.readListEntry( "list" ));
}

void KoAutoFormat::readConfig(bool force)
{
    // Read the autoformat configuration
    // This is done on demand (when typing the first char, or when opening the config dialog)
    // so that loading is faster and to avoid doing it for readonly documents.
    if ( m_configRead && !force )
        return;
    KConfig* config = KoGlobal::kofficeConfig();
    KConfigGroup configGroup( config, "AutoFormat" );
    //when we force don't load format language.
    if ( !force)
        m_autoFormatLanguage = configGroup.readEntry("formatLanguage", QString());

    m_convertUpperCase = configGroup.readEntry( "ConvertUpperCase", false );
    m_convertUpperUpper = configGroup.readEntry( "ConvertUpperUpper", false );
    m_includeTwoUpperLetterException = configGroup.readEntry( "includeTwoLetterException", false );
    m_includeAbbreviation = configGroup.readEntry( "includeAbbreviation", false );

    m_advancedAutoCorrect = configGroup.readEntry( "AdvancedAutocorrect", true );
    m_bAutoCorrectionWithFormat = configGroup.readEntry( "AutoCorrectionWithFormat",false );
    m_bCapitalizeNameOfDays = configGroup.readEntry( "CapitalizeNameOfDays",false );

    m_autoDetectUrl = configGroup.readEntry("AutoDetectUrl",false);
    m_ignoreDoubleSpace = configGroup.readEntry("IgnoreDoubleSpace", true);
    m_removeSpaceBeginEndLine = configGroup.readEntry("RemoveSpaceBeginEndLine", true);

    m_useBulletStyle = configGroup.readEntry("UseBulletStyle",false);
    QString tmp = configGroup.readEntry( "BulletStyle", "" );
    m_bulletStyle = tmp.isEmpty() ? QChar() : tmp[0];

    m_autoChangeFormat = configGroup.readEntry( "AutoChangeFormat", false );

    m_autoReplaceNumber = configGroup.readEntry( "AutoReplaceNumber", true );

    m_useAutoNumberStyle = configGroup.readEntry( "AutoNumberStyle", false );


    QString beginDoubleQuote = configGroup.readEntry( "TypographicQuotesBegin" );
    QString endDoubleQuote = configGroup.readEntry( "TypographicQuotesEnd" );

    m_typographicDoubleQuotes.replace = configGroup.readEntry( "TypographicQuotesEnabled", false );

    QString begin = configGroup.readEntry( "TypographicSimpleQuotesBegin" );
    QString end = configGroup.readEntry( "TypographicSimpleQuotesEnd" );
    m_typographicSimpleQuotes.replace = configGroup.readEntry( "TypographicSimpleQuotesEnabled", false );

    m_bAutoSuperScript = configGroup.readEntry( "AutoSuperScript", true );

    config->setGroup( "completion" );
    m_completion = configGroup.readEntry( "completion", false );

    m_completionAppendSpace = configGroup.readEntry( "CompletionAppendSpace", false );
    m_minCompletionWordLength = configGroup.readUnsignedNumEntry( "CompletionMinWordLength", 5 );
    m_nbMaxCompletionWord = configGroup.readUnsignedNumEntry( "NbMaxCompletionWord", 100 );
    m_addCompletionWord = configGroup.readEntry( "AddCompletionWord", true );
    m_toolTipCompletion = configGroup.readEntry( "ToolTipCompletion", true );
    m_keyCompletionAction = ( KoAutoFormat::KeyCompletionAction )configGroup.readUnsignedNumEntry( "CompletionKeyAction", 0 );

    if ( force )
    {
        m_entries.setAutoDelete(true);
        m_entries.clear();
        m_entries.setAutoDelete(false);
        m_allLanguages.setAutoDelete(true);
        m_allLanguages.clear();
        m_allLanguages.setAutoDelete(false);
        m_upperCaseExceptions.clear();
        m_superScriptEntries.clear();
        m_twoUpperLetterException.clear();

    }

    //config->setGroup( "AutoFormatEntries" );

    readAutoCorrectConfig();

    if( beginDoubleQuote.isEmpty())
    {
        if( m_typographicDefaultDoubleQuotes.begin.isNull())
            m_typographicDoubleQuotes.begin = QChar('«');
        else
            m_typographicDoubleQuotes.begin = m_typographicDefaultDoubleQuotes.begin;
    }
    else
        m_typographicDoubleQuotes.begin = beginDoubleQuote[0];

    if( endDoubleQuote.isEmpty() )
    {
        if( m_typographicDefaultDoubleQuotes.end.isNull())
            m_typographicDoubleQuotes.end = QChar('»');
        else
            m_typographicDoubleQuotes.end = m_typographicDefaultDoubleQuotes.end;
    }
    else
        m_typographicDoubleQuotes.end = endDoubleQuote[0];

    m_typographicDoubleQuotes.replace = m_typographicDoubleQuotes.replace
                                        && !m_typographicDoubleQuotes.begin.isNull()
                                        && !m_typographicDoubleQuotes.end.isNull();


    if( begin.isEmpty())
    {
        if( m_typographicDefaultSimpleQuotes.begin.isNull())
            m_typographicSimpleQuotes.begin = QChar('\'');
        else
            m_typographicSimpleQuotes.begin = m_typographicDefaultSimpleQuotes.begin;
    }
    else
        m_typographicSimpleQuotes.begin = begin[0];

    if( end.isEmpty() )
    {
        if( m_typographicDefaultSimpleQuotes.end.isNull())
            m_typographicSimpleQuotes.end = QChar('\'');
        else
            m_typographicSimpleQuotes.end = m_typographicDefaultSimpleQuotes.end;
    }
    else
        m_typographicSimpleQuotes.end = end[0];

    m_typographicSimpleQuotes.replace = m_typographicSimpleQuotes.replace
                                        && !m_typographicSimpleQuotes.end.isNull()
                                        && !m_typographicSimpleQuotes.begin.isNull();


    loadAllLanguagesAutoCorrection();
    buildMaxLen();
    autoFormatIsActive();
    m_configRead = true;
}

void KoAutoFormat::readAutoCorrectConfig()
{
    Q_ASSERT( m_entries.isEmpty() ); // readConfig is only called once...
    KLocale klocale(m_doc->instance()->instanceName());
    QString kdelang = klocale.languageList().front();
    kdelang.remove( QRegExp( "@.*" ) );
    kDebug(32500) << "KoAutoFormat: m_autoFormatLanguage=" << m_autoFormatLanguage << " kdelang=" << kdelang << endl;
    QString fname;
    if ( !m_autoFormatLanguage.isEmpty() )
    {
        fname = locate( "data", "koffice/autocorrect/" + m_autoFormatLanguage + ".xml", m_doc->instance() );
    }
    if ( m_autoFormatLanguage != "all_languages" )
    {
        if ( fname.isEmpty() && !kdelang.isEmpty() )
            fname = locate( "data", "koffice/autocorrect/" + kdelang + ".xml", m_doc->instance() );
        if ( fname.isEmpty() && kdelang.contains("_") )
        {
            kdelang.remove( QRegExp( "_.*" ) );
            fname = locate( "data", "koffice/autocorrect/" + kdelang + ".xml", m_doc->instance() );
        }
        if ( fname.isEmpty() )
            fname = locate( "data", "koffice/autocorrect/autocorrect.xml", m_doc->instance() );
    }
    if ( fname.isEmpty() )
        return;
    QFile xmlFile(fname);
    if(!xmlFile.open(QIODevice::ReadOnly))
        return;

    QDomDocument doc;
    if(!doc.setContent(&xmlFile))
        return;

    if(doc.doctype().name() != "autocorrection") {
        //return;
    }
    QDomElement de=doc.documentElement();

    loadAutoCorrection( de );

    QDomElement upper = de.namedItem( "UpperCaseExceptions" ).toElement();
    if(!upper.isNull())
    {
        QDomNodeList nl = upper.childNodes();
        for(int i = 0; i < nl.count(); i++)
        {
            m_upperCaseExceptions+= nl.item(i).toElement().attribute("exception");
        }
    }

    QDomElement twoUpper = de.namedItem( "TwoUpperLetterExceptions" ).toElement();
    if(!twoUpper.isNull())
    {
        QDomNodeList nl = twoUpper.childNodes();
        for(int i = 0; i < nl.count(); i++)
        {
            m_twoUpperLetterException+= nl.item(i).toElement().attribute("exception");
        }
    }

    QDomElement superScript = de.namedItem( "SuperScript" ).toElement();
    if(!superScript.isNull())
    {
        QDomNodeList nl = superScript.childNodes();
        for(int i = 0; i < nl.count() ; i++) {
            //bug in qmap we overwrite = false doesn't work
            //so we can't add multiple "othernb"
            m_superScriptEntries.insert( nl.item(i).toElement().attribute("find"), KoAutoFormatEntry(nl.item(i).toElement().attribute("super")),FALSE );
        }
    }

    QDomElement doubleQuote = de.namedItem( "DoubleQuote" ).toElement();
    if(!doubleQuote.isNull())
    {
        QDomElement childItem = doubleQuote.namedItem("doublequote").toElement();
        if ( !childItem.isNull() )
        {
            QString attr = childItem.attribute( "begin" );
            if ( !attr.isEmpty() && attr[0] != 0 )
                m_typographicDefaultDoubleQuotes.begin = attr[0];
            attr = childItem.attribute( "end" );
            if ( !attr.isEmpty() && attr[0] != 0 )
                m_typographicDefaultDoubleQuotes.end = attr[0];
        }
    }
    QDomElement simpleQuote = de.namedItem( "SimpleQuote" ).toElement();
    if(!simpleQuote.isNull())
    {
        QDomElement childItem = simpleQuote.namedItem("simplequote").toElement();
        if ( !childItem.isNull() )
        {
            QString attr = childItem.attribute( "begin" );
            if ( !attr.isEmpty() && attr[0] != 0 )
                m_typographicDefaultSimpleQuotes.begin = attr[0];
            attr = childItem.attribute( "end" );
            if ( !attr.isEmpty() && attr[0] != 0 )
                m_typographicDefaultSimpleQuotes.end = attr[0];
        }
    }
}

void KoAutoFormat::loadAllLanguagesAutoCorrection()
{
    QString fname = locate( "data", "koffice/autocorrect/all_languages.xml", m_doc->instance() );
    if ( fname.isEmpty() )
        return;
    QFile xmlFile( fname );
    if(xmlFile.open(QIODevice::ReadOnly))
    {
        QDomDocument doc;
        if(!doc.setContent(&xmlFile)) {
            return;
        }
        if(doc.doctype().name() != "autocorrection") {
            //return;
        }
        QDomElement de=doc.documentElement();

        loadAutoCorrection( de, true );
        xmlFile.close();
    }
}

void KoAutoFormat::loadAutoCorrection( const QDomElement & _de, bool _allLanguages )
{
    QDomElement item = _de.namedItem( "items" ).toElement();
    if(!item.isNull())
    {
        QDomNodeList nl = item.childNodes();
        m_maxFindLength=nl.count();
        for(int i = 0; i < m_maxFindLength; i++) {
            loadEntry( nl.item(i).toElement(), _allLanguages);
        }
    }
}

void KoAutoFormat::loadEntry( const QDomElement &nl, bool _allLanguages)
{
    KoAutoFormatEntry *tmp =new KoAutoFormatEntry(nl.attribute("replace"));
    if ( nl.hasAttribute("FONT"))
    {
        tmp->createNewEntryContext();
        tmp->formatEntryContext()->m_family=nl.attribute("FONT");
        tmp->formatEntryContext()->m_optionsMask |= KoSearchContext::Family;
    }
    if ( nl.hasAttribute("SIZE" ))
    {
        tmp->createNewEntryContext();
        tmp->formatEntryContext()->m_size = nl.attribute("SIZE" ).toInt();
        tmp->formatEntryContext()->m_optionsMask |= KoSearchContext::Size;
    }
    if (nl.hasAttribute("BOLD" ))
    {
        tmp->createNewEntryContext();
        tmp->formatEntryContext()->m_optionsMask |= KoSearchContext::Bold;
        QString value = nl.attribute("BOLD");
        if ( value.toInt() == 1 )
            tmp->formatEntryContext()->m_options |= KoSearchContext::Bold;
    }
    if (nl.hasAttribute("ITALIC" ))
    {
        tmp->createNewEntryContext();
        tmp->formatEntryContext()->m_optionsMask |= KoSearchContext::Italic;
        QString value = nl.attribute("ITALIC");
        if ( value.toInt() == 1 )
            tmp->formatEntryContext()->m_options |= KoSearchContext::Italic;
    }
    if (nl.hasAttribute("UNDERLINE" ))
    {
        tmp->createNewEntryContext();
        tmp->formatEntryContext()->m_optionsMask |= KoSearchContext::Underline;
        QString value = nl.attribute("UNDERLINE");
        if ( value =="single" )
            tmp->formatEntryContext()->m_underline = KoTextFormat::U_SIMPLE;
        else if ( value =="double" )
            tmp->formatEntryContext()->m_underline = KoTextFormat::U_DOUBLE;
        else if ( value =="single-bold" )
            tmp->formatEntryContext()->m_underline = KoTextFormat::U_SIMPLE_BOLD;
        else
            tmp->formatEntryContext()->m_underline = KoTextFormat::U_NONE;
    }
    if (nl.hasAttribute("STRIKEOUT" ))
    {
        tmp->createNewEntryContext();
        tmp->formatEntryContext()->m_optionsMask |= KoSearchContext::StrikeOut;
        QString value = nl.attribute("STRIKEOUT");
        if ( value =="single" )
            tmp->formatEntryContext()->m_strikeOut = KoTextFormat::S_SIMPLE;
        else if ( value =="double" )
            tmp->formatEntryContext()->m_strikeOut = KoTextFormat::S_DOUBLE;
        else if ( value =="single-bold" )
            tmp->formatEntryContext()->m_strikeOut = KoTextFormat::S_SIMPLE_BOLD;
        else
            tmp->formatEntryContext()->m_strikeOut = KoTextFormat::S_NONE;
    }
    if (nl.hasAttribute("VERTALIGN" ))
    {
        tmp->createNewEntryContext();
        tmp->formatEntryContext()->m_optionsMask |= KoSearchContext::VertAlign;
        QString value = nl.attribute("VERTALIGN");
        tmp->formatEntryContext()->m_vertAlign=static_cast<KoTextFormat::VerticalAlignment>( value.toInt() );

    }
    if ( nl.hasAttribute("TEXTCOLOR" ))
    {
        tmp->createNewEntryContext();
        tmp->formatEntryContext()->m_optionsMask |= KoSearchContext::Color;
        QColor col( nl.attribute("TEXTCOLOR" ));
        tmp->formatEntryContext()->m_color = col;
    }
    if ( nl.hasAttribute("TEXTBGCOLOR" ))
    {
        tmp->createNewEntryContext();
        tmp->formatEntryContext()->m_optionsMask |= KoSearchContext::BgColor;
        QColor col( nl.attribute("TEXTBGCOLOR" ));
        tmp->formatEntryContext()->m_backGroundColor = col;
    }
    if ( !_allLanguages )
        m_entries.insert( nl.attribute("find"), tmp );
    else
        m_allLanguages.insert( nl.attribute("find"), tmp );

}

void KoAutoFormat::saveConfig()
{
    KConfig* config = KoGlobal::kofficeConfig();
    KLocale klocale(m_doc->instance()->instanceName());

    KConfigGroup configGroup( config, "AutoFormat" );
    configGroup.writeEntry( "ConvertUpperCase", m_convertUpperCase );
    configGroup.writeEntry( "formatLanguage", m_autoFormatLanguage=="all_languages" ? klocale.languageList().front() : m_autoFormatLanguage);

    configGroup.writeEntry( "ConvertUpperUpper", m_convertUpperUpper );
    configGroup.writeEntry( "includeTwoLetterException", m_includeTwoUpperLetterException );
    configGroup.writeEntry( "includeAbbreviation", m_includeAbbreviation );

    configGroup.writeEntry( "TypographicQuotesBegin", QString( m_typographicDoubleQuotes.begin ) );
    configGroup.writeEntry( "TypographicQuotesEnd", QString( m_typographicDoubleQuotes.end ) );
    configGroup.writeEntry( "TypographicQuotesEnabled", m_typographicDoubleQuotes.replace );
    configGroup.writeEntry( "TypographicSimpleQuotesBegin", QString( m_typographicSimpleQuotes.begin ) );
    configGroup.writeEntry( "TypographicSimpleQuotesEnd", QString( m_typographicSimpleQuotes.end ) );
    configGroup.writeEntry( "TypographicSimpleQuotesEnabled", m_typographicSimpleQuotes.replace );

    configGroup.writeEntry( "AdvancedAutocorrect", m_advancedAutoCorrect );
    configGroup.writeEntry( "AutoCorrectionWithFormat", m_bAutoCorrectionWithFormat );
    configGroup.writeEntry( "CapitalizeNameOfDays", m_bCapitalizeNameOfDays );

    configGroup.writeEntry( "AutoDetectUrl",m_autoDetectUrl);

    configGroup.writeEntry( "IgnoreDoubleSpace",m_ignoreDoubleSpace );
    configGroup.writeEntry( "RemoveSpaceBeginEndLine",m_removeSpaceBeginEndLine );

    configGroup.writeEntry( "UseBulletStyle", m_useBulletStyle);
    configGroup.writeEntry( "BulletStyle", QString(m_bulletStyle));

    configGroup.writeEntry( "AutoChangeFormat", m_autoChangeFormat);

    configGroup.writeEntry( "AutoReplaceNumber", m_autoReplaceNumber);

    configGroup.writeEntry( "AutoNumberStyle", m_useAutoNumberStyle );

    configGroup.writeEntry( "AutoSuperScript", m_bAutoSuperScript );

    config->setGroup( "completion" );
    configGroup.writeEntry( "completion", m_completion );
    configGroup.writeEntry( "CompletionAppendSpace", m_completionAppendSpace );
    configGroup.writeEntry( "CompletionMinWordLength", m_minCompletionWordLength);
    configGroup.writeEntry( "NbMaxCompletionWord", m_nbMaxCompletionWord);
    configGroup.writeEntry( "AddCompletionWord", m_addCompletionWord );
    configGroup.writeEntry( "ToolTipCompletion", m_toolTipCompletion );
    configGroup.writeEntry( "CompletionKeyAction", ( int )m_keyCompletionAction );

    config->setGroup( "AutoFormatEntries" );
    Q3DictIterator<KoAutoFormatEntry> it( m_entries );

    //refresh m_maxFindLength
    m_maxFindLength=0;
    QDomDocument doc("autocorrection");

    QDomElement begin = doc.createElement( "Word" );
    doc.appendChild( begin );
    QDomElement items;
    items = doc.createElement("items");
    QDomElement data;
    for ( ; it.current() ; ++it )
    {
	items.appendChild(saveEntry( it, doc));
        //m_maxFindLength=qMax(m_maxFindLength,it.currentKey().length());
    }
    buildMaxLen();
    begin.appendChild(items);

    QDomElement upper;
    upper = doc.createElement("UpperCaseExceptions");
    for ( QStringList::Iterator it = m_upperCaseExceptions.begin(); it != m_upperCaseExceptions.end();++it )
    {
	data = doc.createElement("word");
	data.setAttribute("exception",(*it) );
	upper.appendChild(data);
    }
    begin.appendChild(upper);

    QDomElement twoUpper;
    twoUpper = doc.createElement("TwoUpperLetterExceptions");

    for ( QStringList::Iterator it = m_twoUpperLetterException.begin(); it != m_twoUpperLetterException.end();++it )
    {
	data = doc.createElement("word");
	data.setAttribute("exception",(*it) );
	twoUpper.appendChild(data);
    }
    begin.appendChild(twoUpper);

    QDomElement super;
    super = doc.createElement("SuperScript");
    KoAutoFormatEntryMap::Iterator it2 = m_superScriptEntries.begin();
    for ( ; it2 != m_superScriptEntries.end() ; ++it2 )
    {
	data = doc.createElement("superscript");
	data.setAttribute("find", it2.key());
	data.setAttribute("super", it2.data().replace());
	super.appendChild(data);
    }
    begin.appendChild(super);

    QDomElement doubleQuote;
    doubleQuote = doc.createElement("DoubleQuote");
    data = doc.createElement("doublequote");
    data.setAttribute("begin", QString(m_typographicDefaultDoubleQuotes.begin));
    data.setAttribute("end", QString(m_typographicDefaultDoubleQuotes.end));
    doubleQuote.appendChild(data);
    begin.appendChild(doubleQuote);


    QDomElement simpleQuote;
    simpleQuote = doc.createElement("SimpleQuote");
    data = doc.createElement("simplequote");
    data.setAttribute("begin", QString(m_typographicDefaultSimpleQuotes.begin));
    data.setAttribute("end", QString(m_typographicDefaultSimpleQuotes.end));
    simpleQuote.appendChild(data);
    begin.appendChild(simpleQuote);
    QFile f;
    if ( m_autoFormatLanguage.isEmpty())
        f.setName(locateLocal("data", "koffice/autocorrect/"+klocale.languageList().front() + ".xml",m_doc->instance()));
    else
        f.setName(locateLocal("data", "koffice/autocorrect/"+m_autoFormatLanguage + ".xml",m_doc->instance()));
    if(!f.open(QIODevice::WriteOnly)) {
        kWarning()<<"Error during saving autoformat to " << f.name() << endl;
	return;
    }
    QTextStream ts(&f);
    doc.save(ts, 2);
    f.close();
    autoFormatIsActive();
    config->sync();
}

QDomElement KoAutoFormat::saveEntry( Q3DictIterator<KoAutoFormatEntry> _entry, QDomDocument doc)
{
    QDomElement data;
    data = doc.createElement("item");
    data.setAttribute("find", _entry.currentKey());
    data.setAttribute("replace", _entry.current()->replace());
    if ( _entry.current()->formatEntryContext() )
    {
        KoSearchContext *tmp = _entry.current()->formatEntryContext();
        if ( tmp->m_optionsMask & KoSearchContext::Family )
        {
            data.setAttribute("FONT", tmp->m_family);
        }
        if ( tmp->m_optionsMask &  KoSearchContext::Size )
        {
            data.setAttribute("SIZE", tmp->m_size);
        }
        if ( tmp->m_optionsMask & KoSearchContext::Italic )
        {
            data.setAttribute("ITALIC", static_cast<bool>(tmp->m_options & KoSearchContext::Italic));
        }
        if ( tmp->m_optionsMask & KoSearchContext::Bold )
        {
            data.setAttribute("BOLD", static_cast<bool>(tmp->m_options & KoSearchContext::Bold));
        }
        if ( tmp->m_optionsMask & KoSearchContext::Shadow )
        {
            data.setAttribute("SHADOWTEXT", static_cast<bool>(tmp->m_options & KoSearchContext::Shadow));
        }
        if ( tmp->m_optionsMask & KoSearchContext::WordByWord )
        {
            data.setAttribute("WORDBYWORD", static_cast<bool>(tmp->m_options & KoSearchContext::WordByWord));
        }

        if ( tmp->m_optionsMask & KoSearchContext::Underline )
        {
            switch( tmp->m_underline )
            {
            case KoTextFormat::U_SIMPLE:
                data.setAttribute("UNDERLINE", "single");
                break;
            case KoTextFormat::U_DOUBLE:
                data.setAttribute("UNDERLINE", "double");
                break;
            case KoTextFormat::U_SIMPLE_BOLD:
                data.setAttribute("UNDERLINE", "single-bold");
                break;
            case KoTextFormat::U_WAVE:
                data.setAttribute("UNDERLINE", "wave");
                break;
            case KoTextFormat::U_NONE:
                data.setAttribute("UNDERLINE", "none");
                break;
            }
        }
        if ( tmp->m_optionsMask & KoSearchContext::StrikeOut )
        {
            switch( tmp->m_strikeOut )
            {
            case KoTextFormat::S_SIMPLE:
                data.setAttribute("STRIKEOUT", "single");
                break;
            case KoTextFormat::S_DOUBLE:
                data.setAttribute("STRIKEOUT", "double");
                break;
            case KoTextFormat::S_NONE:
                data.setAttribute("STRIKEOUT", "none");
                break;
            case KoTextFormat::S_SIMPLE_BOLD:
                data.setAttribute("STRIKEOUT", "single-bold");
                break;
            }
        }
        if ( tmp->m_optionsMask & KoSearchContext::Attribute )
        {
            data.setAttribute("FONTATTRIBUTE", KoTextFormat::attributeFontToString( tmp->m_attribute ) );
        }

        if ( tmp->m_optionsMask & KoSearchContext::VertAlign)
        {
            data.setAttribute( "VERTALIGN", static_cast<int>(tmp->m_vertAlign) );
        }
        if ( tmp->m_optionsMask & KoSearchContext::BgColor )
        {
            data.setAttribute( "TEXTCOLOR", tmp->m_color.name());
        }
        if ( tmp->m_optionsMask & KoSearchContext::Color )
        {
            data.setAttribute( "TEXTCOLOR", tmp->m_color.name());
        }
        if ( tmp->m_optionsMask & KoSearchContext::BgColor )
        {
            data.setAttribute( "TEXTBGCOLOR", tmp->m_backGroundColor.name());
        }
        if ( tmp->m_optionsMask & KoSearchContext::Language )
            data.setAttribute( "LANGUAGE", tmp->m_language );
    }
    return data;
}

void KoAutoFormat::addAutoFormatEntry( const QString &key, const QString &replace )
{
    KoAutoFormatEntry *findEntry = m_entries.find( key);
    if ( findEntry )
    {
        if ( findEntry->replace().lower() == replace.lower() )
            return;
    }

    KoAutoFormatEntry *tmp = new KoAutoFormatEntry( replace );
    m_entries.insert( key, tmp );
    saveConfig();
    buildMaxLen();
}

QString KoAutoFormat::getLastWord(KoTextParag *parag, int const index)
{
    QString lastWord;
    KoTextString *s = parag->string();
    for ( int i = index - 1; i >= 0; --i )
    {
        QChar ch = s->at( i ).c;
        if ( ch.isSpace() || ch.isPunct() )
            break;
        lastWord.prepend( ch );
    }
    return lastWord;
}

QString KoAutoFormat::getLastWord(const int max_words, KoTextParag *parag, int const index)
{
    QString lastWord;
    KoTextString const *s = parag->string();
    int words = 0;
    for ( int i = index - 1; i >= 0; --i )
    {
        QChar ch = s->at( i ).c;
        if ( ch.isSpace() || ch.isPunct() )
        {
                ++words;
                if (words >= max_words)
                        break;
        }
        lastWord.prepend( ch );
    }
    return lastWord;
}

QString KoAutoFormat::getWordAfterSpace(KoTextParag *parag, int const index)
{
    QString word;
    KoTextString *s = parag->string();
    for ( int i = index - 1; i >= 0; --i )
    {
        QChar ch = s->at( i ).c;
        if ( ch.isSpace() )
            break;
        word.prepend( ch );
    }
    return word;

}

bool KoAutoFormat::doCompletion( KoTextCursor* textEditCursor, KoTextParag *parag, int const index, KoTextObject *txtObj )
{
    if( m_completion )
    {
        bool part=false;
        QString lastWord, word;
        if (m_completionBox && m_completionBox->isShown() ) //word completion with the tool-tip box
        {
                word = m_completionBox->text();
                lastWord = m_completionBox->lastWord();
        }
        else
        {
                QStringList wordlist, new_wordlist;
                for (int i=1; i <= m_countMaxWords; i++ )
                {
                        lastWord = getLastWord(i, parag, index+1);
                        wordlist += m_listCompletion->substringCompletion( lastWord ); //find all completion words that contains lastWord
                }
                int maxlength = 0;
                for ( QStringList::ConstIterator it = wordlist.begin(); it != wordlist.end(); ++it ) // several completion words were found
                {
                  if ( (*it).startsWith( lastWord, false ) && new_wordlist.find(*it) == new_wordlist.end() ) //the completion words that begin with lastWord
                  {
                    if ( (*it).length() > maxlength )
                      maxlength = (*it).length();
                    new_wordlist.append(*it);
                    //kDebug() << "adding word completion:" << *it << endl;
                  }
                }
                if ( new_wordlist.isEmpty() )
                    return false;
                if ( new_wordlist.count() == 1 ) // only one completion word was found
                  word = new_wordlist.first();
                else
                {
                  //we must extract the common part of the completions
                  for (int i = lastWord.length(); i<maxlength && !part; i++) //iterate through all completion words
                  {
                    QChar ch = new_wordlist.first().at(i);
                    for (QStringList::ConstIterator it = new_wordlist.begin(); it != new_wordlist.end(); ++it )
                    {
                      if ( (*it).at(i).lower() != ch.lower() )
                      {
                        word = (*it).left(i); //the completion word is truncated here
                        //kDebug() << "set the word completion to:" << word << endl;
                        part=true; // completion of a part of a word; a space-character after the completion should not be inserted
                        break;
                      }
                    }
                  }
                }
                if (word == lastWord)
                        return false;

                word=lastWord+word.right(word.length()-lastWord.length() );
        }
        if( !word.isEmpty() )
        {
            int const lastword_length = lastWord.length();
            int const start = index+1 - lastword_length;
            int const length = word.length();

            KMacroCommand *macro = new KMacroCommand( i18n("Completion Word"));
            KoTextCursor cursor( parag->document() );
            cursor.setParag( parag );
            cursor.setIndex( start );
            KoTextDocument * textdoc = parag->textDocument();
            if( m_completionAppendSpace && !part)
                word+=" ";
            textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
            cursor.setIndex( start + lastword_length );
            textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );

            macro->addCommand( txtObj->replaceSelectionCommand( textEditCursor, word,
                                                                i18n("Completion Word"),
                                                                KoTextDocument::HighlightSelection ));

            if ( m_completionAppendSpace && !m_ignoreUpperCase && (m_convertUpperUpper || m_convertUpperCase) && !part)
            {
                //find the first word
                for (int i=1; i < word.length(); i++)
                        if ( word.at(i).isSpace() || word.at(i).isPunct() )
                        {
                               word.truncate(i);
                               break;
                        }
                int const newPos = start + word.length();// + index - 3;
                KCommand *cmd = doUpperCase( textEditCursor, parag, newPos, word, txtObj );
                if( cmd )
                    macro->addCommand( cmd );
                txtObj->emitHideCursor();
                textEditCursor->setIndex(start+ length+1);
            }
            else
            {
                txtObj->emitHideCursor();
                textEditCursor->setIndex(start+ length);
            }
            txtObj->emitNewCommand( macro );

            // The space/tab/CR that we inserted is still there but delete/insert moved the cursor
            // -> go right

            txtObj->emitShowCursor();
            removeToolTipCompletion();
            return true;
        }
    }
    return false;
}

bool KoAutoFormat::doToolTipCompletion( KoTextCursor* textEditCursor, KoTextParag *parag, int index, KoTextObject *txtObj, int keyPressed )
{
    if( m_completion && m_toolTipCompletion && m_completionBox && m_completionBox->isShown() )
    {
        if ( ( keyPressed == Qt::Key_Return && m_keyCompletionAction==Enter )
             || ( keyPressed == Qt::Key_Enter && m_keyCompletionAction==Enter )
             || ( keyPressed == Qt::Key_Tab && m_keyCompletionAction==Tab )
             || ( keyPressed == Qt::Key_Space && m_keyCompletionAction==Space )
             || ( keyPressed == Qt::Key_End && m_keyCompletionAction==End )
             || ( keyPressed == Qt::Key_Right && m_keyCompletionAction==Qt::DockRight ))
        {
            return doCompletion(textEditCursor, parag, index, txtObj);
        }
    }
    return false;
}
void KoAutoFormat::showToolTipBox(KoTextParag *parag,  int index, QWidget *widget, const QPoint &pos )
{

    if( m_completion && m_toolTipCompletion)
    {
        QString lastWord, word;
        for (int i=1; i <= m_countMaxWords; i++ )
        {
                lastWord = getLastWord(i, parag, index+1);
                word=m_listCompletion->makeCompletion( lastWord );
                if ( !word.isEmpty())
                        break;
        }
        if( !word.isEmpty() && word!=lastWord )
        {
            int const length = lastWord.length();
            if (length<=3)
                return;
            word=lastWord+word.right(word.length()-length);
            if (!m_completionBox)
                m_completionBox = new KoCompletionBox(0,0,Qt::WType_Popup);
            QPoint const show_pos = widget->mapToGlobal(pos);
            m_completionBox->setText(word);
            m_completionBox->setLastWord(lastWord);
            m_completionBox->adjustSize();
            int const height = m_completionBox->sizeHint().height();
            m_completionBox->move( show_pos.x(), show_pos.y() - height );

            if (!m_completionBox->isShown() )
            {
                m_completionBox->show();
                widget->setFocus();
            }
        }
        else
                removeToolTipCompletion();
    }
}
void KoAutoFormat::removeToolTipCompletion()
{
    if (m_completion && m_toolTipCompletion && m_completionBox && m_completionBox->isShown())
        m_completionBox->hide();
}

void KoAutoFormat::autoFormatIsActive()
{
    m_bAutoFormatActive = m_useBulletStyle ||
                          m_removeSpaceBeginEndLine ||
                          m_autoDetectUrl ||
                          m_convertUpperUpper ||
                          m_convertUpperCase ||
                          m_autoReplaceNumber ||
                          m_autoChangeFormat ||
                          m_completion ||
                          m_typographicDoubleQuotes.replace ||
                          m_typographicSimpleQuotes.replace ||
                          m_entries.count()!=0 ||
                          m_allLanguages.count()!=0;
}

void KoAutoFormat::doAutoFormat( KoTextCursor* textEditCursor, KoTextParag *parag, int index, QChar ch,KoTextObject *txtObj )
{
    m_ignoreUpperCase = false;

    if ( !m_configRead )
        readConfig();

    if ( !m_bAutoFormatActive )
        return;

    if( ch.isSpace())
    {
        //a link doesn't have a space
        //=>m_ignoreUpperCase = false
        //m_ignoreUpperCase=false;

        QString word=getWordAfterSpace(parag,index);

        if ( m_autoChangeFormat && index > 3)
        {
            KCommand *cmd =doAutoChangeFormat( textEditCursor, parag, index, word, txtObj );
            if ( cmd )
                txtObj->emitNewCommand( cmd );

        }
        if ( m_autoReplaceNumber )
        {
            KCommand *cmd = doAutoReplaceNumber( textEditCursor, parag, index, word, txtObj );
            if ( cmd )
                txtObj->emitNewCommand( cmd );
        }
    }

    if( ch =='\n' )
    {

        if( m_removeSpaceBeginEndLine && index > 1)
        {
            KCommand *cmd = doRemoveSpaceBeginEndLine( textEditCursor, parag, txtObj, index );
            if ( cmd )
                txtObj->emitNewCommand( cmd );
        }
        if( m_useBulletStyle  && index > 3)
        {
            KCommand *cmd =doUseBulletStyle( textEditCursor, parag, txtObj, index );
            if ( cmd )
                txtObj->emitNewCommand( cmd );
        }
        if( m_useAutoNumberStyle && index > 3 )
        {
            KCommand *cmd =doUseNumberStyle( textEditCursor, parag, txtObj, index );
            if ( cmd )
                txtObj->emitNewCommand( cmd );
        }
        if( m_convertUpperUpper && m_includeTwoUpperLetterException )
            doAutoIncludeUpperUpper(textEditCursor, parag, txtObj );
        if( m_convertUpperCase && m_includeAbbreviation )
            doAutoIncludeAbbreviation(textEditCursor, parag, txtObj );
    }

    //kDebug(32500) << "KoAutoFormat::doAutoFormat ch=" << QString(ch) << endl;
    //if ( !m_enabled )
    //    return;
    // Auto-correction happens when pressing space, tab, CR, punct etc.
    if ( (ch.isSpace() || ch==':' || ch=='?' || ch=='!' || ch==',' || (m_advancedAutoCorrect && ch=='.') ) && index > 0 )
    {
        KCommand *cmd = 0L;
        KMacroCommand *macro = 0L;
        QString lastWord = getWordAfterSpace(parag, index);
        //kDebug(32500) << "KoAutoFormat::doAutoFormat lastWord=" << lastWord << endl;

        if ( ch == '.')
                detectStartOfLink( parag, index, true );
        else
                detectStartOfLink( parag, index, false );

        if ( !m_wordInserted && m_advancedAutoCorrect && !m_ignoreUpperCase)
        {
                int const completionBeginPos = index -lastWord.length();
                int newPos = index;
                cmd = doAutoCorrect( textEditCursor, parag, newPos, txtObj );

                if( cmd )
                {
                if (!macro)
                        macro = new KMacroCommand(i18n("Autocorrection"));
                macro->addCommand( cmd );
                }

                int const endPos=textEditCursor->index();
                bool was_a_replacement;
                if (index == newPos)
                        was_a_replacement = false;
                else
                        was_a_replacement = true;

                if( was_a_replacement) // a replacement took place
                {
                        txtObj->emitHideCursor();
                        if(endPos==0) //new line, the user pressed enter
                        {
                                textEditCursor->gotoUp();
                                textEditCursor->gotoLineEnd();
                                newPos=textEditCursor->index();
                        }
                        else
                                newPos= endPos-1;

                        m_wordInserted = true; //don't allow other replacements in this replacement
                        for(int i=completionBeginPos; i<newPos;i++)
                        {
                                textEditCursor->setIndex(i);
                                doAutoFormat( textEditCursor, parag, i, parag->toString().at(i),txtObj );

                        }
                        textEditCursor->setIndex(newPos);
                        doAutoFormat( textEditCursor, parag, newPos, ch,txtObj );
                        m_wordInserted = false;
                        if (endPos==0)
                        {
                                textEditCursor->gotoLineStart();
                                textEditCursor->gotoDown();
                        }
                        else
                                textEditCursor->setIndex(newPos+1);
                        txtObj->emitShowCursor();
                        return;
                }

        }

        if (!m_ignoreUpperCase && m_bCapitalizeNameOfDays)
        {
            KCommand *cmd = doCapitalizeNameOfDays( textEditCursor, parag, index, lastWord, txtObj  );

            if( cmd )
            {
                if (!macro)
                macro = new KMacroCommand(i18n("Autocorrection"));
                macro->addCommand( cmd );
                m_ignoreUpperCase = true;
            }
        }

        if (ch=='.')
                return;

        //kDebug(32500)<<" m_listCompletion->items() :"<<m_listCompletion->items()<<endl;
        if( !m_ignoreUpperCase && m_completion && m_addCompletionWord && m_listCompletion->items().count() < m_nbMaxCompletionWord )
        {
                QString completionWord("");
                QChar ch;
                for (int i=0;i<lastWord.length();i++)
                {
                        ch = lastWord.at(i);
                        if (ch.isPunct() && ch!='-' && ch!='=' )
                        {
                                if (completionWord.at(0) == '-')
                                        completionWord.remove(0,1);

                                if (completionWord.length()>= m_minCompletionWordLength  && !completionWord.isEmpty() && m_listCompletion->makeCompletion(completionWord).isEmpty())
                                {
                                        kDebug() << "Adding:" << completionWord << endl;
                                        m_listCompletion->addItem( completionWord );
                                        if ( completionWord.length() > m_countMaxWords )
                                            m_countMaxWords = completionWord.length();

                                }
                                completionWord = "";
                        }
                        else
                        {
                                completionWord.append(ch);
                                if (i==lastWord.length()-1)
                                {
                                        if (completionWord.at(0) == '-')
                                                completionWord.remove(0,1);
                                        if (completionWord.at(completionWord.length()-1) == '-')
                                                completionWord.truncate(completionWord.length()-1);
                                        completionWord.remove('=');
                                        if (completionWord.length()>= m_minCompletionWordLength && !completionWord.isEmpty() && m_listCompletion->makeCompletion(completionWord).isEmpty())
                                        {
                                                kDebug() << "Adding:" << completionWord << endl;
                                                m_listCompletion->addItem( completionWord );
                                                if ( completionWord.length() > m_countMaxWords )
                                                    m_countMaxWords = completionWord.length();
                                        }
                                }
                        }
                }
        }

        if( m_autoDetectUrl && m_ignoreUpperCase && (ch!='?' || lastWord.at(lastWord.length()-1)=='?') )
        {
                doAutoDetectUrl( textEditCursor, parag, index, lastWord, txtObj );
                //textEditCursor->gotoRight();
        }

        if (!m_ignoreUpperCase && (m_convertUpperUpper || m_convertUpperCase) )
        {
            cmd = doUpperCase( textEditCursor, parag, index, lastWord, txtObj );

            if( cmd )
            {
                if (!macro)
                    macro = new KMacroCommand(i18n("Autocorrection"));
                macro->addCommand( cmd );
            }
        }

        if ( macro )
            txtObj->emitNewCommand( macro );

        if(!m_ignoreUpperCase &&  m_bAutoSuperScript && m_superScriptEntries.count()>0)
        {
            if( lastWord.at(0).isPunct() )
                lastWord.remove(0,1);
            KCommand * cmd = doAutoSuperScript( textEditCursor, parag, index, lastWord, txtObj  );
            if ( cmd )
                txtObj->emitNewCommand( cmd );
        }

    }
    else
    {
        if ( ch == '"' && m_typographicDoubleQuotes.replace )
        {
                KCommand *cmd = doTypographicQuotes( textEditCursor, parag, index, txtObj, true /*double quote*/ );
                if ( cmd )
                txtObj->emitNewCommand( cmd );
        }
        else if ( ch == '\'' && m_typographicDoubleQuotes.replace )
        {
                KCommand *cmd = doTypographicQuotes( textEditCursor, parag, index, txtObj, false /* simple quote*/ );
                if ( cmd )
                txtObj->emitNewCommand( cmd );
        }
    }
}

KCommand *KoAutoFormat::doAutoCorrect( KoTextCursor* textEditCursor, KoTextParag *parag, int &index, KoTextObject *txtObj )
{
    //if(!m_advancedAutoCorrect)
      //  return 0L;
    // Prepare an array with words of different lengths, all terminating at "index".
    // Obviously only full words are put into the array
    // But this allows 'find strings' with spaces and punctuation in them.
    QString * wordArray = new QString[m_maxFindLength+1];
    {
        QString word;
        KoTextString *s = parag->string();
        for ( int i = index - 1; i >= 0; --i )
        {
            QChar ch = s->at( i ).c;
	    // It's necessary to stop at spaces - #99063
            if ( ch.isSpace() /*|| ch.isPunct()*/ || i==0)
            {
                if(i==0 && word.length()<m_maxFindLength)
                    word.prepend( ch );
                wordArray[word.length()]=word;
            }
            word.prepend( ch );
            if (((index - 1)-i) == (int)m_maxFindLength)
                break;
        }

    }
    KCommand *cmd = autoFormatWord( textEditCursor, parag, index, txtObj, wordArray, false );
    if ( !cmd )
        cmd = autoFormatWord( textEditCursor, parag, index, txtObj, wordArray, true );
    delete [] wordArray;
    return cmd;
}


KCommand *KoAutoFormat::autoFormatWord( KoTextCursor* textEditCursor, KoTextParag *parag, int &index, KoTextObject *txtObj, QString * _wordArray, bool _allLanguages )
{
    KoTextDocument * textdoc = parag->textDocument();

    // Now for each entry in the autocorrect list, look if
    // the word of the same size in wordArray matches.
    // This allows an o(n) behaviour instead of an o(n^2).
    for(int i=m_maxFindLength;i>0;--i)
    {
        if ( !_wordArray[i].isEmpty())
        {
            KoAutoFormatEntry* it = 0L;
            if ( _allLanguages )
                it = m_allLanguages[ _wordArray[i] ];
            else
                it = m_entries[ _wordArray[i] ];
            if ( _wordArray[i]!=0  && it )
            {
                unsigned int length = _wordArray[i].length();
                int const start = index - length;
                KoTextCursor cursor( parag->document() );
                cursor.setParag( parag );
                cursor.setIndex( start );
                textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
                cursor.setIndex( start + length );
                textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );
                KCommand *cmd = 0L;
                kDebug()<<"it->replace() :"<<it->replace()<<endl;
                if (!it->formatEntryContext() || !m_bAutoCorrectionWithFormat)
                {
                    cmd = txtObj->replaceSelectionCommand( textEditCursor, it->replace(),
                                                           i18n("Autocorrect Word"),
                                                           KoTextDocument::HighlightSelection );
                }
                else
                {
                    int flags = 0;
                    KoTextFormat * lastFormat = parag->at( start )->format();
                    KoTextFormat * newFormat = new KoTextFormat(*lastFormat);
                    changeTextFormat(it->formatEntryContext(), newFormat, flags );
                    KMacroCommand *macro = new KMacroCommand( i18n("Autocorrect Word with Format"));
                    KCommand *cmd2=txtObj->replaceSelectionCommand( textEditCursor, it->replace(),
                                                                    i18n("Autocorrect Word"),
                                                                    KoTextDocument::HighlightSelection );
                    if ( cmd2 )
                        macro->addCommand(cmd2);
                    KoTextCursor cursor( parag->document() );
                    cursor.setParag( parag );
                    cursor.setIndex( start );
                    textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
                    cursor.setIndex( start + it->replace().length()/*+ length + 1*/ );
                    textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );

                    cmd2 =txtObj->setFormatCommand( textEditCursor, &lastFormat, newFormat, flags, false, KoTextDocument::HighlightSelection );
                    macro->addCommand( cmd2);

                    index = index - length + it->replace().length();
                    textEditCursor->setIndex(index+1);
                    cmd2 =txtObj->setFormatCommand( textEditCursor, &newFormat, lastFormat, 0 );
                    macro->addCommand( cmd2);
                    parag->at( index+1 )->setFormat(lastFormat);

                    cmd = macro;
                    txtObj->emitHideCursor();
                    textEditCursor->gotoRight();
                    txtObj->emitShowCursor();

                    return cmd;
                }
                // The space/tab/CR that we inserted is still there but delete/insert moved the cursor
                // -> go right

                txtObj->emitHideCursor();
                textEditCursor->gotoRight();
                txtObj->emitShowCursor();
                index = index - length + it->replace().length();
                return cmd;
            }
        }
    }
    return 0L;
}

KCommand *KoAutoFormat::doTypographicQuotes( KoTextCursor* textEditCursor, KoTextParag *parag, int index, KoTextObject *txtObj, bool doubleQuotes )
{
    //kDebug(32500) << "KoAutoFormat::doTypographicQuotes" << endl;
    KoTextDocument * textdoc = parag->textDocument();
    KoTextCursor cursor( parag->document() );
    cursor.setParag( parag );
    cursor.setIndex( index );
    textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
    cursor.setIndex( index + 1 );
    textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );

    // Need to determine if we want a starting or ending quote.
    // I see two solutions: either simply alternate, or depend on leading space.
    // MSWord does the latter afaics...
    QString replacement;
    if ( index > 0 && !parag->at( index - 1 )->c.isSpace() )
    {
        if( doubleQuotes )
            replacement = m_typographicDoubleQuotes.end;
        else
            replacement = m_typographicSimpleQuotes.end;
    }
    else
    {
        if( doubleQuotes )
            replacement = m_typographicDoubleQuotes.begin;
        else
            replacement = m_typographicSimpleQuotes.begin;
    }
    return txtObj->replaceSelectionCommand( textEditCursor, replacement,
                                            i18n("Typographic Quote"),
                                            KoTextDocument::HighlightSelection );
}

KCommand * KoAutoFormat::doUpperCase( KoTextCursor *textEditCursor, KoTextParag *parag,
                                int index, const QString & word, KoTextObject *txtObj )
{
    KoTextDocument * textdoc = parag->textDocument();
    unsigned int length = word.length();
    if (word.at(length-1) == '.' )
    {
        --index;
        --length;
    }
    int const start = index - length;
    KoTextCursor backCursor( parag->document() );
    backCursor.setParag( parag );
    backCursor.setIndex( start );

    // backCursor now points at the first char of the word
    QChar const firstChar = backCursor.parag()->at( backCursor.index() )->c;

    bool bNeedMove = false;
    KCommand *cmd = 0L;
    if ( m_convertUpperCase && isLower( firstChar ) )
    {
        bool beginningOfSentence = true; // true if beginning of text
        // Go back over any space/tab/CR
        while ( backCursor.index() > 0 || backCursor.parag()->prev() )
        {
            beginningOfSentence = false; // we could go back -> false unless we'll find '.'
            backCursor.gotoLeft();
            if ( !backCursor.parag()->at( backCursor.index() )->c.isSpace() )
                break;
        }
        // We are now at the first non-space char before the word
        if ( !beginningOfSentence )
                beginningOfSentence = isMark( backCursor.parag()->at( backCursor.index() )->c);
            //beginningOfSentence = isMark( backCursor.parag()->at( backCursor.index() )->c ) && (backCursor.parag()->at( backCursor.index()+1 )->c.isSpace());
        if ( !beginningOfSentence && start==0 )
            if ( parag->counter() || backCursor.parag()->at( backCursor.index() )->c.isPunct() )
                beginningOfSentence = true;

        // Now look for exceptions
        if ( beginningOfSentence )
        {
            QChar const punct = backCursor.parag()->at( backCursor.index() )->c;
            QString const text = getLastWord( backCursor.parag(), backCursor.index() )
                           + punct;
                           kDebug() << "text: " << text << endl;
            // text has the word at the end of the 'sentence', including the termination. Example: "Mr."
            beginningOfSentence = (m_upperCaseExceptions.findIndex(text)==-1); // Ok if we can't find it
        }

        if ( beginningOfSentence )
        {
            KoTextCursor cursor( parag->document() );
            cursor.setParag( parag );
            cursor.setIndex( start );
            textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
            cursor.setIndex( start + 1 );
            textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );
            cmd = txtObj->replaceSelectionCommand( textEditCursor, QString( firstChar.upper() ),
                                                   i18n("Autocorrect (capitalize first letter)"),
                                                   KoTextDocument::HighlightSelection );
            bNeedMove = true;
        }
    }
    else if ( m_convertUpperUpper && isUpper( firstChar ) && length > 2 )
    {
        backCursor.setIndex( backCursor.index() + 1 );
        QChar secondChar = backCursor.parag()->at( backCursor.index() )->c;
        //kDebug(32500)<<" secondChar :"<<secondChar<<endl;
        if ( isUpper( secondChar ) )
        {
            // Check next letter - we still want to be able to write fully uppercase words...
            backCursor.setIndex( backCursor.index() + 1 );
            QChar thirdChar = backCursor.parag()->at( backCursor.index() )->c;
            if ( isLower( thirdChar ) && (m_twoUpperLetterException.findIndex(word)==-1))
            {
                // Ok, convert
                KoTextCursor cursor( parag->document() );
                cursor.setParag( parag );
                cursor.setIndex( start + 1 ); // After all the first letter's fine, so only change the second letter
                textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
                cursor.setIndex( start + 2 );
                textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );

                QString replacement = word[1].lower();
                cmd = txtObj->replaceSelectionCommand( textEditCursor, replacement,
                                                       i18n("Autocorrect"),
                                                       KoTextDocument::HighlightSelection );

                bNeedMove = true;
            }
        }
    }
    if ( bNeedMove )
    {
        if (word.at(word.length()-1) == '.' )
                ++index;
        txtObj->emitHideCursor();
        textEditCursor->setParag( parag );
        textEditCursor->setIndex( index );
        textEditCursor->gotoRight(); // not the same thing as index+1, in case of CR
        txtObj->emitShowCursor();
    }
    return cmd;
}

KCommand * KoAutoFormat::doAutoReplaceNumber( KoTextCursor* textEditCursor, KoTextParag *parag, int& index, const QString & word , KoTextObject *txtObj )
{
    unsigned int length = word.length();
    if ( length != 3 )
        return 0L;
    KoTextDocument * textdoc = parag->textDocument();
    int start = index - length;
    if( word == QString("1/2") || word == QString("1/4") || word == QString("3/4") )
    {
        KoTextCursor cursor( parag->document() );
        cursor.setParag( parag );
        cursor.setIndex( start );
        textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
        cursor.setIndex( start + length );
        textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );
        QString replacement;
        if( word == QString("1/2") )
            replacement=QString("½");
        else if (word == QString("1/4") )
            replacement=QString("¼");
        else if (word == QString("3/4") )
            replacement=QString("¾");
        QString cmdName = i18n("Autocorrect for Fraction");
        KCommand *cmd =txtObj->replaceSelectionCommand( textEditCursor, replacement,
                                                        cmdName,
                                                        KoTextDocument::HighlightSelection );
        txtObj->emitHideCursor();
        textEditCursor->gotoRight();
        txtObj->emitShowCursor();
        index = index - length + replacement.length();
        return cmd;
    }
    return 0L;
}

void KoAutoFormat::detectStartOfLink(KoTextParag * parag, int const index, bool const insertedDot)
{
    QString word;
    KoTextString *s = parag->string();
    for ( int i = 0; i < index; ++i )
    {
        word.append( s->at( i ).c );
    }

    if (word.find("http")!=-1 || word.find("https")!=-1 || word.find("mailto")!=-1 || word.find("ftp")!=-1 || word.find("file")!=-1
        || word.find("news")!=-1 || word.find('@')!=-1)
                m_ignoreUpperCase=true;
    else
    {
        int const tmp_pos=word.find("www.");
        if (tmp_pos!=-1 && (word.find('.',tmp_pos+4)!=-1 || insertedDot) )
               m_ignoreUpperCase=true;
    }
}

void KoAutoFormat::doAutoDetectUrl( KoTextCursor *textEditCursor, KoTextParag *parag, int &index, QString & word, KoTextObject *txtObj )
{
    kDebug() << "link:" << word << endl;
    char link_type = 0;
    int pos = word.find("http://");
    int tmp_pos = word.find("https://");
    if(tmp_pos<pos && tmp_pos!=-1)
          pos = tmp_pos;
    tmp_pos = word.find("mailto:/");
    if((tmp_pos<pos || pos==-1 ) && tmp_pos!=-1)
          pos = tmp_pos;
    tmp_pos = word.find("ftp://");
    if((tmp_pos<pos || pos==-1 ) && tmp_pos!=-1)
          pos = tmp_pos;
    tmp_pos = word.find("ftp.");
    if((tmp_pos<pos || pos==-1 ) && tmp_pos!=-1)
    {
          pos = tmp_pos;
          link_type = 3;
    }
    tmp_pos = word.find("file:/");
    if((tmp_pos<pos || pos==-1 ) && tmp_pos!=-1)
          pos = tmp_pos;
    tmp_pos = word.find("news:");
    if((tmp_pos<pos || pos==-1 ) && tmp_pos!=-1)
          pos = tmp_pos;
    tmp_pos = word.find("www.");
    if((tmp_pos<pos || pos==-1 ) && tmp_pos!=-1 && word.find('.',tmp_pos+4)!=-1 )
    {
          pos = tmp_pos;
          link_type = 2;
    }
    tmp_pos = word.find('@');
    if ( pos == -1 && tmp_pos != -1 )
    {
          pos = tmp_pos-1;
          QChar c;
          while( pos>=0 )
          {
                c = word.at(pos);
                if ( c.isPunct() && c!='.'&& c!='_')    break;
                else    --pos;
          }
          if ( pos == tmp_pos-1 ) //it not a valid address
          {
                m_ignoreUpperCase = false;
                pos = -1;
          }
          else
                ++pos;
          link_type = 1;
    }
    if(pos!=-1)
    {
        // A URL inside e.g. quotes (like "http://www.koffice.org" with the quotes) shouldn't include the quote in the URL.
	while ( !word.at(word.length()-1).isLetter() &&  !word.at(word.length()-1).isDigit() && word.at(word.length()-1)!='/')
        {
                word.truncate(word.length()-1);
                --index;
        }
        word.remove(0,pos);
        unsigned int const length = word.length();
        int const start = index - length;
        KoTextCursor cursor( parag->document() );
        KoTextDocument * textdoc = parag->textDocument();
        cursor.setParag( parag );
        cursor.setIndex( start );
        textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
        cursor.setIndex( start + length );
        textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );
        QString newWord = word;
        if(link_type==1)
            newWord = QString("mailto:") + word;
        else if(link_type==2)
            newWord = QString("http://") + word;
        else if(link_type==3)
            newWord = QString("ftp://") + word;

        KoVariable* var = new KoLinkVariable( textdoc, word, newWord, m_varFormatCollection->format( "STRING" ), m_varCollection );
        CustomItemsMap customItemsMap;
        customItemsMap.insert( 0, var );
        KoTextFormat * lastFormat = parag->at( start )->format();
        int origCursorIndex = textEditCursor->index();
        txtObj->insert( textEditCursor, lastFormat, KoTextObject::customItemChar(), i18n("Insert Variable"),
                        KoTextDocument::HighlightSelection, KoTextObject::DefaultInsertFlags, customItemsMap );
        var->recalc();
        parag->invalidate(0);
        parag->setChanged( true );

        // adjust index
        index -= length-1; // we removed length chars and inserted one instead

        txtObj->emitHideCursor();
        textEditCursor->setIndex( origCursorIndex - (length-1) );
        txtObj->emitShowCursor();

        // ###### TODO: Move to a common method, this code is duplicated...
        if ( m_completion && m_addCompletionWord && m_listCompletion->items().count() < m_nbMaxCompletionWord )
        {
            if (word.length()>= m_minCompletionWordLength  && !word.isEmpty() && m_listCompletion->makeCompletion(word).isEmpty())
            {
                kDebug() << "Adding:" << word << endl;
                m_listCompletion->addItem( word );
                if ( word.length() > m_countMaxWords )
                    m_countMaxWords = word.length();
            }
        }
    }
}

void KoAutoFormat::doAutoIncludeUpperUpper(KoTextCursor* /*textEditCursor*/, KoTextParag *parag, KoTextObject* /*txtObj*/ )
{
    KoTextString *s = parag->string();

    if( s->length() < 2 )
        return;

    for (int i=0; i<=(s->length() - 1);i++)
    {
        QString word;
        for ( int j = i ; j < s->length() - 1; j++ )
        {
            QChar ch = s->at( j ).c;
            if ( ch.isSpace() )
                break;
            word.append( ch );
        }
        if( word.length() > 2 && word.left(2)==word.left(2).upper() && word.at(3)!=word.at(3).upper() )
        {
            if ( m_twoUpperLetterException.findIndex(word )==-1)
                m_twoUpperLetterException.append( word);
        }
        i+=word.length();
    }

}


void KoAutoFormat::doAutoIncludeAbbreviation(KoTextCursor* /*textEditCursor*/, KoTextParag *parag, KoTextObject* /*txtObj*/ )
{
    KoTextString *s = parag->string();
    if( s->length() < 2 )
        return;
    for (int i=0; i<=(s->length() - 1);i++)
    {
        QString wordAfter;
        QString word;

        for ( int j = i ; j < s->length() - 1; j++ )
        {
            QChar ch = s->at( j ).c;
            if ( ch.isSpace() )
                break;
            word.append( ch );
        }
        if ( isMark( word.at(word.length()-1)) )
        {
            for ( int j = i+word.length()+1 ; j < s->length() - 1; j++ )
            {
                QChar ch = s->at( j ).c;
                if ( ch.isSpace() )
                    break;
                wordAfter.append( ch );
            }
            if( word.length()>1 && !wordAfter.isEmpty() && wordAfter.at(0)==wordAfter.at(0).lower())
            {
                if ( m_upperCaseExceptions.findIndex(word )==-1)
                    m_upperCaseExceptions.append( word );
            }
        }
        i+=word.length();
        if( !wordAfter.isEmpty())
        {
            i+=wordAfter.length()+1;
        }
    }

}


KCommand * KoAutoFormat::doAutoChangeFormat( KoTextCursor *textEditCursor, KoTextParag *parag,int index, const QString & word, KoTextObject *txtObj )
{
    bool underline = (word.at(0)=='_' && word.at(word.length()-1)=='_');
    bool bold = (word.at(0)=='*' && word.at(word.length()-1)=='*');
    if( bold || underline)
    {
        QString replacement=word.mid(1,word.length()-2);
        int start = index - word.length();
        KoTextDocument * textdoc = parag->textDocument();
        KMacroCommand *macro=new KMacroCommand(i18n("Autocorrection: Change Format"));
        KoTextCursor cursor( parag->document() );

        cursor.setParag( parag );
        cursor.setIndex( start );
        textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
        cursor.setIndex( start + word.length() );
        textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );
        macro->addCommand(txtObj->replaceSelectionCommand( textEditCursor, replacement,
                                                           i18n("Autocorrect Word"),
                                                           KoTextDocument::HighlightSelection));

        KoTextFormat * lastFormat = parag->at( start )->format();
        KoTextFormat * newFormat = new KoTextFormat(*lastFormat);
        cursor.setIndex( start );
        textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
        cursor.setIndex( start + word.length()-2 );
        textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );

        if( bold)
        {
            newFormat->setBold(true);
            macro->addCommand(txtObj->setFormatCommand( textEditCursor, 0L, newFormat, KoTextFormat::Bold , false,KoTextDocument::HighlightSelection  ));
        }
        else if( underline )
        {
            newFormat->setUnderline(true);
            macro->addCommand(txtObj->setFormatCommand( textEditCursor, 0L, newFormat, KoTextFormat::Underline , false,KoTextDocument::HighlightSelection  ));
        }
        txtObj->emitHideCursor();
        textEditCursor->gotoRight();
        txtObj->emitShowCursor();
        return macro;
    }
    return 0L;
}

KCommand *KoAutoFormat::doUseBulletStyle(KoTextCursor * /*textEditCursor*/, KoTextParag *parag, KoTextObject *txtObj, int& index )
{
    KoTextDocument * textdoc = parag->textDocument();
    KoTextCursor cursor( parag->document() );
    KoTextString *s = parag->string();
    QChar ch = s->at( 0 ).c;

    if( m_useBulletStyle && (ch =='*' || ch == '-' || ch =='+') && (s->at(1).c).isSpace())
    {
        if ( parag->counter() && parag->counter()->numbering() == KoParagCounter::NUM_FOOTNOTE )
            return 0L;
        KMacroCommand *macroCmd = new KMacroCommand( i18n("Autocorrect (use bullet style)"));
        cursor.setParag( parag );
        cursor.setIndex( 0 );
        textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
        cursor.setParag( parag );
        cursor.setIndex( 2 );
        textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );
        KCommand *cmd=txtObj->removeSelectedTextCommand( &cursor, KoTextDocument::HighlightSelection  );
        // Adjust index
        index -= 2;
        if(cmd)
            macroCmd->addCommand(cmd);

        cursor.setParag( parag );
        cursor.setIndex( 0 );
        textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );

        cursor.setIndex( 2 );
        textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );


        KoParagCounter c;
        if( m_bulletStyle.isNull() && (ch == '*' || ch == '+' || ch == '-'))
        {
            if ( ch =='*')
            {
                c.setNumbering( KoParagCounter::NUM_LIST );
                c.setStyle( KoParagCounter::STYLE_DISCBULLET );
            }
            else if ( ch =='+' || ch=='-')
            {
                c.setNumbering( KoParagCounter::NUM_LIST );
                c.setStyle( KoParagCounter::STYLE_CUSTOMBULLET );
                if ( ch =='-' )
                    c.setCustomBulletCharacter( '-' );
                else if ( ch=='+')
                    c.setCustomBulletCharacter( '+' );
            }
        }
        else
        {
            c.setNumbering( KoParagCounter::NUM_LIST );
            c.setStyle( KoParagCounter::STYLE_CUSTOMBULLET );
            c.setCustomBulletCharacter( m_bulletStyle );
        }
        c.setSuffix(QString::null);
        cmd=txtObj->setCounterCommand( &cursor, c ,KoTextDocument::HighlightSelection );
        if( cmd)
            macroCmd->addCommand(cmd);
        if (parag->next() )
                cursor.setParag( parag->next() );
        else
              return 0L;

        cursor.setIndex( 0 );
        textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
        cursor.setIndex( 0 );
        textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );
        cmd=txtObj->setCounterCommand( &cursor, c ,KoTextDocument::HighlightSelection );
        if(cmd)
            macroCmd->addCommand(cmd);
        return macroCmd;
    }
    return 0L;

}

KCommand *KoAutoFormat::doUseNumberStyle(KoTextCursor * /*textEditCursor*/, KoTextParag *parag, KoTextObject *txtObj, int& index )
{
    if ( parag->counter() && parag->counter()->numbering() == KoParagCounter::NUM_FOOTNOTE )
        return 0L;
    KoTextDocument * textdoc = parag->textDocument();
    KoTextCursor cursor( parag->document() );
    KoTextString *s = parag->string();
    QString word;
    for ( int i = 0 ; i < s->length() - 1; i++ )
    {
        QChar ch = s->at( i ).c;
        if ( ch.isSpace() )
            break;
        word.append( ch );
    }
    QChar punct=word[word.length()-1];
    if( punct.isPunct() )
    {
        QString number=word.mid(0,word.length()-1);
        bool ok;
        int val=number.toInt(&ok);
        if( ok )
        {
            KMacroCommand *macroCmd = new KMacroCommand( i18n("Autocorrect (use number style)"));
            cursor.setParag( parag );
            cursor.setIndex( 0 );
            textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
            cursor.setParag( parag );
            cursor.setIndex( word.length()+1 );
            textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );
            KCommand *cmd=txtObj->removeSelectedTextCommand( &cursor, KoTextDocument::HighlightSelection  );
            // Adjust index
            index -= word.length()+1;
            if(cmd)
                macroCmd->addCommand(cmd);

            // Apply counter to this paragraph
            cursor.setParag( parag );
            cursor.setIndex( 0 );
            textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );

            cursor.setIndex( 2 );
            textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );

            KoParagCounter c;
            c.setNumbering( KoParagCounter::NUM_LIST );
            c.setStyle( KoParagCounter::STYLE_NUM );
            c.setSuffix(QString( punct ));
            c.setStartNumber( (int)val);

            // Look at which number this parag will have without a restart counter flag,
            // to see if we need it. Thanks to Shaheed for number() taking a parag as param,
            // so that it works even if the parag doesn't have this counter yet!
            if ( c.number( parag ) != (int)val )
                c.setRestartCounter( true );

            cmd=txtObj->setCounterCommand( &cursor, c, KoTextDocument::HighlightSelection );
            if( cmd)
                macroCmd->addCommand(cmd);
            // Apply counter to next paragraph too
            // but without restart
            c.setRestartCounter( false );
            cursor.setParag( parag->next() );
            cursor.setIndex( 0 );
            textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
            cursor.setIndex( 0 );
            textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );
            cmd=txtObj->setCounterCommand( &cursor, c, KoTextDocument::HighlightSelection );
            if(cmd)
                macroCmd->addCommand(cmd);
            return macroCmd;
        }
    }
    return 0L;
}


KCommand * KoAutoFormat::doRemoveSpaceBeginEndLine( KoTextCursor *textEditCursor, KoTextParag *parag, KoTextObject *txtObj, int &index )
{
    KoTextString *s = parag->string();
    KoTextDocument * textdoc = parag->textDocument();
    KoTextCursor cursor( parag->document() );

    KMacroCommand *macroCmd = 0L;
    // Cut away spaces at end of paragraph
    for ( int i = parag->lastCharPos(); i >= 0; --i )
    {
        QChar ch = s->at( i ).c;
        if ( ch != ' ' )   // was: !ch.isSpace(), but this includes tabs, and this option is only about spaces
        {
            if( i == parag->lastCharPos() )
                break;
            cursor.setParag( parag );
            cursor.setIndex( i+1 );
            textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
            cursor.setParag( parag );
            cursor.setIndex( parag->lastCharPos()+1 );
            textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );
            KCommand *cmd=txtObj->replaceSelectionCommand( &cursor, "", QString::null, KoTextDocument::HighlightSelection );

            if(cmd)
            {
                if ( index > i )
                    index = i;
                if ( !macroCmd )
                    macroCmd = new KMacroCommand( i18n("Autocorrect (remove start and end line space)"));
                macroCmd->addCommand(cmd);
            }
            break;
        }
    }

    // Cut away spaces at start of parag.

    for ( int i = 0 ; i <= parag->lastCharPos() ; i++ )
    {
        QChar ch = s->at( i ).c;
        if ( ch != ' ' )   // was: !ch.isSpace(), but this includes tabs, and this option is only about spaces
        {
            if( i == 0 )
                break;

            cursor.setParag( parag );
            cursor.setIndex( 0 );
            textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
            cursor.setParag( parag );
            cursor.setIndex( i );
            textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );
            KCommand *cmd=txtObj->replaceSelectionCommand( &cursor, "", QString::null, KoTextDocument::HighlightSelection );

            if(cmd)
            {
                index -= i; // adjust index
                if ( !macroCmd )
                    macroCmd = new KMacroCommand( i18n("Autocorrect (remove start and end line space)"));
                macroCmd->addCommand(cmd);
            }
            break;
        }
    }

    if( macroCmd )
    {
        txtObj->emitHideCursor();
        textEditCursor->setParag( parag->next() );
        //textEditCursor->cursorgotoRight();
        txtObj->emitShowCursor();
    }
    return macroCmd;
}

KCommand *KoAutoFormat::doCapitalizeNameOfDays( KoTextCursor* textEditCursor, KoTextParag *parag, int index, const QString & word , KoTextObject *txtObj )
{
    //m_cacheNameOfDays
    //todo
    int pos = m_cacheNameOfDays.findIndex( word.lower() );
    if ( pos == -1 )
        return 0L;
    KoTextDocument * textdoc = parag->textDocument();
    QString replaceStr= m_cacheNameOfDays[pos];
    int start = index - replaceStr.length();
    int length = replaceStr.length();
    if( word.at(0).isLetter() && word.at(0)==word.at(0).lower() )
    {
        KoTextCursor cursor( parag->document() );
        cursor.setParag( parag );
        cursor.setIndex( start );
        textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
        cursor.setIndex( start + length );
        QString replacement = replaceStr.at(0).upper() + replaceStr.right( length-1 );
        textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );
        QString cmdName=i18n("Capitalize Name of Days");
        KCommand *cmd =txtObj->replaceSelectionCommand( textEditCursor, replacement,
                                                        cmdName,
                                                        KoTextDocument::HighlightSelection );
        txtObj->emitHideCursor();
        textEditCursor->gotoRight();
        txtObj->emitShowCursor();
        return cmd;
    }
    return 0L;
}

KCommand *KoAutoFormat::doAutoSuperScript( KoTextCursor* textEditCursor, KoTextParag *parag, int index, const QString & word , KoTextObject *txtObj )
{
    KoAutoFormatEntryMap::Iterator it = m_superScriptEntries.begin();
    bool found = false;
    QString replace;
    for ( ; it != m_superScriptEntries.end() ; ++it )
    {
        if( it.key()==word)
        {
            replace = it.data().replace();
            found = true;
            break;
        }
        else if ( it.key()=="othernb")
        {
            QString tmp = it.data().replace();
            int pos = word.find( tmp );
            if( pos != -1)
            {
                if( pos + tmp.length() == word.length())
                {
                    bool ok;
                    word.left( pos ).toInt( &ok);
                    if( ok )
                    {
                        replace = tmp;
                        found = true;
                        break;
                    }
                }
            }
        }
    }
    if (found )
    {
        KoTextDocument * textdoc = parag->textDocument();

        int start = index - replace.length();
        KoTextFormat * lastFormat = parag->at( start )->format();
        KoTextFormat * newFormat = new KoTextFormat(*lastFormat);
        KoTextCursor cursor( parag->document() );

        cursor.setParag( parag );
        cursor.setIndex( start );
        textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
        cursor.setIndex( start + word.length() -1 );
        textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );
        newFormat->setVAlign(KoTextFormat::AlignSuperScript);
        KCommand *cmd =txtObj->setFormatCommand( textEditCursor, 0L, newFormat, KoTextFormat::VAlign , false,KoTextDocument::HighlightSelection  );
        textdoc->removeSelection( KoTextDocument::HighlightSelection );

        return cmd;
    }
    return 0L;
}

bool KoAutoFormat::doIgnoreDoubleSpace( KoTextParag *parag, int index, QChar ch )
{
    if( m_ignoreDoubleSpace && ch==' ' && index >=  0 && !parag->hasAnySelection() )
    {
        KoTextString *s = parag->string();
        QChar ch = s->at( index ).c;
        if ( ch==' ' )
          return true;
    }
    return false;
}

void KoAutoFormat::configTypographicSimpleQuotes( TypographicQuotes _tq )
{
    m_typographicSimpleQuotes = _tq;
}

void KoAutoFormat::configTypographicDoubleQuotes( TypographicQuotes _tq )
{
    m_typographicDoubleQuotes = _tq;
}

void KoAutoFormat::configUpperCase( bool _uc )
{
    m_convertUpperCase = _uc;
}

void KoAutoFormat::configUpperUpper( bool _uu )
{
    m_convertUpperUpper = _uu;
}

void KoAutoFormat::configAdvancedAutocorrect( bool _aa )
{
    m_advancedAutoCorrect = _aa;
}

void KoAutoFormat::configAutoDetectUrl(bool _au)
{
    m_autoDetectUrl=_au;
}

void KoAutoFormat::configIgnoreDoubleSpace( bool _ids)
{
    m_ignoreDoubleSpace=_ids;
}

void KoAutoFormat::configRemoveSpaceBeginEndLine( bool _space)
{
    m_removeSpaceBeginEndLine=_space;
}

void KoAutoFormat::configUseBulletStyle( bool _ubs)
{
    m_useBulletStyle=_ubs;
}

void KoAutoFormat::configBulletStyle( QChar b )
{
    m_bulletStyle = b;
}

void KoAutoFormat::configAutoChangeFormat( bool b)
{
    m_autoChangeFormat = b;
}


void KoAutoFormat::configAutoReplaceNumber( bool b )
{
    m_autoReplaceNumber = b;
}

void KoAutoFormat::configAutoNumberStyle( bool b )
{
    m_useAutoNumberStyle = b;
}

void KoAutoFormat::configCompletion( bool b )
{
    m_completion = b;
}

void KoAutoFormat::configToolTipCompletion( bool b )
{
    m_toolTipCompletion = b;
    if (!b && m_completionBox)
    {
        delete m_completionBox;
        m_completionBox = 0;
    }
}

void KoAutoFormat::configKeyCompletionAction( KeyCompletionAction action )
{
    m_keyCompletionAction = action;
}

void KoAutoFormat::configAppendSpace( bool b)
{
    m_completionAppendSpace= b;
}

void KoAutoFormat::configMinWordLength( int val )
{
   m_minCompletionWordLength = val;
}

void KoAutoFormat::configNbMaxCompletionWord( int val )
{
    m_nbMaxCompletionWord = val;
}


void KoAutoFormat::configAddCompletionWord( bool b )
{
    m_addCompletionWord= b;
}

bool KoAutoFormat::isUpper( const QChar &c )
{
    return c.lower() != c;
}

bool KoAutoFormat::isLower( const QChar &c )
{
    // Note that this is not the same as !isUpper !
    // For instance '1' is not lower nor upper,
    return c.upper() != c;
}

bool KoAutoFormat::isMark( const QChar &c )
{
    return ( c == QChar( '.' ) ||
	     c == QChar( '?' ) ||
	     c == QChar( '!' ) );
}

bool KoAutoFormat::isSeparator( const QChar &c )
{
    return ( !c.isLetter() && !c.isNumber() && !c.isDigit() );
}

void KoAutoFormat::buildMaxLen()
{
    m_maxFindLength = 0;
    Q3DictIterator<KoAutoFormatEntry> it( m_entries );
    for( ; it.current(); ++it )
    {
	m_maxFindLength = qMax( m_maxFindLength, it.currentKey().length() );
    }
    Q3DictIterator<KoAutoFormatEntry> it2( m_allLanguages );
    for( ; it2.current(); ++it2 )
    {
	m_maxFindLength = qMax( m_maxFindLength, it2.currentKey().length() );
    }
}

QStringList KoAutoFormat::listCompletion() const
{
   return m_listCompletion->items();
}


void KoAutoFormat::configIncludeTwoUpperUpperLetterException( bool b)
{
    m_includeTwoUpperLetterException = b;
}

void KoAutoFormat::configIncludeAbbreviation( bool b )
{
    m_includeAbbreviation = b;
}

void KoAutoFormat::configAutoSuperScript( bool b )
{
    m_bAutoSuperScript = b;
}

void KoAutoFormat::configCorrectionWithFormat( bool b)
{
    m_bAutoCorrectionWithFormat = b;
}

void KoAutoFormat::configCapitalizeNameOfDays( bool b)
{
    m_bCapitalizeNameOfDays = b;
}

void KoAutoFormat::configAutoFormatLanguage( const QString &_lang)
{
    m_autoFormatLanguage=_lang;
}

KCommand *KoAutoFormat::applyAutoFormat( KoTextObject * obj )
{
  KoTextParag * parag = obj->textDocument()->firstParag();
  KoTextCursor *cursor = new KoTextCursor( obj->textDocument() );
  KMacroCommand *macro = 0L;
  while ( parag )
  {
    cursor->setIndex(0);
    for (int i=0;i<parag->length();i++)
    {
      cursor->gotoRight();
      //kDebug() << "ch:" << parag->string()->at(i).c << endl;
      if (i == parag->length()-1)
	doAutoFormat(cursor,parag,i,'\n',obj);
      else
	doAutoFormat(cursor,parag,i, parag->string()->at(i).c,obj);
    }
    parag = parag->next();

  }
  delete cursor;
  return macro;
}

void KoAutoFormat::changeTextFormat(KoSearchContext *formatOptions, KoTextFormat * format, int & flags )
{
    if (formatOptions )
    {
        if (formatOptions->m_optionsMask & KoSearchContext::Bold)
        {
            format->setBold( formatOptions->m_options & KoSearchContext::Bold);
            flags |=KoTextFormat::Bold;
        }
        if ( formatOptions->m_optionsMask & KoSearchContext::Size)
        {
            format->setPointSize( formatOptions->m_size );
            flags |=KoTextFormat::Size;
        }
        if ( formatOptions->m_optionsMask & KoSearchContext::Family)
        {
            format->setFamily( formatOptions->m_family );
            flags |=KoTextFormat::Family;
        }
        if ( formatOptions->m_optionsMask & KoSearchContext::Color)
        {
            format->setColor(formatOptions->m_color);
            flags |=KoTextFormat::Color;
        }
        if ( formatOptions->m_optionsMask & KoSearchContext::BgColor)
        {
            format->setTextBackgroundColor(formatOptions->m_backGroundColor);
            flags |=KoTextFormat::TextBackgroundColor;
        }

        if ( formatOptions->m_optionsMask & KoSearchContext::Italic)
        {
            format->setItalic( formatOptions->m_options & KoSearchContext::Italic);
            flags |=KoTextFormat::Italic;
        }
        if ( formatOptions->m_optionsMask & KoSearchContext::WordByWord)
        {
            format->setWordByWord( formatOptions->m_options & KoSearchContext::WordByWord );
            flags |=KoTextFormat::WordByWord;
        }
        if ( formatOptions->m_optionsMask & KoSearchContext::Shadow)
        {
            if ( formatOptions->m_options & KoSearchContext::Shadow )
                format->setShadow( 1, 1, Qt::gray );
            else
                format->setShadow( 0, 0, QColor() );
            flags |=KoTextFormat::ShadowText;
        }

        if ( formatOptions->m_optionsMask & KoSearchContext::Underline)
        {
            format->setUnderlineType(formatOptions->m_underline);
            flags |=KoTextFormat::ExtendUnderLine;
        }
        if ( formatOptions->m_optionsMask & KoSearchContext::StrikeOut)
        {
            format->setStrikeOutType(formatOptions->m_strikeOut);
            flags |= KoTextFormat::StrikeOut;
        }
        if ( formatOptions->m_optionsMask & KoSearchContext::VertAlign)
        {
            format->setVAlign(formatOptions->m_vertAlign);
            flags |=KoTextFormat::VAlign;
        }
        if ( formatOptions->m_optionsMask & KoSearchContext::Attribute)
        {
            format->setAttributeFont(formatOptions->m_attribute);
            flags |= KoTextFormat::Attribute;
        }
        if (formatOptions->m_optionsMask & KoSearchContext::Language)
        {
            flags |= KoTextFormat::Language;
            format->setLanguage( formatOptions->m_language );
        }
    }
}

#include "KoAutoFormat.moc"
