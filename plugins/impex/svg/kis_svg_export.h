#ifndef KIS_SVG_EXPORT_H
#define KIS_SVG_EXPORT_H

#include <QVariant>

#include <KisImportExportFilter.h>

class KisSVGExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisSVGExport(QObject *parent, const QVariantList &);
    ~KisSVGExport() override;

    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration) override;

    void initializeCapabilities() override;

};

#endif
