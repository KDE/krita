#ifndef _IMPORT_H_
#define _IMPORT_H_

#include <KoFilter.h>

class gifImport : public KoFilter {
    Q_OBJECT
    public:
        gifImport(QObject* parent, const QStringList&);
        virtual ~gifImport();
    public:
        virtual KoFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to);
};

#endif
