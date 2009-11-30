#ifndef _GIF_EXPORT_H_
#define _GIF_EXPORT_H_

#include <KoFilter.h>

class gifExport : public KoFilter {
    Q_OBJECT
    public:
        gifExport(QObject* parent, const QStringList&);
        virtual ~gifExport();
    public:
        virtual KoFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to);
};

#endif
