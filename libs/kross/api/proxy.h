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

#include "krossconfig.h"
#include "object.h"
#include "variant.h"
#include "list.h"

#include <QString>

namespace Kross { namespace Api {

    /**
     * \internal used struct to translate an argument-value dynamicly.
     */
    template<class OBJ>
    struct ProxyArgTranslator {
        OBJ* m_object;
        ProxyArgTranslator(Kross::Api::Object* obj) {
            m_object = Kross::Api::Object::fromObject<OBJ>(obj);
        }
        template<typename T>
        inline operator T () {
            return m_object->operator T();
        }
    };

    /**
     * \internal used struct to translate a return-value dynamicly.
     */
    struct ProxyRetTranslator {
        template<class RETURNOBJ, typename TYPE>
        inline static Object::Ptr cast(TYPE t) {
            return RETURNOBJ::toObject(t);
        }
    };

    /**
     * The ProxyFunction template-class is used to publish any C/C++
     * method (not only slots) of a struct or class instance as a
     * a \a Function to Kross.
     *
     * With this class we don't need to have a method-wrapper for
     * each single function what a) should reduce the code needed for
     * wrappers and b) is more typesafe cause the connection to the
     * C/C++ method is done on compiletime.
     *
     * Example how a ProxyFunction may got used;
     * @code
     * #include "../api/class.h"
     * #include "../api/proxy.h"
     * // The class which should be published.
     * class MyClass : public Kross::Api::Class<MyClass> {
     *     public:
     *         MyClass(const QString& name) : Kross::Api::Class<MyClass>(name) {
     *             // publish the function myfunc, so that scripting-code is able
     *             // to call that method.
     *             this->addProxyFunction <
     *                 Kross::Api::Variant, // the uint returnvalue is handled with Variant.
     *                 Kross::Api::Variant, // the QString argument is handled with Variant too.
     *                 MyClass // the MyClass* is handled implicit by our class.
     *             > ( "myfuncname", // the name the function should be published as.
     *                 this, // pointer to the class-instance which has the method.
     *                 &TestPluginObject::myfunc ); // pointer to the method itself.
     *         }
     *         virtual ~MyClass() {}
     *         virtual const QString getClassName() const { return "MyClass"; }
     *     private:
     *         uint myfunc(const QCString&, MyClass* myotherclass) {
     *             // This method will be published to the scripting backend. So, scripting
     *             // code is able to call this method.
     *         }
     * }
     * @endcode
     */
    template< class INSTANCE, // the objectinstance
              typename METHOD, // the method-signature
              class RETURNOBJ,// = Kross::Api::Object, // return-value
              class ARG1OBJ = Kross::Api::Object, // first parameter-value
              class ARG2OBJ = Kross::Api::Object, // second parameter-value
              class ARG3OBJ = Kross::Api::Object, // theird parameter-value
              class ARG4OBJ = Kross::Api::Object // forth parameter-value
    >
    class ProxyFunction : public Function
    {
            template<class PROXYFUNC, typename RETURNTYPE>
            friend struct ProxyFunctionCaller;
        private:
            /// Pointer to the objectinstance which method should be called.
            INSTANCE* m_instance;
            /// Pointer to the method which should be called.
            const METHOD m_method;

            /// First default argument.
            KSharedPtr<ARG1OBJ> m_defarg1;
            /// Second default argument.
            KSharedPtr<ARG2OBJ> m_defarg2;
            /// Theird default argument.
            KSharedPtr<ARG3OBJ> m_defarg3;
            /// Forth default argument.
            KSharedPtr<ARG4OBJ> m_defarg4;

            /**
             * \internal used struct that does the execution of the wrapped
             * method.
             */
            template<class PROXYFUNC, typename RETURNTYPE>
            struct ProxyFunctionCaller {
                inline static Object::Ptr exec(PROXYFUNC* self, Kross::Api::Object* arg1, Kross::Api::Object* arg2, Kross::Api::Object* arg3, Kross::Api::Object* arg4) {
                    return ProxyRetTranslator::cast<RETURNTYPE>(
                        ( (self->m_instance)->*(self->m_method) )( ProxyArgTranslator<ARG1OBJ>(arg1), ProxyArgTranslator<ARG2OBJ>(arg2), ProxyArgTranslator<ARG3OBJ>(arg3), ProxyArgTranslator<ARG4OBJ>(arg4) )
                    );
                }
            };

