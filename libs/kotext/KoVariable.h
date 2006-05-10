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

#ifndef kovariable_h
#define kovariable_h

#include <QString>
#include <QDateTime>
#include <q3asciidict.h>
#include <q3ptrlist.h>
#include <QMap>
#include <QObject>
//Added by qt3to4:
#include <QString>
#include <Q3ValueList>
#include <kaction.h>
#include "KoRichText.h"
#include <QVariant>
#include <koffice_export.h>

class QDomElement;
// Always add new types at the _end_ of this list (but before VT_ALL of course).
// (and update KWView::setupActions)
enum VariableType {
    VT_NONE             = -1,

    VT_DATE             = 0,
    VT_DATE_VAR_KWORD10 = 1,
    VT_TIME             = 2,
    VT_TIME_VAR_KWORD10 = 3,
    VT_PGNUM            = 4,
    // No number 5
    VT_CUSTOM           = 6,
    VT_MAILMERGE        = 7,
    VT_FIELD            = 8,
    VT_LINK             = 9,
    VT_NOTE             = 10,
    VT_FOOTNOTE         = 11,
    VT_STATISTIC        = 12,

    VT_ALL              = 256
};

enum VariableFormat {
    VF_DATE   = 0,
    VF_TIME   = 1,
    VF_STRING = 2,
    VF_NUM    = 3
};

class KoVariable;
class KoOasisSettings;

class KOTEXT_EXPORT KoVariableSettings
{
 public:
    KoVariableSettings();
    virtual ~KoVariableSettings();

    int startingPageNumber()const { return m_startingPageNumber; }
    void setStartingPageNumber(int num) { m_startingPageNumber=num; }

    bool displayLink() const{ return m_displayLink; }
    void setDisplayLink( bool b) { m_displayLink=b; }

    bool underlineLink() const { return m_underlineLink; }
    void setUnderlineLink( bool b) { m_underlineLink=b; }

    bool displayComment() const { return m_displayComment; }
    void setDisplayComment( bool b) { m_displayComment=b; }

    bool displayFieldCode() const { return m_displayFieldCode; }
    void setDisplayFieldCode( bool b) { m_displayFieldCode=b; }

    virtual void save( QDomElement &parentElem );
    virtual void load( QDomElement &elem );

    QDateTime lastPrintingDate() const;
    void setLastPrintingDate( const QDateTime & _date);

    QDateTime creationDate() const;
    void setCreationDate( const QDateTime & _date);

    QDateTime modificationDate() const;
    void setModificationDate( const QDateTime & _date);

    virtual void saveOasis( KoXmlWriter &settingsWriter ) const;
    virtual void loadOasis(const KoOasisSettings&settingsDoc);


 private:
    int m_startingPageNumber;
    bool m_displayLink;
    bool m_displayComment;
    bool m_underlineLink;
    bool m_displayFieldCode;
    class KoVariableSettingPrivate;
    KoVariableSettingPrivate *d;
};

/**
 * Class: KoVariableFormat
 * Base class for a variable format - held by KWDocument.
 * Example of formats are time, date, string, number, floating-point number...
 * The reason for formats to be separated is that it allows to
 * customize the formats, to implement subformats (various date formats, etc.).
 */
