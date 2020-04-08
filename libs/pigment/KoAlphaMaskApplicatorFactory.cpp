#include "KoAlphaMaskApplicatorFactory.h"
#include "KoAlphaMaskApplicator.h"

template<typename _channels_type_,
         int _channels_nb_,
         int _alpha_pos_>
template<Vc::Implementation _impl>
KoAlphaMaskApplicatorBase*
KoAlphaMaskApplicatorFactory<_channels_type_, _channels_nb_, _alpha_pos_>::create(int)
{
    return new KoAlphaMaskApplicator<_channels_type_,
                                     _channels_nb_,
                                     _alpha_pos_,
                                     _impl>();
}

template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint8,  4, 3>::create<Vc::CurrentImplementation::current()>(int);
template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint16, 4, 3>::create<Vc::CurrentImplementation::current()>(int);
template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<half,    4, 3>::create<Vc::CurrentImplementation::current()>(int);
template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<float,   4, 3>::create<Vc::CurrentImplementation::current()>(int);

template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint8,  5, 4>::create<Vc::CurrentImplementation::current()>(int);
template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint16, 5, 4>::create<Vc::CurrentImplementation::current()>(int);
template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<half,    5, 4>::create<Vc::CurrentImplementation::current()>(int);
template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<float,   5, 4>::create<Vc::CurrentImplementation::current()>(int);

template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint8,  2, 1>::create<Vc::CurrentImplementation::current()>(int);
template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint16, 2, 1>::create<Vc::CurrentImplementation::current()>(int);
template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<half,    2, 1>::create<Vc::CurrentImplementation::current()>(int);
template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<float,   2, 1>::create<Vc::CurrentImplementation::current()>(int);

template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint8,  1, 0>::create<Vc::CurrentImplementation::current()>(int);
template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint16, 1, 0>::create<Vc::CurrentImplementation::current()>(int);
template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<half,    1, 0>::create<Vc::CurrentImplementation::current()>(int);
template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<float,   1, 0>::create<Vc::CurrentImplementation::current()>(int);
