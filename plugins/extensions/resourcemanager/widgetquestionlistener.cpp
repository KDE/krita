/*
 *  Copyright (c) 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *  Copyright (c) 2017 Aniketh Girish anikethgireesh@gmail.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "widgetquestionlistener.h"

#include <KNSCore/Question>

#include <KMessageBox>
#include <KPasswordDialog>
#include <QInputDialog>

class WidgetQuestionListenerHelper {
public:
    WidgetQuestionListenerHelper() : q(nullptr) {}
    ~WidgetQuestionListenerHelper() { delete q; }
    WidgetQuestionListener *q;
};
Q_GLOBAL_STATIC(WidgetQuestionListenerHelper, s_widgetQuestionListener)

WidgetQuestionListener* WidgetQuestionListener::instance()
{
    if(!s_widgetQuestionListener()->q) {
        new WidgetQuestionListener;
    }
    return s_widgetQuestionListener()->q;
}

WidgetQuestionListener::WidgetQuestionListener()
    : KNSCore::QuestionListener(nullptr)
{
    s_widgetQuestionListener()->q = this;
}

WidgetQuestionListener::~WidgetQuestionListener()
{
}

void WidgetQuestionListener::askQuestion(KNSCore::Question* question)
{
    switch(question->questionType())
    {
    case KNSCore::Question::SelectFromListQuestion:
        {
            bool ok = false;
            question->setResponse(QInputDialog::getItem(nullptr, question->title(), question->question(), question->list(), 0, false, &ok));
            if(ok) {
                question->setResponse(KNSCore::Question::OKResponse);
            }
            else {
                question->setResponse(KNSCore::Question::CancelResponse);
            }
        }
        break;
    case KNSCore::Question::ContinueCancelQuestion:
        {
            KMessageBox::ButtonCode response = KMessageBox::warningContinueCancel(nullptr, question->question(), question->title());
            if(response == KMessageBox::Continue) {
                question->setResponse(KNSCore::Question::ContinueResponse);
            }
            else {
                question->setResponse(KNSCore::Question::CancelResponse);
            }
        }
        break;
    case KNSCore::Question::PasswordQuestion:
        {
            KPasswordDialog dlg;
            dlg.setPrompt(question->question());
            if(dlg.exec()) {
                question->setResponse(dlg.password());
                question->setResponse(KNSCore::Question::ContinueResponse);
            }
            else {
                question->setResponse(KNSCore::Question::CancelResponse);
            }
        }
        break;
    case KNSCore::Question::YesNoQuestion:
    default:
        {
            KMessageBox::ButtonCode response = KMessageBox::questionYesNo(nullptr, question->question(), question->title());
            if(response == KMessageBox::Yes) {
                question->setResponse(KNSCore::Question::YesResponse);
            }
            else {
                question->setResponse(KNSCore::Question::NoResponse);
            }
        }
        break;
    }
}