class KOTEXT_EXPORT KoVariableFormat
{
public:
    KoVariableFormat() {}
    virtual ~KoVariableFormat() {}
    /**
     * Return a key describing this format.
     * Used for the flyweight pattern in KoVariableFormatCollection
     */
    virtual QString key() const = 0;
    /**
     * @return the key for a given set of properties.
     * Use this key to lookup the format in the "variable format" collection.
     * @param props properties of this format, e.g. DD/MM/YYYY for a date format.
     */
    virtual QString getKey( const QString& props ) const = 0;
    /**
     * Create a format from this key.
     */
    virtual void load( const QString &key ) = 0;
    /**
     * Use this format to convert a piece of data into a string.
     */
    virtual QString convert(const QVariant& data ) const = 0;
    /**
     * Set the properties of this format, e.g. DD/MM/YYYY for a date format.
     * WARNING: if you call this, you might be modifying a format that
     * other variables use as well. Don't do it, use getKey.
     */
    virtual void setFormatProperties( const QString& ) {}
    /**
     * @return the properties of this format, e.g. DD/MM/YYYY for a date format.
     */
    virtual QString formatProperties() const { return QString::null; }
    /**
     * @return the list of available properties strings (e.g. hh:mm:ss)
     */
    virtual QStringList formatPropsList() const { return QStringList(); }
    /**
     * @return the translated version of the list of format properties
     */
    virtual QStringList translatedFormatPropsList() const { return QStringList(); }
};

/**
 * Implementation of the "date" formats
 * TODO: merge with KoVariableTimeFormat, for a single QDateTime-based class.
 */
class KOTEXT_EXPORT KoVariableDateFormat : public KoVariableFormat
{
public:
    KoVariableDateFormat();
    virtual QString convert(const QVariant& data ) const;
    virtual QString key() const;
    virtual QString getKey( const QString& props ) const;
    virtual void load( const QString &key );

    /// Set the format string (e.g. DDMMYYYY)
    virtual void setFormatProperties( const QString& props ) {
        m_strFormat = props;
    }
    /// @return the format string (e.g. DDMMYYYY)
    virtual QString formatProperties() const { return m_strFormat; }

    /// @return the list of available format strings
    virtual QStringList formatPropsList() const { return staticFormatPropsList(); }

    /// @return the translated version of the list of formats
    virtual QStringList translatedFormatPropsList() const { return staticTranslatedFormatPropsList(); }

    static QStringList staticFormatPropsList();
    static QStringList staticTranslatedFormatPropsList();

private:
    QString m_strFormat;
};

/**
 * Implementation of the "time" formats
 */
class KOTEXT_EXPORT KoVariableTimeFormat : public KoVariableFormat
{
public:
    KoVariableTimeFormat();
    virtual QString convert(const QVariant& data ) const;
    virtual QString key() const;
    virtual QString getKey( const QString& props ) const;
    virtual void load( const QString & /*key*/ );

    /// Set the format string (e.g. hh:mm:ss)
    virtual void setFormatProperties( const QString& props ) {
        m_strFormat = props;
    }
    /// @return the format string (e.g. hh:mm:ss)
    virtual QString formatProperties() const { return m_strFormat; }

    /// @return the list of available properties strings (e.g. hh:mm:ss)
    virtual QStringList formatPropsList() const { return staticFormatPropsList(); }

    /// @return the translated version of the list of format properties
    virtual QStringList translatedFormatPropsList() const { return staticTranslatedFormatPropsList(); }

    static QStringList staticFormatPropsList();
    static QStringList staticTranslatedFormatPropsList();

private:
    QString m_strFormat;
};


/**
 * Implementation of the string format
 */
class KoVariableStringFormat : public KoVariableFormat
{
public:
    KoVariableStringFormat() : KoVariableFormat() {}
    virtual QString convert(const QVariant& data ) const;
    virtual QString key() const;
    virtual QString getKey( const QString& props ) const;
    virtual void load( const QString & /*key*/ ) {}
};


class KOTEXT_EXPORT KoVariableNumberFormat : public KoVariableFormat
{
public:
    KoVariableNumberFormat() : KoVariableFormat() {}
    virtual QString convert(const QVariant& data ) const;
    virtual QString key() const;
    virtual QString getKey( const QString& props ) const;
    virtual void load( const QString & /*key*/ ) {}
};


/**
 * The collection of formats for variables.
 * Example: date (short or long), time, string (prefix/suffix), number (prefix/suffix, decimals?)...
 * Implements the flyweight pattern to share formats and create them on demand.
 * Each KoDocument holds a KoVariableFormatCollection.
 */
