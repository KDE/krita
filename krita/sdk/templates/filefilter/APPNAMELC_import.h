#ifndef _IMPORT_H_
#define _IMPORT_H_

#include <QVariant>
#include <KoFilter.h>

class %{APPNAME}Import : public KoFilter {
    Q_OBJECT
    public:
        %{APPNAME}Import(QObject *parent, const QVariantList &);
        virtual ~%{APPNAME}Import();
    public:
        virtual KoFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to);
};

#endif
