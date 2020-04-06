#ifndef KOALPHAMASKAPPLICATORFACTORY_H
#define KOALPHAMASKAPPLICATORFACTORY_H

#include "kritapigment_export.h"
#include <KoVcMultiArchBuildSupport.h>
#include <KoAlphaMaskApplicatorBase.h>

template<typename _channels_type_,
         int _channels_nb_,
         int _alpha_pos_>
class KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorFactory
{
public:
    typedef int ParamType;
    typedef KoAlphaMaskApplicatorBase* ReturnType;

    template<Vc::Implementation _impl>
    static KoAlphaMaskApplicatorBase* create(int);
};

#endif // KOALPHAMASKAPPLICATORFACTORY_H
