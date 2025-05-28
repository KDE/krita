/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWaylandKeyboardWatcher.h"

#include <kis_debug.h>

#include <QWaylandClientExtension>
#include <qwayland-wayland.h>

#include <config-qt-wayland-patches-present.h>
#include <QtGui/private/qxkbcommon_p.h>

#include <sys/mman.h>
#include <unistd.h>


#if KRITA_QT_HAS_XKB_CONTEXT_IN_NATIVE_INTERFACE

#include <QGuiApplication>
#include <QtWaylandClient/private/qwaylandintegration_p.h>

#else

#include <QtGui/private/qguiapplication_p.h>
#include <QtWaylandClient/private/qwaylandintegration_p.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>

#endif /* KRITA_QT_HAS_XKB_CONTEXT_IN_NATIVE_INTERFACE */

static struct xkb_context *getGlobalXkbContextFromQt() {
#if KRITA_QT_HAS_XKB_CONTEXT_IN_NATIVE_INTERFACE
    auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    return waylandApp ? waylandApp->xkbContext() : nullptr;
#else
    /**
     * Linking to QPlatformIntegration has no binary compatibility guarantees, so we should
     * prefer the usage of a direct published interface that is more stable.
     */
    QPlatformIntegration *platformIntegration = QGuiApplicationPrivate::platformIntegration();
    auto waylandIntegration = dynamic_cast<QtWaylandClient::QWaylandIntegration *>(platformIntegration);
    return waylandIntegration ? waylandIntegration->display()->xkbContext() : nullptr;
#endif /* KRITA_QT_HAS_XKB_CONTEXT_IN_NATIVE_INTERFACE */
}

/****************************************************************************/
/*              KisWaylandKeyboardWatcher::Watcher::Keyboard                         */
/****************************************************************************/

class KisWaylandKeyboardWatcher::Keyboard : public QtWayland::wl_keyboard
{
public:
    Keyboard(::wl_keyboard *keyboard)
        : wl_keyboard(keyboard)
    {
    }

    ~Keyboard()
    {
        release();
    }

    QList<Qt::Key> pressedKeys() const
    {
        QList<Qt::Key> keys;

        if (mXkbState) {
            for (uint32_t key : std::as_const(mNativeKeys)) {
                auto code = key + 8; // map to wl_keyboard::keymap_format::keymap_format_xkb_v1

                xkb_keysym_t sym = xkb_state_key_get_one_sym(mXkbState.get(), code);
                Qt::KeyboardModifiers modifiers = QXkbCommon::modifiers(mXkbState.get(), sym);

                Qt::Key qtKey = static_cast<Qt::Key>(QXkbCommon::keysymToQtKey(sym, modifiers, mXkbState.get(), code));

                if (qtKey != Qt::Key_unknown)
                    keys.append(qtKey);
            }
        }

        return keys;
    }

    bool hasKeyboardFocus() const
    {
        return m_focus;
    }

    Qt::KeyboardModifiers modifiers() const
    {
        Qt::KeyboardModifiers ret = Qt::NoModifier;
        if (!mXkbState)
            return ret;
        ret = QXkbCommon::modifiers(mXkbState.get());
        return ret;
    }

private:
    void keyboard_keymap(uint32_t format, int32_t fd, uint32_t size) override
    {
        mKeymapFormat = format;

        if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
            qDebug() << "unknown keymap format:" << format;
            close(fd);
            return;
        }

        char *map_str = static_cast<char *>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
        if (map_str == MAP_FAILED) {
            close(fd);
            return;
        }

        struct xkb_context *xkbContext = getGlobalXkbContextFromQt();
        if (xkbContext) {
            mXkbKeymap.reset(xkb_keymap_new_from_string(xkbContext, map_str, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS));
            QXkbCommon::verifyHasLatinLayout(mXkbKeymap.get());
        }

        munmap(map_str, size);
        close(fd);

