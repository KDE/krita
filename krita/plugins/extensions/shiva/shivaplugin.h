#ifndef _SHIVAPLUGIN_H_
#define _SHIVAPLUGIN_H_

#include <kparts/plugin.h>
#include "generator/kis_generator.h"

class KisConfigWidget;

class ShivaPlugin : public KParts::Plugin {
public:
    ShivaPlugin(QObject *parent, const QStringList &);
    virtual ~ShivaPlugin();
};

#endif
