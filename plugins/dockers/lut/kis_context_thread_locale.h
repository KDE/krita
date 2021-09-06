// SPDX-FileCopyrightText: 2013-2018 INRIA
// SPDX-License-Identifier: GPL-2.0-or-later

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <windows.h>
#endif // defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <locale.h>
#ifdef __APPLE__
#include <xlocale.h>
#endif

#ifndef KIS_CONTEXT_THREAD_LOCALE_H
#define KIS_CONTEXT_THREAD_LOCALE_H

// Helper class to set the C locale when doing OCIO calls.
//
// See https://github.com/AcademySoftwareFoundation/OpenColorIO/issues/297#issuecomment-505636123
//
// Source: https://github.com/NatronGitHub/openfx-io/commit/27a13e00db8bae1a31308d66b4039a3542c14805
class AutoSetAndRestoreThreadLocale
{
public:
    AutoSetAndRestoreThreadLocale()
    {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
        // set locale will only change locale on the current thread
        previousThreadConfig = _configthreadlocale(_ENABLE_PER_THREAD_LOCALE);

        // get and store current locale
        ssaLocale.assign(setlocale(LC_ALL, NULL));

        // set to "C" locale
        setlocale(LC_ALL, "C");
#else
        // set to C locale, saving the old one (returned from useLocale)
        currentLocale = newlocale(LC_ALL_MASK, "C", NULL);
        oldLocale = uselocale(currentLocale);
#endif
    }

    ~AutoSetAndRestoreThreadLocale()
    {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
        // thread specific
        setlocale(LC_ALL, ssaLocale.c_str());

        // set back to global settings]
        _configthreadlocale(previousThreadConfig);
#else
        // restore the previous locale and freeing the created locale
        uselocale(oldLocale);
        freelocale(currentLocale);
#endif
    }

private:
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    std::string ssaLocale;
    int previousThreadConfig;
#else
    locale_t oldLocale;
    locale_t currentLocale;
#endif
};

#endif // KIS_CONTEXT_THREAD_LOCALE_H
