//
// Copyright 2014 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "SendAreaToolBar.h"

#include <cassert>

#include <QtCore/QObject>
#include <QtGui/QIcon>

namespace comms_champion
{

namespace
{

const QString StartTooltip("Send Selected");
const QString StartAllTooltip("Send All");
const QString StopTooltip("Stop Sending");

const QIcon& startIcon()
{
    static const QIcon Icon(":/image/start.png");
    return Icon;
}

const QIcon& startAllIcon()
{
    static const QIcon Icon(":/image/start_all.png");
    return Icon;
}

const QIcon& stopIcon()
{
    static const QIcon Icon(":/image/stop.png");
    return Icon;
}

const QIcon& addIcon()
{
    static const QIcon Icon(":/image/add.png");
    return Icon;
}

const QIcon& editIcon()
{
    static const QIcon Icon(":/image/edit.png");
    return Icon;
}

const QIcon& deleteIcon()
{
    static const QIcon Icon(":/image/delete.png");
    return Icon;
}

QAction* createStartButton(QToolBar& bar)
{
    auto* action = bar.addAction(startIcon(), StartTooltip);
    QObject::connect(action, SIGNAL(triggered()),
                     GuiAppMgr::instance(), SLOT(sendStartClicked()));
    return action;
}

QAction* createStartAllButton(QToolBar& bar)
{
    auto* action = bar.addAction(startAllIcon(), StartAllTooltip);
    QObject::connect(action, SIGNAL(triggered()),
                     GuiAppMgr::instance(), SLOT(sendStartAllClicked()));
    return action;
}

QAction* createAddButton(QToolBar& bar)
{
    auto* action = bar.addAction(addIcon(), "Add New Message");
    QObject::connect(action, SIGNAL(triggered()),
                     GuiAppMgr::instance(), SLOT(sendAddClicked()));
    return action;
}

QAction* createEditButton(QToolBar& bar)
{
    auto* action = bar.addAction(editIcon(), "Edit Selected Message");
    QObject::connect(action, SIGNAL(triggered()),
                     GuiAppMgr::instance(), SLOT(sendEditClicked()));
    return action;
}

QAction* createDeleteButton(QToolBar& bar)
{
    auto* action = bar.addAction(deleteIcon(), "Delete Selected Message");
    QObject::connect(action, SIGNAL(triggered()),
                     GuiAppMgr::instance(), SLOT(sendDeleteClicked()));
    return action;
}

}  // namespace

SendAreaToolBar::SendAreaToolBar(QWidget* parent)
  : Base(parent),
    m_startStopButton(createStartButton(*this)),
    m_startStopAllButton(createStartAllButton(*this)),
    m_addButton(createAddButton(*this)),
    m_editButton(createEditButton(*this)),
    m_deleteButton(createDeleteButton(*this)),
    m_state(GuiAppMgr::instance()->sendState()),
    m_listEmpty(GuiAppMgr::instance()->sendListEmpty())
{
    auto* guiAppMgr = GuiAppMgr::instance();
    connect(
        guiAppMgr, SIGNAL(sigSendListEmpty(bool)),
        this, SLOT(sendListEmptyReport(bool)));

    connect(
        guiAppMgr, SIGNAL(sigSendMsgSelected(bool)),
        this, SLOT(sendMsgSelectedReport(bool)));

    connect(
        guiAppMgr, SIGNAL(sigSetSendState(int)),
        this, SLOT(stateChanged(int)));


    refresh();
}

void SendAreaToolBar::sendListEmptyReport(bool empty)
{
    m_listEmpty = empty;
    refresh();
}

void SendAreaToolBar::sendMsgSelectedReport(bool selected)
{
    m_msgSelected = selected;
    refresh();
}

void SendAreaToolBar::stateChanged(int state)
{
    auto castedState = static_cast<State>(state);
    if (m_state == castedState) {
        return;
    }

    m_state = castedState;
    refresh();
    return;
}

void SendAreaToolBar::refresh()
{
    refreshStartStopButton();
    refreshStartStopAllButton();
    refreshAddButton();
    refreshEditButton();
    refreshDeleteButton();
}

void SendAreaToolBar::refreshStartStopButton()
{
    auto* button = m_startStopButton;
    assert(button);
    bool enabled = (!m_listEmpty) && m_msgSelected;
    button->setEnabled(enabled);

    if (m_state == State::SendingSingle) {
        button->setIcon(stopIcon());
        button->setText(StopTooltip);
    }
    else {
        button->setIcon(startIcon());
        button->setText(StartTooltip);
    }
}

void SendAreaToolBar::refreshStartStopAllButton()
{
    auto* button = m_startStopAllButton;
    assert(button);
    bool enabled = !m_listEmpty;
    button->setEnabled(enabled);

    if (m_state == State::SendingAll) {
        button->setIcon(stopIcon());
        button->setText(StopTooltip);
    }
    else {
        button->setIcon(startAllIcon());
        button->setText(StartAllTooltip);
    }
}

void SendAreaToolBar::refreshAddButton()
{
    auto* button = m_addButton;
    assert(button);
    bool enabled = (m_state == State::Idle);
    button->setEnabled(enabled);
}

void SendAreaToolBar::refreshEditButton()
{
    auto* button = m_editButton;
    assert(button);
    bool enabled =
        (m_state == State::Idle) &&
        (m_msgSelected);
    button->setEnabled(enabled);
}

void SendAreaToolBar::refreshDeleteButton()
{
    auto* button = m_deleteButton;
    assert(button);
    bool enabled =
        (m_state == State::Idle) &&
        (m_msgSelected);
    button->setEnabled(enabled);
}


}  // namespace comms_champion