            /**
             * \internal template-specialization of the \a ProxyFunctionCaller
             * above which handles void-returnvalues. We need to handle this
             * special case seperatly cause compilers deny to return void :-/
             */
            template<class PROXYFUNC>
            struct ProxyFunctionCaller<PROXYFUNC, void> {
                inline static Object::Ptr exec(PROXYFUNC* self, Kross::Api::Object* arg1, Kross::Api::Object* arg2, Kross::Api::Object* arg3, Kross::Api::Object* arg4) {
                    ( (self->m_instance)->*(self->m_method) )( ProxyArgTranslator<ARG1OBJ>(arg1), ProxyArgTranslator<ARG1OBJ>(arg2), ProxyArgTranslator<ARG3OBJ>(arg3), ProxyArgTranslator<ARG4OBJ>(arg4) );
                    return Object::Ptr(0); // void return-value
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
            ProxyFunction(INSTANCE* instance, const METHOD& method, ARG1OBJ* defarg1 = 0, ARG2OBJ* defarg2 = 0, ARG3OBJ* defarg3 = 0, ARG4OBJ* defarg4 = 0)
                : m_instance(instance), m_method(method), m_defarg1(defarg1), m_defarg2(defarg2), m_defarg3(defarg3), m_defarg4(defarg4) {}

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
                return ProxyFunctionCaller<ProxyFunction, RETURNOBJ>::exec(this,
                    args->item(0, m_defarg1.data()),
                    args->item(1, m_defarg2.data()),
                    args->item(2, m_defarg3.data()),
                    args->item(3, m_defarg4.data())
                );
            }
    };

    /**
     * Template-specialization of the \a ProxyFunction above with three arguments.
     */
    template<class INSTANCE, typename METHOD, class RETURNOBJ, class ARG1OBJ, class ARG2OBJ, class ARG3OBJ>
    class ProxyFunction<INSTANCE, METHOD, RETURNOBJ, ARG1OBJ, ARG2OBJ, ARG3OBJ> : public Function
    {
            template<class PROXYFUNC, typename RETURNTYPE>
            friend struct ProxyFunctionCaller;
        private:
            INSTANCE* m_instance;
            const METHOD m_method;
            KSharedPtr<ARG1OBJ> m_defarg1;
            KSharedPtr<ARG2OBJ> m_defarg2;
            KSharedPtr<ARG3OBJ> m_defarg3;

            template<class PROXYFUNC, typename RETURNTYPE>
            struct ProxyFunctionCaller {
                inline static Object::Ptr exec(PROXYFUNC* self, Kross::Api::Object* arg1, Kross::Api::Object* arg2, Kross::Api::Object* arg3) {
                    return ProxyRetTranslator::cast<RETURNTYPE>(
                        ( (self->m_instance)->*(self->m_method) )( ProxyArgTranslator<ARG1OBJ>(arg1), ProxyArgTranslator<ARG2OBJ>(arg2), ProxyArgTranslator<ARG3OBJ>(arg3) )
                    );
                }
            };

            template<class PROXYFUNC>
            struct ProxyFunctionCaller<PROXYFUNC, void> {
                inline static Object::Ptr exec(PROXYFUNC* self, Kross::Api::Object* arg1, Kross::Api::Object* arg2, Kross::Api::Object* arg3) {
                    ( (self->m_instance)->*(self->m_method) )( ProxyArgTranslator<ARG1OBJ>(arg1), ProxyArgTranslator<ARG2OBJ>(arg2), ProxyArgTranslator<ARG3OBJ>(arg3) );
                    return Object::Ptr(0);
                }
            };

        public:
            ProxyFunction(INSTANCE* instance, const METHOD& method, ARG1OBJ* defarg1 = 0, ARG2OBJ* defarg2 = 0, ARG3OBJ* defarg3 = 0)
                : m_instance(instance), m_method(method), m_defarg1(defarg1), m_defarg2(defarg2), m_defarg3(defarg3) {}
            Object::Ptr call(List::Ptr args) {
                return ProxyFunctionCaller<ProxyFunction, RETURNOBJ>::exec(this,
                    args->item(0, m_defarg1.data()),
                    args->item(1, m_defarg2.data()),
                    args->item(2, m_defarg3.data())
                );
            }
    };

    /**
     * Template-specialization of the \a ProxyFunction above with two arguments.
     */
    template<class INSTANCE, typename METHOD, class RETURNOBJ, class ARG1OBJ, class ARG2OBJ>
    class ProxyFunction<INSTANCE, METHOD, RETURNOBJ, ARG1OBJ, ARG2OBJ> : public Function
    {
            template<class PROXYFUNC, typename RETURNTYPE>
            friend struct ProxyFunctionCaller;
        private:
            INSTANCE* m_instance;
            const METHOD m_method;
            KSharedPtr<ARG1OBJ> m_defarg1;
            KSharedPtr<ARG2OBJ> m_defarg2;

