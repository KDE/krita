/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2023 Ivan Santa María <ghevan@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KisMacosSystemProber_h
#define KisMacosSystemProber_h

#ifdef __cplusplus
extern "C" {
#endif

/**
 * returns true if there is another krita instance running
 * This method is reliant on NSWorkspace information
 *
 * Use this instead of QSharedMemory to ensure only one
 * instance of krita is ever launched, this works with or
 * without sandbox sessions
 */
bool iskritaRunningActivate(void);

#ifdef __cplusplus
}
#endif

#endif /* KisMacosSystemProber_h */