class KOTEXT_EXPORT KoVariableFormatCollection
{
public:
    KoVariableFormatCollection();

    /**
     * Forget (and erase) all the formats this collection knows about
     */
    void clear() { m_dict.clear(); }

    /**
     * Find or create the format for the given @p key
     */
    KoVariableFormat *format( const QString &key );

    // TODO Refcounting and removing unused formats
    // Not critical, that we don't delete unused formats until closing the doc...
protected:
    KoVariableFormat *createFormat( const QString &key );

private:
    Q3AsciiDict<KoVariableFormat> m_dict;
};

class KoVariable;
class KoVariableFormat;
class KoDocument;
class KoVariableFormatCollection;
class KoTextDocument;
class KoVariableCollection;
class KOTEXT_EXPORT KoVariableCollection : public QObject
{
    Q_OBJECT
public:
    // Note that the KoVariableSettings becomes owned by the collection;
    // we take it as argument so that it can be subclassed though.
    KoVariableCollection(KoVariableSettings *settings, KoVariableFormatCollection *formatCollection);
    ~KoVariableCollection();
    void registerVariable( KoVariable *var );
    void unregisterVariable( KoVariable *var );
    Q3ValueList<KoVariable *> recalcVariables(int type);

    // For custom variables
    void setVariableValue( const QString &name, const QString &value );
    QString getVariableValue( const QString &name ) const;

    const Q3PtrList<KoVariable>& getVariables() const {
        return variables;
    }
    void clear();

    bool customVariableExist(const QString &varname)const ;

    virtual KoVariable *createVariable( int type, short int subtype, KoVariableFormatCollection * coll, KoVariableFormat *varFormat,KoTextDocument *textdoc, KoDocument * doc, int _correct , bool _forceDefaultFormat=false, bool loadFootNote= true );

    /// Load variable from OASIS file format (called "field" in the OASIS format)
    virtual KoVariable* loadOasisField( KoTextDocument* textdoc, const QDomElement& tag, KoOasisContext& context );
    virtual KoVariable* loadOasisFieldCreateVariable( KoTextDocument* textdoc, const QDomElement& tag, KoOasisContext& context, const QString &key, int type );

    KoVariableSettings *variableSetting() const { return m_variableSettings; }
    KoVariableFormatCollection *formatCollection() const { return m_formatCollection; }

    /// Variable that's under the popupmenu
    void setVariableSelected(KoVariable * var);
    KoVariable *selectedVariable()const {return m_varSelected;}

    /// List of KActions to put into the popupmenu on a variable
    QList<KAction*> popupActionList() const;

 protected slots:
    // This is here because variables and formats are not QObjects
    void slotChangeSubType();
    void slotChangeFormat();

 private:
    //typedef QMap<KAction *, int> VariableSubTextMap;
    //VariableSubTextMap m_variableSubTextMap;

    Q3PtrList<KoVariable> variables;
    QMap< QString, QString > varValues; // for custom variables
    KoVariableSettings *m_variableSettings;
    KoVariable *m_varSelected;
    KoVariableFormatCollection *m_formatCollection;
};


// ----------------------------------------------------------------
//                KoVariable and derived classes


class KoDocument;
class KoVariable;
class QDomElement;
class KoTextFormat;


/**
 * A KoVariable is a custom item, i.e. considered as a single character.
 * KoVariable is the abstract base class.
 */
class KOTEXT_EXPORT KoVariable : public KoTextCustomItem
{
public:
    KoVariable( KoTextDocument *fs, KoVariableFormat *varFormat,KoVariableCollection *varColl );
    virtual ~KoVariable();

    virtual VariableType type() const = 0;
    virtual short int subType() const { return 0; }

    // KoTextCustomItem stuff
    virtual Placement placement() const { return PlaceInline; }
    virtual void resize();
    virtual int ascent() const { return m_ascent; } // for text, ascent != height!
    virtual int widthHint() const { return width; }
    virtual int minimumWidth() const { return width; }
    virtual void drawCustomItem( QPainter* p, int x, int y, int wpix, int hpix, int ascentpix, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected, int offset,  bool drawingShadow);

