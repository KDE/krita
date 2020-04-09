#ifndef KOALPHAMASKAPPLICATORFACTORY_H
#define KOALPHAMASKAPPLICATORFACTORY_H

#include "kritapigment_export.h"
#include <KoVcMultiArchBuildSupport.h>
#include <KoAlphaMaskApplicatorBase.h>
#include <half.h>

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


extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint8,  4, 3>::create<Vc::CurrentImplementation::current()>(int);
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint16, 4, 3>::create<Vc::CurrentImplementation::current()>(int);
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<half,    4, 3>::create<Vc::CurrentImplementation::current()>(int);
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<float,   4, 3>::create<Vc::CurrentImplementation::current()>(int);

extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint8,  5, 4>::create<Vc::CurrentImplementation::current()>(int);
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint16, 5, 4>::create<Vc::CurrentImplementation::current()>(int);
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<half,    5, 4>::create<Vc::CurrentImplementation::current()>(int);
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<float,   5, 4>::create<Vc::CurrentImplementation::current()>(int);

extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint8,  2, 1>::create<Vc::CurrentImplementation::current()>(int);
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint16, 2, 1>::create<Vc::CurrentImplementation::current()>(int);
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<half,    2, 1>::create<Vc::CurrentImplementation::current()>(int);
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<float,   2, 1>::create<Vc::CurrentImplementation::current()>(int);

extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint8,  1, 0>::create<Vc::CurrentImplementation::current()>(int);
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint16, 1, 0>::create<Vc::CurrentImplementation::current()>(int);
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<half,    1, 0>::create<Vc::CurrentImplementation::current()>(int);
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<float,   1, 0>::create<Vc::CurrentImplementation::current()>(int);


#endif // KOALPHAMASKAPPLICATORFACTORY_H