            template<class PROXYFUNC, typename RETURNTYPE>
            struct ProxyFunctionCaller {
                inline static Object::Ptr exec(PROXYFUNC* self, Kross::Api::Object* arg1, Kross::Api::Object* arg2) {
                    return ProxyRetTranslator::cast<RETURNTYPE>(
                        ( (self->m_instance)->*(self->m_method) )( ProxyArgTranslator<ARG1OBJ>(arg1), ProxyArgTranslator<ARG2OBJ>(arg2) )
                    );
                }
            };

            template<class PROXYFUNC>
            struct ProxyFunctionCaller<PROXYFUNC, void> {
                inline static Object::Ptr exec(PROXYFUNC* self, Kross::Api::Object* arg1, Kross::Api::Object* arg2) {
                    ( (self->m_instance)->*(self->m_method) )( ProxyArgTranslator<ARG1OBJ>(arg1), ProxyArgTranslator<ARG2OBJ>(arg2) );
                    return Object::Ptr(0);
                }
            };

        public:
            ProxyFunction(INSTANCE* instance, const METHOD& method, ARG1OBJ* defarg1 = 0, ARG2OBJ* defarg2 = 0)
                : m_instance(instance), m_method(method), m_defarg1(defarg1), m_defarg2(defarg2) {}
            Object::Ptr call(List::Ptr args) {
                return ProxyFunctionCaller<ProxyFunction, RETURNOBJ>::exec(this,
                    args->item(0, m_defarg1.data()),
                    args->item(1, m_defarg2.data())
                );
            }
    };

    /**
     * Template-specialization of the \a ProxyFunction above with one argument.
     */
    template<class INSTANCE, typename METHOD, class RETURNOBJ, class ARG1OBJ>
    class ProxyFunction<INSTANCE, METHOD, RETURNOBJ, ARG1OBJ> : public Function
    {
            template<class PROXYFUNC, typename RETURNTYPE>
            friend struct ProxyFunctionCaller;
        private:
            INSTANCE* m_instance;
            const METHOD m_method;
            KSharedPtr<ARG1OBJ> m_defarg1;

            template<class PROXYFUNC, typename RETURNTYPE>
            struct ProxyFunctionCaller {
                inline static Object::Ptr exec(PROXYFUNC* self, Kross::Api::Object* arg1) {
                    return ProxyRetTranslator::cast<RETURNTYPE>(
                        ( (self->m_instance)->*(self->m_method) )( ProxyArgTranslator<ARG1OBJ>(arg1) )
                    );
                }
            };

            template<class PROXYFUNC>
            struct ProxyFunctionCaller<PROXYFUNC, void> {
                inline static Object::Ptr exec(PROXYFUNC* self, Kross::Api::Object* arg1) {
                    ( (self->m_instance)->*(self->m_method) )( ProxyArgTranslator<ARG1OBJ>(arg1) );
                    return Object::Ptr(0);
                }
            };

        public:
            ProxyFunction(INSTANCE* instance, const METHOD& method, ARG1OBJ* defarg1 = 0)
                : m_instance(instance), m_method(method), m_defarg1(defarg1) {}
            Object::Ptr call(List::Ptr args) {
                return ProxyFunctionCaller<ProxyFunction, RETURNOBJ>::exec(this,
                    args->item(0, m_defarg1.data())
                );
            }
    };

    /**
     * Template-specialization of the \a ProxyFunction above with no arguments.
     */
    template<class INSTANCE, typename METHOD, class RETURNOBJ>
    class ProxyFunction<INSTANCE, METHOD, RETURNOBJ> : public Function
    {
            template<class PROXYFUNC, typename RETURNTYPE>
            friend struct ProxyFunctionCaller;
        private:
            INSTANCE* m_instance;
            const METHOD m_method;

            template<class PROXYFUNC, typename RETURNTYPE>
            struct ProxyFunctionCaller {
                inline static Object::Ptr exec(PROXYFUNC* self) {
                    return ProxyRetTranslator::cast<RETURNTYPE>(
                        ( (self->m_instance)->*(self->m_method) )()
                    );
                }
            };

            template<class PROXYFUNC>
            struct ProxyFunctionCaller<PROXYFUNC, void> {
                inline static Object::Ptr exec(PROXYFUNC* self) {
                    ( (self->m_instance)->*(self->m_method) )();
                    return Object::Ptr(0);
                }
            };

        public:
            ProxyFunction(INSTANCE* instance, const METHOD& method)
                : m_instance(instance), m_method(method) {}
            Object::Ptr call(List::Ptr) {
                return ProxyFunctionCaller<ProxyFunction, RETURNOBJ>::exec(this);
            }
    };

}}

#endif

