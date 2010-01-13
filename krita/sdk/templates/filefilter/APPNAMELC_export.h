#ifndef _%{APPNAMEUC}_EXPORT_H_
#define _%{APPNAMEUC}_EXPORT_H_

#include <QVariant>
#include <KoFilter.h>

class %{APPNAME}Export : public KoFilter {
    Q_OBJECT
    public:
        %{APPNAME}Export(QObject *parent, const QVariantList &);
        virtual ~%{APPNAME}Export();
    public:
        virtual KoFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to);
};

#endif
