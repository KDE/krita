/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_UI_TYPES_H_
#define KIS_UI_TYPES_H_

template<class T>
class KisWeakSharedPtr;
template<class T>
class KisSharedPtr;

class KisPrescaledProjection;
typedef KisSharedPtr<KisPrescaledProjection> KisPrescaledProjectionSP;

class KisUpdateInfo;
class KisPPUpdateInfo;
typedef KisSharedPtr<KisUpdateInfo> KisUpdateInfoSP;
typedef KisSharedPtr<KisPPUpdateInfo> KisPPUpdateInfoSP;

#endif
