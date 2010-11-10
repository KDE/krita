#include "KoStyle.h"
#include "KoGenStyles.h"

KoStyle::KoStyle()
: m_autoStyle(false)
{
}

KoStyle::~KoStyle()
{
}

void KoStyle::setName(QString name)
{
    m_name = name;
}

QString KoStyle::name() const
{
    return m_name;
}

KoGenStyles::InsertionFlags KoStyle::insertionFlags() const
{
    if(m_name.isEmpty()) {
        return KoGenStyles::NoFlag;
    }
    else {
        return KoGenStyles::DontAddNumberToName | KoGenStyles::AllowDuplicates;
    }
}

QString KoStyle::saveOdf(KoGenStyles& styles) const
{
    KoGenStyle::Type type;
    if(m_name.isEmpty()) {
        type = automaticstyleType();
    }
    else {
        type = styleType();
    }
    KoGenStyle style(type, styleFamilyName());
    prepareStyle(style);

    QString styleName = m_name;
    if(styleName.isEmpty()) {
        styleName = defaultPrefix();
    }

    return styles.insert(style, styleName, insertionFlags());
}