    /**
     * Called by drawCustomItem. Some special variables can
     * reimplement drawCustomItem to change the parameters passed to
     * drawCustomItemHelper
     */
    void drawCustomItemHelper( QPainter* p, int x, int y, int wpix, int hpix, int ascentpix, const QColorGroup& cg, bool selected, int offset, KoTextFormat* fmt, const QFont& font, QColor textColor, bool drawingShadow );

    void setVariableFormat( KoVariableFormat *_varFormat );

    KoVariableFormat *variableFormat() const
        { return m_varFormat; }

    KoVariableCollection *variableColl() const
        { return m_varColl; }

    /**
     * Returns the text to be displayed for this variable
     * It doesn't need to be cached, convert() is fast, and it's the actual
     * value (date, time etc.) that is cached in the variable already.
     */
    virtual QString text(bool realValue=false);

    virtual QString fieldCode();

    /// Return the variable value, as a QVariant, before format conversion
    QVariant varValue() const { return m_varValue; }

    /**
     * Ask this variable to recalculate and to repaint itself
     * Only use this if you're working on a single variable (e.g. popupmenu).
     * Otherwise, better do the repainting all at once.
     * @see KoVariableCollection::recalcVariables()
     */
    void recalcAndRepaint();

    /** Save the variable. Public API, does the common job and then calls saveVariable. */
    virtual void save( QDomElement &parentElem );
    virtual void saveOasis( KoXmlWriter& writer, KoSavingContext& context ) const;
    virtual void load( QDomElement &elem );
    virtual void loadOasis( const QDomElement &elem, KoOasisContext& context );

    /** Part of the KoTextCustomItem interface. Returns the code for a variable, see DTD.
      * Do NOT reimplement in koVariable-derived classes.
      */
    virtual int typeId() const { return 4; }

    /// List of available subtypes (translated). Use variableSubType() to map index to ID
    virtual QStringList subTypeList();

    /// Set this variable's subtype.
    virtual void setVariableSubType( short int /*subtype*/ ) {}

    /**
     * Converts the @p menuNumber to variable subtype number (VST_x)
     */
    virtual short int variableSubType(short int menuNumber){ return menuNumber; }

    QString convertKlocaleToQDateTimeFormat( const QString & _format );

protected:
    /** Variable should reimplement this to implement saving. */
    virtual void saveVariable( QDomElement &parentElem ) = 0;
    virtual int correctValue() const { return 0;}

    KoVariableFormat *m_varFormat;
    KoVariableCollection *m_varColl;
    QVariant m_varValue;
    int m_ascent;

    //typedef QMap<KAction *, int> SubTextMap;
    //SubTextMap m_subTextMap;
    class Private;
    Private *d;
};


/**
 * Date-related variables
 */
class KOTEXT_EXPORT KoDateVariable : public KoVariable
{
public:
    KoDateVariable( KoTextDocument *textdoc, short int subtype, KoVariableFormat *_varFormat,KoVariableCollection *_varColl , int _correctDate = 0);

    virtual VariableType type() const
    { return VT_DATE; }

    enum { VST_DATE_FIX = 0, VST_DATE_CURRENT = 1, VST_DATE_LAST_PRINTING = 2, VST_DATE_CREATE_FILE = 3, VST_DATE_MODIFY_FILE =4 };
    static QStringList actionTexts();

    virtual void recalc();
    virtual QString fieldCode();
    virtual void resize();
    void setDate( const QDate & _date ) { m_varValue = QVariant(_date); }

    virtual void saveVariable( QDomElement &parentElem );
    virtual int correctValue() const { return m_correctDate;}
    virtual void load( QDomElement &elem );
    virtual void loadOasis( const QDomElement &elem, KoOasisContext& context );
    virtual void saveOasis( KoXmlWriter& writer, KoSavingContext& context ) const;

