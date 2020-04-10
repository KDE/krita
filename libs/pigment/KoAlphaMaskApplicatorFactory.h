#ifndef KOALPHAMASKAPPLICATORFACTORY_H
#define KOALPHAMASKAPPLICATORFACTORY_H

#include "kritapigment_export.h"

#include <KoID.h>
#include <KoAlphaMaskApplicatorBase.h>

class KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorFactory
{
public:
    static KoAlphaMaskApplicatorBase* create(KoID depthId, int numChannels, int alphaPos);
};

#endif // KOALPHAMASKAPPLICATORFACTORY_H
