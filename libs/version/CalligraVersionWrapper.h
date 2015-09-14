#ifndef CALLIGRAVERSIONWRAPPER_H
#define CALLIGRAVERSIONWRAPPER_H

#include "koversion_export.h"
#include <QString>

namespace CalligraVersionWrapper {

    KOVERSION_EXPORT QString versionYear();
    KOVERSION_EXPORT QString versionString(bool checkGit = false);
}

#endif // CALLIGRAVERSIONWRAPPER_H
