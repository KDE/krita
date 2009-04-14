#ifndef _SHIVAGENERATOR_H_
#define _SHIVAGENERATOR_H_

#include <kparts/plugin.h>
#include "generator/kis_generator.h"

class KisConfigWidget;

class ShivaPlugin : public KParts::Plugin {
public:
    ShivaPlugin(QObject *parent, const QStringList &);
    virtual ~ShivaPlugin();
};

namespace OpenShiva {
  class Source;
};

class ShivaGenerator : public KisGenerator
{
public:

    ShivaGenerator( OpenShiva::Source* kernel );

    using KisGenerator::generate;

    void generate(KisProcessingInformation dst,
                  const QSize& size,
                  const KisFilterConfiguration* config,
                  KoUpdater* progressUpdater
                 ) const;
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageSP image) const;

  private:
    OpenShiva::Source* m_source;
};

#endif