    virtual QStringList subTypeList();
    /// Set this variable's subtype.
    virtual void setVariableSubType( short int subtype )
        { m_subtype = subtype; }
    virtual short int subType() const { return m_subtype; }
    /**
     * Ask the user and return the date format string with prefix "DATE"
     */
    static QString formatStr( int & correct );
    /**
     * Return the default date format for old file.
     */
    static QString defaultFormat();

protected:
    short int m_subtype;
    int m_correctDate;
};


/**
 * Time-related variables
 */
class KOTEXT_EXPORT KoTimeVariable : public KoVariable
{
public:
    KoTimeVariable( KoTextDocument *textdoc, short int subtype, KoVariableFormat *varFormat, KoVariableCollection *_varColl,  int _correct);

    virtual VariableType type() const
    { return VT_TIME; }

    enum { VST_TIME_FIX = 0, VST_TIME_CURRENT = 1 };
    static QStringList actionTexts();

    virtual void recalc();
    virtual void resize();
    virtual QString fieldCode();

    void setTime( const QTime & _time ) { m_varValue = QVariant(_time); }

    virtual void saveVariable( QDomElement &parentElem );
    virtual int correctValue() const { return m_correctTime;}
    virtual void load( QDomElement &elem );
    virtual void loadOasis( const QDomElement &elem, KoOasisContext& context );
    virtual void saveOasis( KoXmlWriter& writer, KoSavingContext& context ) const;

    virtual QStringList subTypeList();
    virtual void setVariableSubType( short int subtype )
        { m_subtype = subtype; }
    virtual short int subType() const { return m_subtype; }
    /**
     * Returns the time format string with prefix "TIME"
     */
    static QString formatStr(int & _correct);
    /**
     * Return the default date format for old file.
     */
    static QString defaultFormat();

protected:
    short int m_subtype;
    int m_correctTime; // in minutes
};



/**
 * A custom variable is a variable whose value is entered
 * by the user.
 */
class KOTEXT_EXPORT KoCustomVariable : public KoVariable
{
public:
    KoCustomVariable(KoTextDocument *textdoc , const QString &name, KoVariableFormat *varFormat,KoVariableCollection *_varcoll );

    virtual VariableType type() const
    { return VT_CUSTOM; }
    static QStringList actionTexts();

    virtual void saveVariable( QDomElement &parentElem );
    virtual void load( QDomElement &elem );
    virtual void loadOasis( const QDomElement &elem, KoOasisContext& context );
    virtual void saveOasis( KoXmlWriter& writer, KoSavingContext& context ) const;

    QString name() const { return m_varValue.toString(); }
    virtual void recalc();
    virtual QString fieldCode();

    virtual QString text(bool realValue=false);

    QString value() const;
    void setValue( const QString &v );

protected:
};


/**
 * Any variable that is a string, and whose value is automatically
 * determined - as opposed to custom variables whose value is
 * entered by the user
 */
class KOTEXT_EXPORT KoFieldVariable : public KoVariable
{
public:
    KoFieldVariable( KoTextDocument *textdoc, short int subtype, KoVariableFormat *varFormat,KoVariableCollection *_varColl, KoDocument *_doc );

    // Do not change existing values, they are saved in document files
    enum FieldSubType { VST_NONE = -1,
                        VST_FILENAME = 0, VST_DIRECTORYNAME = 1,
                        VST_AUTHORNAME = 2, VST_EMAIL = 3, VST_COMPANYNAME = 4,
			VST_PATHFILENAME = 5, VST_FILENAMEWITHOUTEXTENSION=6,
                        VST_TELEPHONE_WORK = 7, VST_FAX = 8, VST_COUNTRY = 9,
                        VST_TITLE = 10, VST_ABSTRACT = 11,
                        VST_POSTAL_CODE = 12, VST_CITY = 13, VST_STREET = 14,
                        VST_AUTHORTITLE = 15, VST_INITIAL = 16, VST_TELEPHONE_HOME = 17, VST_SUBJECT = 18, VST_KEYWORDS=19,VST_AUTHORPOSITION = 20 };

