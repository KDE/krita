#ifndef _%{APPNAMEUC}_H_
#define _%{APPNAMEUC}_H_

#include <kparts/plugin.h>
#include "kis_filter.h"

class %{APPNAME}Plugin : public KParts::Plugin
{
public:
    %{APPNAME}Plugin(QObject *parent, const QStringList &);
    virtual ~%{APPNAME}Plugin();
};

class %{APPNAME}Filter : public KisFilter
{
public:
    %{APPNAME}Filter();
public:
    virtual void process(const KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint& dstTopLeft, const QSize& size, KisFilterConfiguration* config);
    virtual ColorSpaceIndependence colorSpaceIndependence() { return FULLY_INDEPENDENT; }
    static inline KoID id() { return KoID("%{APPNAMELC}", i18n("%{APPNAME}")); }
    virtual bool supportsPainting() { return true; }
    virtual bool supportsPreview() { return true; }
    virtual bool supportsIncrementalPainting() { return false; }
};

#endif