        if (mXkbKeymap)
            mXkbState.reset(xkb_state_new(mXkbKeymap.get()));
        else
            mXkbState.reset(nullptr);
    }

    void keyboard_key([[maybe_unused]] uint32_t serial, [[maybe_unused]] uint32_t time, uint32_t key, uint32_t state) override
    {
        const bool isDown = state != WL_KEYBOARD_KEY_STATE_RELEASED;
        if (isDown) {
            auto it = std::find(mNativeKeys.begin(), mNativeKeys.end(), key);
            if (it != mNativeKeys.end()) {
                qWarning().nospace() << "WARNING: got a key-down for a key (" << Qt::hex << Qt::showbase << key << ") which is reported to be pressed already!";
            } else {
                mNativeKeys.append(key);
            }
        } else {
            auto it = std::find(mNativeKeys.begin(), mNativeKeys.end(), key);
            if (it == mNativeKeys.end()) {
                qWarning().nospace() << "WARNING: got a key-up for a key (" << Qt::hex << Qt::showbase << key << ") which is NOT reported to be pressed!";
            } else {
                mNativeKeys.erase(it);
            }
        }
    }
    void keyboard_modifiers([[maybe_unused]] uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) override
    {
        if (mXkbState)
            xkb_state_update_mask(mXkbState.get(), mods_depressed, mods_latched, mods_locked, 0, 0, group);
        mNativeModifiers = mods_depressed | mods_latched | mods_locked;
    }

    void keyboard_enter([[maybe_unused]] uint32_t serial, [[maybe_unused]] wl_surface *surface, [[maybe_unused]] wl_array *keys) override
    {
        mNativeKeys.clear();
        mNativeKeys.reserve(keys->size / sizeof(uint32_t));

        for (const uint32_t *keyPtr = static_cast<const uint32_t *>(keys->data);
             reinterpret_cast<const char *>(keyPtr) < static_cast<const char *>(keys->data) + keys->size;
             keyPtr++) {
            mNativeKeys.append(*keyPtr);
        }

        m_focus = true;
    }
    void keyboard_leave([[maybe_unused]] uint32_t serial, [[maybe_unused]] wl_surface *surface) override
    {
        mNativeKeys.clear();
        m_focus = false;
    }

    uint32_t mNativeModifiers = 0;
    QList<uint32_t> mNativeKeys;

    uint32_t mKeymapFormat = WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1;
    QXkbCommon::ScopedXKBKeymap mXkbKeymap;
    QXkbCommon::ScopedXKBState mXkbState;

    bool m_focus = false;
};


/****************************************************************************/
/*              KisWaylandKeyboardWatcher::Seat                          */
/****************************************************************************/


class KisWaylandKeyboardWatcher::Seat
    : public QWaylandClientExtensionTemplate<KisWaylandKeyboardWatcher::Seat>
    , public QtWayland::wl_seat
{
    Q_OBJECT
public:
    Seat();
    ~Seat() override;
    void seat_capabilities(uint32_t capabilities) override;
    bool hasKeyboardFocus() const;

    QList<Qt::Key> pressedKeys() const;
    Qt::KeyboardModifiers modifiers() const;

private:
    class Keyboard;

    bool m_focus = false;
    std::unique_ptr<KisWaylandKeyboardWatcher::Keyboard> m_keyboard;
};

KisWaylandKeyboardWatcher::Seat::Seat()
    : QWaylandClientExtensionTemplate(5)
{
    initialize();
}
KisWaylandKeyboardWatcher::Seat::~Seat()
{
    if (isActive()) {
        release();
    }
}

void KisWaylandKeyboardWatcher::Seat::seat_capabilities(uint32_t capabilities)
{
    const bool hasKeyboard = capabilities & capability_keyboard;
    if (hasKeyboard && !m_keyboard) {
        m_keyboard = std::make_unique<KisWaylandKeyboardWatcher::Keyboard>(get_keyboard());
    } else if (!hasKeyboard && m_keyboard) {
        m_keyboard.reset();
    }
}
bool KisWaylandKeyboardWatcher::Seat::hasKeyboardFocus() const
{
    return m_keyboard && m_keyboard->hasKeyboardFocus();
}

QList<Qt::Key> KisWaylandKeyboardWatcher::Seat::pressedKeys() const
{
    if (m_keyboard) {
        return m_keyboard->pressedKeys();
    }
    return {};
}

Qt::KeyboardModifiers KisWaylandKeyboardWatcher::Seat::modifiers() const
{
    if (!m_keyboard)
        return Qt::NoModifier;

    return m_keyboard->modifiers();
}

/****************************************************************************/
/*              KisWaylandKeyboardWatcher                                   */
/****************************************************************************/


KisWaylandKeyboardWatcher::KisWaylandKeyboardWatcher()
    : m_seat(new Seat())
{
}

KisWaylandKeyboardWatcher::~KisWaylandKeyboardWatcher()
{
}

bool KisWaylandKeyboardWatcher::hasKeyboardFocus() const
{
    return m_seat->hasKeyboardFocus();
}

QList<Qt::Key> KisWaylandKeyboardWatcher::pressedKeys() const
{
    return m_seat->pressedKeys();
}

Qt::KeyboardModifiers KisWaylandKeyboardWatcher::modifiers() const
{
    return m_seat->modifiers();
}

#include "KisWaylandKeyboardWatcher.moc"