    virtual VariableType type() const
    { return VT_FIELD; }

    virtual void saveVariable( QDomElement &parentElem );
    virtual void load( QDomElement &elem );
    virtual void loadOasis( const QDomElement &elem, KoOasisContext& context );
    virtual void saveOasis( KoXmlWriter& writer, KoSavingContext& context ) const;
    virtual QString fieldCode();

    virtual void recalc();
    virtual QString text(bool realValue=false);

    QString value() const { return m_varValue.toString(); }

    static QStringList actionTexts();
    /**
     * Converts @p menuNumber to field variable subtype number
     */
    virtual short int variableSubType( short int menuNumber );
    /**
     * Converts @p menuNumber to field variable subtype number
     */
    static FieldSubType fieldSubType( short int menuNumber);

    virtual QStringList subTypeList();
    virtual void setVariableSubType( short int subtype )
        { m_subtype = subtype; }
    virtual short int subType() const { return m_subtype; }

protected:
    short int m_subtype;
    KoDocument *m_doc;
};


class KOTEXT_EXPORT KoMailMergeVariable : public KoVariable
{
public:
    KoMailMergeVariable( KoTextDocument *textdoc, const QString &name, KoVariableFormat *varFormat, KoVariableCollection *_varColl );

    virtual VariableType type() const
    { return VT_MAILMERGE; }
    static QStringList actionTexts();
    virtual QString fieldCode();

    virtual void saveVariable( QDomElement &parentElem );
    virtual void load( QDomElement &elem );
    virtual void loadOasis( const QDomElement &elem, KoOasisContext& context );
    virtual void saveOasis( KoXmlWriter& writer, KoSavingContext& context ) const;

    virtual QString text(bool realValue=false);
    QString name() const { return m_varValue.toString(); }
    virtual QString value() const;

protected:
};


/**
 * "current page number" and "number of pages" variables
 * This is a base class, it must be inherited by applications,
 * to provide recalc().
 */
class KOTEXT_EXPORT KoPageVariable : public KoVariable
{
public:
    KoPageVariable( KoTextDocument *textdoc, short int subtype, KoVariableFormat *varFormat ,KoVariableCollection *_varColl);

    virtual VariableType type() const
    { return VT_PGNUM; }

    enum { VST_PGNUM_CURRENT = 0, VST_PGNUM_TOTAL = 1, VST_CURRENT_SECTION = 2 , VST_PGNUM_PREVIOUS = 3, VST_PGNUM_NEXT = 4 };
    static QStringList actionTexts();
    virtual QString fieldCode();

    virtual QStringList subTypeList();

    virtual void setVariableSubType( short int subtype );

    // For the 'current page' variable. This is called by the app e.g. when painting
    // a given page (see KWTextFrameSet::drawFrame and KPTextObject::recalcPageNum)
    void setPgNum( int pgNum ) { m_varValue = QVariant( pgNum); }
    // For the 'current section title' variable. Same thing.
    void setSectionTitle( const QString& title );

    virtual short int subType() const { return m_subtype; }

    virtual void recalc() = 0;

    virtual void saveVariable( QDomElement &parentElem );
    virtual void load( QDomElement &elem );
    virtual void loadOasis( const QDomElement &elem, KoOasisContext& context );
    virtual void saveOasis( KoXmlWriter& writer, KoSavingContext& context ) const;
protected:
    short int m_subtype;
};


class KOTEXT_EXPORT KoLinkVariable : public KoVariable
{
public:
    KoLinkVariable( KoTextDocument *textdoc, const QString & _linkName, const QString & _ulr,KoVariableFormat *varFormat, KoVariableCollection *_varColl );
    virtual void drawCustomItem( QPainter* p, int x, int y, int wpix, int hpix, int ascentpix, int /*cx*/, int /*cy*/, int /*cw*/, int /*ch*/, const QColorGroup& cg, bool selected, int offset, bool drawingShadow );

