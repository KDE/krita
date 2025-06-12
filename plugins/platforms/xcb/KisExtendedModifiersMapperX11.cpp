/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisExtendedModifiersMapperX11.h"

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QX11Info>
#else
#include <QGuiApplication>
#endif

#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <krita_container_utils.h>

struct KeyMapping {
    KeySym x11KeySym {0};
    Qt::Key qtKey {Qt::Key_unknown};
};

static const KeyMapping s_mapping[] = {
    {XK_Shift_L, Qt::Key_Shift},
    {XK_Shift_R, Qt::Key_Shift},

    {XK_Control_L, Qt::Key_Control},
    {XK_Control_R, Qt::Key_Control},

    {XK_Meta_L, Qt::Key_Alt},
    {XK_Meta_R, Qt::Key_Alt},
    {XK_Mode_switch, Qt::Key_AltGr},
    {XK_ISO_Level3_Shift, Qt::Key_AltGr},

    {XK_Alt_L, Qt::Key_Alt},
    {XK_Alt_R, Qt::Key_Alt},

    {XK_Super_L, Qt::Key_Meta},
    {XK_Super_R, Qt::Key_Meta},

    {XK_Hyper_L, Qt::Key_Hyper_L},
    {XK_Hyper_R, Qt::Key_Hyper_R},

    {XK_space, Qt::Key_Space},

    {XK_0, Qt::Key_0},
    {XK_1, Qt::Key_1},
    {XK_2, Qt::Key_2},
    {XK_3, Qt::Key_3},
    {XK_4, Qt::Key_4},
    {XK_5, Qt::Key_5},
    {XK_6, Qt::Key_6},
    {XK_7, Qt::Key_7},
    {XK_8, Qt::Key_8},
    {XK_9, Qt::Key_9},

    {XK_a, Qt::Key_A},
    {XK_b, Qt::Key_B},
    {XK_c, Qt::Key_C},
    {XK_d, Qt::Key_D},
    {XK_e, Qt::Key_E},
    {XK_f, Qt::Key_F},
    {XK_g, Qt::Key_G},
    {XK_h, Qt::Key_H},
    {XK_i, Qt::Key_I},
    {XK_j, Qt::Key_J},
    {XK_k, Qt::Key_K},
    {XK_l, Qt::Key_L},
    {XK_m, Qt::Key_M},
    {XK_n, Qt::Key_N},
    {XK_o, Qt::Key_O},
    {XK_p, Qt::Key_P},
    {XK_q, Qt::Key_Q},
    {XK_r, Qt::Key_R},
    {XK_s, Qt::Key_S},
    {XK_t, Qt::Key_T},
    {XK_u, Qt::Key_U},
    {XK_v, Qt::Key_V},
    {XK_w, Qt::Key_W},
    {XK_x, Qt::Key_X},
    {XK_y, Qt::Key_Y},
    {XK_z, Qt::Key_Z},

    {XK_A, Qt::Key_A},
    {XK_B, Qt::Key_B},
    {XK_C, Qt::Key_C},
    {XK_D, Qt::Key_D},
    {XK_E, Qt::Key_E},
    {XK_F, Qt::Key_F},
    {XK_G, Qt::Key_G},
    {XK_H, Qt::Key_H},
    {XK_I, Qt::Key_I},
    {XK_J, Qt::Key_J},
    {XK_K, Qt::Key_K},
    {XK_L, Qt::Key_L},
    {XK_M, Qt::Key_M},
    {XK_N, Qt::Key_N},
    {XK_O, Qt::Key_O},
    {XK_P, Qt::Key_P},
    {XK_Q, Qt::Key_Q},
    {XK_R, Qt::Key_R},
    {XK_S, Qt::Key_S},
    {XK_T, Qt::Key_T},
    {XK_U, Qt::Key_U},
    {XK_V, Qt::Key_V},
    {XK_W, Qt::Key_W},
    {XK_X, Qt::Key_X},
    {XK_Y, Qt::Key_Y},
    {XK_Z, Qt::Key_Z}
};

KisExtendedModifiersMapperX11::KisExtendedModifiersMapperX11(QObject *parent, const QVariantList &)
    : KisExtendedModifiersMapperPluginInterface(parent)
{
}

KisExtendedModifiersMapperX11::~KisExtendedModifiersMapperX11() = default;

KisExtendedModifiersMapperPluginInterface::ExtendedModifiers KisExtendedModifiersMapperX11::queryExtendedModifiers()
{
    KisExtendedModifiersMapperPluginInterface::ExtendedModifiers modifiers;

    char keysState[32];
    int minKeyCode = 0;
    int maxKeyCode = 0;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    XDisplayKeycodes(QX11Info::display(), &minKeyCode, &maxKeyCode);
    XQueryKeymap(QX11Info::display(), keysState);
#else
    XDisplayKeycodes(qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->display(),
                     &minKeyCode,
                     &maxKeyCode);
    XQueryKeymap(qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->display(), keysState);
#endif


    auto checkKeyCodePressedX11 = [&keysState] (KeyCode key) -> bool {
        int byte = key / 8;
        char mask = 1 << (key % 8);
        return keysState[byte] & mask;
    };

    for (int keyCode = minKeyCode; keyCode <= maxKeyCode; keyCode++) {
        if (checkKeyCodePressedX11(keyCode)) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            KeySym sym = XkbKeycodeToKeysym(QX11Info::display(),
                                            keyCode,
#else
            KeySym sym = XkbKeycodeToKeysym(qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->display(),
                                            keyCode,
#endif
                                            0,
                                            0);
            for (size_t i = 0; i < sizeof(s_mapping) / sizeof(s_mapping[0]); i++) {
                const KeyMapping &map = s_mapping[i];
                if (map.x11KeySym == sym) {
                    modifiers << map.qtKey;
                    break;
                }
            }
        }
    }

    // in X11 some keys may have multiple keysyms,
    // (Alt Key == XK_Meta_{L,R}, XK_Meta_{L,R})
    KritaUtils::makeContainerUnique(modifiers);
    return modifiers;
}

