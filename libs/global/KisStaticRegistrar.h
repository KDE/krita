/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSTATICREGISTRAR_H
#define KISSTATICREGISTRAR_H

#ifndef CONCAT
#define CONCAT(a, b) CONCAT_IMPL(a, b)
#define CONCAT_IMPL(a, b) a ## b
#endif

#define KIS_DECLARE_STATIC_REGISTRAR_IMPL(uniqueId)  \
    static void CONCAT(registrationFunc, uniqueId)(); \
\
    class CONCAT(RegistrarStruct, uniqueId) \
    { \
        public: \
        CONCAT(RegistrarStruct, uniqueId)(void (*initializer)()) { \
            initializer(); \
        } \
    }; \
\
    static CONCAT(RegistrarStruct, uniqueId) CONCAT(__registrarVariable, uniqueId)(&CONCAT(registrationFunc, uniqueId)); \
\
    void CONCAT(registrationFunc, uniqueId)()

/**
 * Sometimes we need to declare a static object that performs some actions on Krita
 * loading, e.g. to register Qt's metatype for a Krita type. We call this pattern as
 * "registrar" and this macro helps with that:
 *
 * KIS_DECLARE_STATIC_REGISTRAR {
 *     qRegisterMetaType<KoResourceSP>("KoResourceSP");
 * }
 */

#define KIS_DECLARE_STATIC_REGISTRAR KIS_DECLARE_STATIC_REGISTRAR_IMPL(__COUNTER__)


#endif // KISSTATICREGISTRAR_H
