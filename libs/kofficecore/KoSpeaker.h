/** @file
*   This file is part of the KDE/KOffice project.
*   Copyright (C) 2005, Gary Cramblitt <garycramblitt@comcast.net>
*
*   @author Gary Cramblitt <garycramblitt@comcast.net>
*   @since KOffice 1.5
*
*   This library is free software; you can redistribute it and/or
*   modify it under the terms of the GNU Library General Public
*   License as published by the Free Software Foundation; either
*   version 2 of the License, or (at your option) any later version.
*
*   This library is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   Library General Public License for more details.
*
*   You should have received a copy of the GNU Library General Public License
*   along with this library; see the file COPYING.LIB.  If not, write to
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*   Boston, MA 02110-1301, USA.
*/

#ifndef KOSPEAKER_H
#define KOSPEAKER_H

// Qt includes.
#include <qobject.h>
#include <qstring.h>

// KDE includes.
#include <ksharedptr.h>

// KOffice includes.
#include <koffice_export.h>

class QWidget;
class QPoint;
class KConfig;
class KoSpeakerPrivate;

#define kospeaker KoSpeaker::koSpeaker()

/** KoSpeaker is a singleton object that provides Text-to-Speech services for KOffice applications.
  * When activated, it will speak the text of widgets under the mouse pointer and/or the
  * the widget with focus.
  *
  * It also provides some methods for speaking text from documents.
  *
  * IMPORTANT: This class will be removed from KOffice when KOffice is converted to KDE4.
  * It will be replaced with a proper screen reading capability using the AT-SPI.
  *
  * This is quite a hack and doesn't work reliably.  The following are current problems:
  *   1.  Cannot speak menu items in a QMenuBar (top menu of app).
  *   2.  Doesn't understand every possible widget.
  *
  * This capability is @em not intended for completely blind users.  Such users cannot use
  * KDE 3.x anyway, since it lacks a screen reader.  Instead, this capability is intended as
  * an aid to users with other vision disabilities.
  *
  * KOffice applications can access this object using the kospeaker global.
  */
class KOFFICECORE_EXPORT KoSpeaker : public QObject, public KShared
{
   Q_OBJECT
public:
    KoSpeaker();
    ~KoSpeaker();

    /** Speech Options */
    enum SpeakFlags {
        SpeakFocusWidget =      0x0001  /**< Speak widget with focus */,
        SpeakPointerWidget =    0x0002  /**< Speak widget under mouse pointer */,
        SpeakWhatsThis =        0x0004  /**< Speak Whats This if available */,
        SpeakTooltip =          0x0008  /**< Speak tooltip if available */,
        SpeakAccelerator =      0x0010  /**< Speak accelerator */,
        SpeakDisabled =         0x0020  /**< Say 'disabled' if not enabled */
    };

    /**
     * Returns true if TTS services are available.  If KTTSD daemon is not running, it is started.
     * Will return false if:
     * -- KTTSD daemon is not installed, or
     * -- Was not able to start KTTSD daemon for some reason.
     */
    bool isEnabled() const;

    /**
     * Reads configuration options from @p config object and starts TTS if screen reader
     * capability is requested.
     * If KTTSD daemon is not installed, @ref isEnabled will return false.
     * If screen reader is requested and KTTSD is installed, but not running, it will be started.
     */
    void readConfig(KConfig* config);

    /**
     * Given a widget @p w and its @p pos screen coordinates, tries to extract the text of the widget
     * and speak it.  If @p pos is not specified, and the widget has multiple parts (such as
     * a QListView), uses the current part.
     * Call @ref isEnabled to ensure TTS is available before calling this method.
     */
    bool maybeSayWidget(QWidget* w, const QPoint& pos = QPoint());

    /**
     * Speak a @p msg that came from a widget, such as the widget's text label, tool tip, etc.
     * Speaks using ScreenReaderOutput, which has highest priority, and therefore, should only be
     * be used in very time-sensitive contexts and for short messages.
     * Certain standard substitutions are performed on the message.  For example, "Ctrl+" becomes
     * "control plus".  "Qt" markup is stripped.
     * @returns true if anything is actually spoken.
     * Call @ref isEnabled to ensure TTS is available before calling this method.
     */
    bool sayWidget(const QString& msg);

    /**
     * Cancels speaking of widget.  Usually called by slots that receive @ref customSpeakNewWidget
     * signal when they wish to speak the widget themselves.
     */
    void cancelSpeakWidget();

    /**
     * Queue a @p msg as a speech text job.  The text is encoded in the @p langCode language.
     * Examples "en", "es", "en_US".  If not specified, defaults to current desktop setting.
     * If @p first is true and a job is already speaking, cancel it.
     * If @p first is false, appends to the already queued job.
     * If the KTTSD daemon is not already running, it is started.
     */
    void queueSpeech(const QString& msg, const QString& langCode = QString(), bool first = true);

    /**
     * Start speaking queued text job (if any).
     */
    void startSpeech();

    /**
     * Returns whether the KTTSD deamon is installed in the system.  If not, apps should disable
     * or hide options/commands to speak.
     */
    static bool isKttsdInstalled();

    /**
     * Returns the KoSpeaker object singleton.  Apps should use "kospeaker" rather than this function
     * directly.
     */
    static KoSpeaker* koSpeaker() { return KSpkr; }

signals:
    /**
     * This signal is emitted whenever a new widget has received focus or the mouse pointer
     * has moved to a new widget.  If a receiver wishes to handle speaking of the widget itself,
     * it should call @ref cancelSpeakWidget() .
     * @param w         The widget.
     * @param p         Mouse pointer global coordinates, or in the case of a focus change (0,0).
     * @param flags     Speech options.  @ref SpeakFlags.
     *
     * IMPORTANT: This signal is emitted from the @ref maybeSayWidget method.  Slots who
     * call maybeSayWidget should take care to avoid infinite recursion.
     */
    void customSpeakNewWidget(QWidget* w, const QPoint& p, uint flags);

    /**
     * This signal is emitted each polling interval when KoSpeaker did not speak the widget
     * (either because it did not think the widget was a new one or because it did not
     * understand the widget).  If both mouse pointer and focus flags are set, it may
     * emit twice per polling interval.
     * @param w         The widget.
     * @param p         Mouse pointer global coordinates, or in the case of a focus change (0,0).
     * @param flags     Speech options.  @ref SpeakFlags.
     *
     * IMPORTANT: This signal is emitted frequently.  Receivers should be coded efficiently.
     */
    void customSpeakWidget(QWidget* w, const QPoint& p, uint flags);

protected:
    static KoSpeaker* KSpkr;

private slots:
    /**
     * Tells the class to do it's stuff - ie. figure out
     * which widget is under the mouse pointer or which has focus and speak it.
     */
    void probe();

private:
    // int menuBarItemAt(QMenuBar* m, const QPoint& p);

    // Start the KTTSD daemon if not already running.
    bool startKttsd();
    // Return the KTTSD daemon version string.
    QString getKttsdVersion();

    // These methods correspond to dcop interface in kdelibs/interfaces/kspeech/kspeech.h.
    // They use manual marshalling, instead of using kspeech_stub, because KOffice
    // supports KDE 3.3 and above and kspeech.h didn't appear until 3.4.
    void sayScreenReaderOutput(const QString &msg, const QString &talker);
    uint setText(const QString &text, const QString &talker);
    int appendText(const QString &text, uint jobNum=0);
    void startText(uint jobNum=0);
    void removeText(uint jobNum=0);

    KoSpeakerPrivate* d;
};

#endif      // H_KOSPEAKER
