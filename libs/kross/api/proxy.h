/***************************************************************************
 * proxy.h
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_API_PROXY_H
#define KROSS_API_PROXY_H

#include "../main/krossconfig.h"
#include "object.h"
#include "list.h"

#include <qstring.h>

namespace Kross { namespace Api {

    /**
     * The ProxyValue template-class is used to represent a single
     * value (returnvalue or argument) of a \a ProxyFunction .
     */
    template< class OBJECT, typename TYPE >
    class ProxyValue
    {
        public:

            /**
             * The C/C++ type of the value. This could be something
             * like a int, uint or a const QString&.
             */
            typedef TYPE type;

            /**
             * The \a Kross::Api::Object or a from it inherited class
             * that we should use internaly to wrap the C/C++ type. This
             * could be e.g. a \a Kross::Api::Variant to wrap an uint.
             */
            typedef OBJECT object;
    };

    /**
     * The ProxyFunction template-class is used to publish any C/C++
     * method (not only slots) of a struct or class instance as a
     * a \a Kross::Api::Function to Kross.
     *
     * With this class we don't need to have a method-wrapper for
     * each single function what a) should reduce the code needed for
     * wrappers and b) is more typesafe cause the connection to the
     * C/C++ method is done on compiletime.
     */
    template< class INSTANCE, // the objectinstance
              typename METHOD, // the method-signature
              class RET  = ProxyValue<Kross::Api::Object,void>, // returnvalue
              class ARG1 = ProxyValue<Kross::Api::Object,void>, // first argument
              class ARG2 = ProxyValue<Kross::Api::Object,void>, // second argument
              class ARG3 = ProxyValue<Kross::Api::Object,void>, // forth argument
              class ARG4 = ProxyValue<Kross::Api::Object,void> > // fifth argument
    class ProxyFunction : public Function
    {
        private:
            /// Pointer to the objectinstance which method should be called.
            INSTANCE* m_instance;
            /// Pointer to the method which should be called.
            const METHOD m_method;

            /**
             * Internal used struct that does the execution of the wrapped method.
             */
            template<class PROXYFUNC, typename RETURNRYPE>
            struct ProxyFunctionCaller {
                inline static Object::Ptr exec(PROXYFUNC* self, typename ARG1::type arg1, typename ARG2::type arg2, typename ARG3::type arg3, typename ARG4::type arg4) {
                    return new typename RET::object( ( (self->m_instance)->*(self->m_method) )(arg1,arg2,arg3,arg4) );
                }
            };

            /**
             * Template-specialization of the \a ProxyFunctionCaller above which
             * handles void-returnvalues. We need to handle this special case
             * seperatly cause compilers deny to return void.
             */
            template<class PROXYFUNC>
            struct ProxyFunctionCaller<PROXYFUNC, void> {
                inline static Object::Ptr exec(PROXYFUNC* self, typename ARG1::type arg1, typename ARG2::type arg2, typename ARG3::type arg3, typename ARG4::type arg4) {
                    ( (self->m_instance)->*(self->m_method) )(arg1,arg2,arg3,arg4);
                    return 0;
                }
            };

        public:

            /**
             * Constructor.
             *
             * \param instance The objectinstance to which the \p method
             *        belongs to.
             * \param method The method-pointer.
             */
            ProxyFunction(INSTANCE* instance, const METHOD& method)
                : m_instance(instance), m_method(method) {}

            /**
             * This method got called if the wrapped method should be executed.
             * 
             * \param args The optional list of arguments passed to the
             *        execution-call.
             * \return The returnvalue of the functioncall. It will be NULL if
             *        the functioncall doesn't provide us a returnvalue (e.g.
             *        if the function has void as returnvalue).
             */
            Object::Ptr call(List::Ptr args) {
                return ProxyFunctionCaller<ProxyFunction, typename RET::type>::exec(this,
                    Kross::Api::Object::fromObject<typename ARG1::object>(args->item(0))->operator typename ARG1::type(),
                    Kross::Api::Object::fromObject<typename ARG2::object>(args->item(1))->operator typename ARG2::type(),
                    Kross::Api::Object::fromObject<typename ARG3::object>(args->item(2))->operator typename ARG3::type(),
                    Kross::Api::Object::fromObject<typename ARG4::object>(args->item(3))->operator typename ARG4::type()
                );
            }

            template<class PROXYFUNC, typename RETURNRYPE>
            friend struct ProxyFunctionCaller;
    };

    /**
     * Template-specialization of the \a ProxyFunction above with 3 arguments.
     */
    template<class INSTANCE, typename METHOD, class RET, class ARG1, class ARG2, class ARG3>
    class ProxyFunction<INSTANCE, METHOD, RET, ARG1, ARG2, ARG3 > : public Function
    {
        private:
            INSTANCE* m_instance;
            const METHOD m_method;

            template<class PROXYFUNC, typename RETURNRYPE>
            struct ProxyFunctionCaller {
                inline static Object::Ptr exec(PROXYFUNC* self, typename ARG1::type arg1, typename ARG2::type arg2, typename ARG3::type arg3) {
                    return new typename RET::object( ( (self->m_instance)->*(self->m_method) )(arg1,arg2,arg3) );
                }
            };

            template<class PROXYFUNC>
            struct ProxyFunctionCaller<PROXYFUNC, void> {
                inline static Object::Ptr exec(PROXYFUNC* self, typename ARG1::type arg1, typename ARG2::type arg2, typename ARG3::type arg3) {
                    ( (self->m_instance)->*(self->m_method) )(arg1,arg2,arg3);
                    return 0;
                }
            };

        public:
            ProxyFunction(INSTANCE* instance, const METHOD& method)
                : m_instance(instance), m_method(method) {}
            Object::Ptr call(List::Ptr args) {
                return ProxyFunctionCaller<ProxyFunction, typename RET::type>::exec(this,
                    Kross::Api::Object::fromObject<typename ARG1::object>(args->item(0))->operator typename ARG1::type(),
                    Kross::Api::Object::fromObject<typename ARG2::object>(args->item(1))->operator typename ARG2::type(),
                    Kross::Api::Object::fromObject<typename ARG3::object>(args->item(2))->operator typename ARG3::type()
                );
            }

            template<class PROXYFUNC, typename RETURNRYPE>
            friend struct ProxyFunctionCaller;
    };

    /**
     * Template-specialization of the \a ProxyFunction above with 2 arguments.
     */
    template<class INSTANCE, typename METHOD, class RET, class ARG1, class ARG2>
    class ProxyFunction<INSTANCE, METHOD, RET, ARG1, ARG2 > : public Function
    {
        private:
            INSTANCE* m_instance;
            const METHOD m_method;

            template<class PROXYFUNC, typename RETURNRYPE>
            struct ProxyFunctionCaller {
                inline static Object::Ptr exec(PROXYFUNC* self, typename ARG1::type arg1, typename ARG2::type arg2) {
                    return new typename RET::object( ( (self->m_instance)->*(self->m_method) )(arg1,arg2) );
                }
            };

            template<class PROXYFUNC>
            struct ProxyFunctionCaller<PROXYFUNC, void> {
                inline static Object::Ptr exec(PROXYFUNC* self, typename ARG1::type arg1, typename ARG2::type arg2) {
                    ( (self->m_instance)->*(self->m_method) )(arg1,arg2);
                    return 0;
                }
            };

        public:
            ProxyFunction(INSTANCE* instance, const METHOD& method)
                : m_instance(instance), m_method(method) {}
            Object::Ptr call(List::Ptr args) {
                return ProxyFunctionCaller<ProxyFunction, typename RET::type>::exec(this,
                    Kross::Api::Object::fromObject<typename ARG1::object>(args->item(0))->operator typename ARG1::type(),
                    Kross::Api::Object::fromObject<typename ARG2::object>(args->item(1))->operator typename ARG2::type()
                );
            }

            template<class PROXYFUNC, typename RETURNRYPE>
            friend struct ProxyFunctionCaller;
    };

    /**
     * Template-specialization of the \a ProxyFunction above with one argument.
     */
    template<class INSTANCE, typename METHOD, class RET, class ARG1>
    class ProxyFunction<INSTANCE, METHOD, RET, ARG1 > : public Function
    {
        private:
            INSTANCE* m_instance;
            const METHOD m_method;

            template<class PROXYFUNC, typename RETURNRYPE>
            struct ProxyFunctionCaller {
                inline static Object::Ptr exec(PROXYFUNC* self, typename ARG1::type arg1) {
                    return new typename RET::object( ( (self->m_instance)->*(self->m_method) )(arg1) );
                }
            };

            template<class PROXYFUNC>
            struct ProxyFunctionCaller<PROXYFUNC, void> {
                inline static Object::Ptr exec(PROXYFUNC* self, typename ARG1::type arg1) {
                    ( (self->m_instance)->*(self->m_method) )(arg1);
                    return 0;
                }
            };

        public:
            ProxyFunction(INSTANCE* instance, const METHOD& method)
                : m_instance(instance), m_method(method) {}
            Object::Ptr call(List::Ptr args) {
                return ProxyFunctionCaller<ProxyFunction, typename RET::type>::exec(this,
                    Kross::Api::Object::fromObject<typename ARG1::object>(args->item(0))->operator typename ARG1::type()
                );
            }

            template<class PROXYFUNC, typename RETURNRYPE>
	    friend struct ProxyFunctionCaller;
    };

    /**
     * Template-specialization of the \a ProxyFunction above with no arguments.
     */
    template<class INSTANCE, typename METHOD, class RET>
    class ProxyFunction<INSTANCE, METHOD, RET > : public Function
    {
        private:
            INSTANCE* m_instance;
            const METHOD m_method;

            template<class PROXYFUNC, typename RETURNRYPE>
            struct ProxyFunctionCaller {
                inline static Object::Ptr exec(PROXYFUNC* self) {
                    return new typename RET::object( ( (self->m_instance)->*(self->m_method) )() );
                }
            };

            template<class PROXYFUNC>
            struct ProxyFunctionCaller<PROXYFUNC, void> {
                inline static Object::Ptr exec(PROXYFUNC* self) {
                    ( (self->m_instance)->*(self->m_method) )();
                    return 0;
                }
            };

        public:
            ProxyFunction(INSTANCE* instance, const METHOD& method)
                : m_instance(instance), m_method(method) {}
            Object::Ptr call(List::Ptr) {
                return ProxyFunctionCaller<ProxyFunction, typename RET::type>::exec(this);
            }

            template<class PROXYFUNC, typename RETURNRYPE>
            friend struct ProxyFunctionCaller;
    };

}}

#endif

