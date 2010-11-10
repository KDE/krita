#ifndef KOSTYLE_H
#define KOSTYLE_H

#include "KoGenStyles.h"
#include "KoGenStyle.h"

#include <QString>
#include <QSharedPointer>

class KoStyle {
public:
    KoStyle();
    virtual ~KoStyle();

    QString saveOdf(KoGenStyles& styles) const;

    void setName(QString name);
    QString name() const;

protected:
    virtual void prepareStyle(KoGenStyle& style) const =0;
    virtual QString defaultPrefix() const =0;
    virtual KoGenStyle::Type styleType() const =0;
    virtual KoGenStyle::Type automaticstyleType() const =0;
    virtual const char* styleFamilyName() const =0;

private:
    KoGenStyles::InsertionFlags insertionFlags() const;

    bool m_autoStyle;
    QString m_name;
};

#define KOSTYLE_DECLARE_SHARED_POINTER(class) \
    typedef QSharedPointer<class> Ptr; \
    static Ptr create();

#define KOSTYLE_DECLARE_SHARED_POINTER_IMPL(class) \
    class::Ptr class::create() \
    { \
        return QSharedPointer<class>(new class); \
    }

#endif