    virtual VariableType type() const
    { return VT_LINK; }

    static QStringList actionTexts();
    virtual QString fieldCode();

    virtual void saveVariable( QDomElement &parentElem );
    virtual void load( QDomElement &elem );
    virtual void loadOasis( const QDomElement &elem, KoOasisContext& context );
    virtual void saveOasis( KoXmlWriter& writer, KoSavingContext& context ) const;

    virtual QString text(bool realValue=false);
    QString value() const { return m_varValue.toString(); }
    QString url() const { return m_url;}

    virtual void recalc();

    void setLink(const QString & _linkName, const QString &_url)
	{
	    m_varValue=QVariant(_linkName);
	    m_url=_url;
	}

protected:
    QString m_url;
};


// A custom item that display a small yellow rect. Right-clicking on it shows the comment.
class KOTEXT_EXPORT KoNoteVariable : public KoVariable
{
public:
    KoNoteVariable( KoTextDocument *textdoc, const QString & _note,KoVariableFormat *varFormat, KoVariableCollection *_varColl );
    virtual void drawCustomItem( QPainter* p, int x, int y, int wpix, int hpix, int ascentpix, int /*cx*/, int /*cy*/, int /*cw*/, int /*ch*/, const QColorGroup& cg, bool selected, int offset, bool drawingShadow );

    virtual VariableType type() const
    { return VT_NOTE; }

    static QStringList actionTexts();
    virtual QString fieldCode();

    virtual void saveVariable( QDomElement &parentElem );
    virtual void load( QDomElement &elem );
    virtual void loadOasis( const QDomElement &elem, KoOasisContext& context );
    virtual void saveOasis( KoXmlWriter& writer, KoSavingContext& context ) const;

    virtual QString text(bool realValue=false);
    QString note() const { return m_varValue.toString(); }
    void setNote( const QString & _note) { m_varValue = QVariant(_note); }
    virtual void recalc();

    QString createdNote() const;
protected:
    QDate m_createdNoteDate;
};


class KOTEXT_EXPORT KoStatisticVariable : public KoVariable
{
public:
    KoStatisticVariable( KoTextDocument *textdoc, short int subtype,
			 KoVariableFormat *varFormat,
			 KoVariableCollection *_varColl);

    enum {
	VST_STATISTIC_NB_WORD                      = 0,
	VST_STATISTIC_NB_SENTENCE                  = 1,
	VST_STATISTIC_NB_LINES                     = 2,
	VST_STATISTIC_NB_CHARACTERE                = 3,
	VST_STATISTIC_NB_NON_WHITESPACE_CHARACTERE = 4,
	VST_STATISTIC_NB_SYLLABLE                  = 5,
	VST_STATISTIC_NB_FRAME                     = 6,
	VST_STATISTIC_NB_EMBEDDED                  = 7,
	VST_STATISTIC_NB_PICTURE                   = 8,
	VST_STATISTIC_NB_TABLE                     = 9
    };

    virtual VariableType type() const
    { return VT_STATISTIC; }
    static QStringList actionTexts();

    virtual QStringList subTypeList();

    virtual void saveVariable( QDomElement &parentElem );
    virtual void load( QDomElement &elem );
    virtual void loadOasis( const QDomElement &elem, KoOasisContext& context );
    virtual void saveOasis( KoXmlWriter& writer, KoSavingContext& context ) const;

    virtual short int subType() const { return m_subtype; }

    virtual void setVariableSubType( short int subtype );

    QString name() const { return m_varValue.toString(); }
    virtual QString fieldCode();

    QString value() const;
    void setValue( const QString &v );
    static void setExtendedType( bool _b) { m_extendedType = _b; }
    static bool extendedType() { return m_extendedType; }
protected:
    short int m_subtype;
    // extend type for kword.
    static bool m_extendedType;
};


#endif
