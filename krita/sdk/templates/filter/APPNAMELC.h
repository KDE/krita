#ifndef _%{APPNAMEUC}_H_
#define _%{APPNAMEUC}_H_

#include <QObject>
#include <QVariant>
#include "filter/kis_filter.h"

class %{APPNAME}Plugin : public QObject
{
public:
    %{APPNAME}Plugin(QObject *parent, const QVariantList &);
    virtual ~%{APPNAME}Plugin();
};

class %{APPNAME}Filter : public KisFilter
{
public:
    %{APPNAME}Filter();
public:
    virtual void process(const KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint& dstTopLeft, const QSize& size, KisFilterConfiguration* config);
    virtual ColorSpaceIndependence colorSpaceIndependence() const { return FULLY_INDEPENDENT; }
    static inline KoID id() { return KoID("%{APPNAMELC}", i18n("%{APPNAME}")); }
    virtual bool supportsPainting() const { return true; }
    virtual bool supportsPreview() const { return true; }
    virtual bool supportsIncrementalPainting() const { return false; }
};

#endif
