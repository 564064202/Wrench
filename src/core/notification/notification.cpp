/*
    SnoreNotify is a Notification Framework based on Qt
    Copyright (C) 2013  Patrick von Reth <vonreth@kde.org>


    SnoreNotify is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SnoreNotify is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with SnoreNotify.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "notification.h"
#include "snore.h"
#include "notification/icon.h"
#include "notification/notification_p.h"
#include "plugins/plugincontainer.h"
#include <Qt>

using namespace Snore;


int Notification::DefaultTimeout = 10;

uint NotificationData::notificationCount = 0;

uint NotificationData::m_idCount = 1;

int NotificationData::notificationMetaID = qRegisterMetaType<Notification>();

Notification::Notification () :
    d(NULL)
{
}

Notification::Notification ( const QString &application, const QString &alert, const QString &title, const QString &text, const Icon &icon, int timeout,NotificationEnums::Prioritys::prioritys priority ):
    d(new  NotificationData(application,alert,title,text,icon,timeout,priority))
{
}

Notification::Notification ( const Notification &other ) :
    d(other.d)
{
}

Notification &Notification::operator=(const Notification &other)
{
    d = other.d;
    return *this;
}

Notification::~Notification()
{
}

const uint &Notification::id() const
{
    return d->m_id;
}

const Icon &Notification::icon() const
{
    return d->m_icon;
}

const int &Notification::timeout() const
{
    return d->m_timeout;
}

void Notification::setUpdateID(uint id)
{
    d->m_updateID = id;
}

const uint &Notification::updateID() const
{
    return d->m_updateID;
}

const Notification::Action *Notification::actionInvoked() const
{
    return d->m_actionInvoked;
}

void Notification::setActionInvoked ( Action *action )
{
    d->m_actionInvoked = action;
}

void Notification::setActionInvoked ( const int &id)
{
    d->m_actionInvoked = d->m_actions[id];
}

void Notification::setSource(SnoreFrontend *source){
    d->m_source = source;
}

SnoreFrontend *Notification::source() const
{
    return d->m_source;
}

const QString &Notification::application() const
{
    return d->m_application;
}

const QString &Notification::title() const
{
    return d->m_title;
}

const QString &Notification::text() const
{
    return d->m_text;
}

const QString &Notification::alert() const
{
    return d->m_alert;
}

bool Notification::sticky() const
{
    return d->m_timeout == 0;
}

void Notification::setSticky()
{
    d->m_timeout = 0;
}

const NotificationEnums::Prioritys::prioritys &Notification::priority() const
{
    return d->m_priority;
}

void Notification::addAction(Notification::Action *a) 
{
    qDebug()<<"Added notification"<<a->id<<a->name;
    d->m_actions.insert(a->id,a);
}


const QHash<int, Notification::Action *> &Notification::actions() const
{
    return d->m_actions;
}

const NotificationEnums::CloseReasons::closeReasons &Notification::closeReason(){
    return d->m_closeReason;
}

void Notification::setCloseReason(const NotificationEnums::CloseReasons::closeReasons &r){
    d->m_closeReason = r;
}

Hint &Notification::hints()
{
    return d->m_hints;
}

void Notification::setSilent(bool silent)
{
    d->m_hints.setValue("silent", silent);
}

bool Notification::isValid() const
{
    return d;
}

QDataStream &operator<< ( QDataStream &stream, const Notification &noti )
{
    stream << "Title: " << noti.title() << " Text: " << noti.text() << " ID: " << noti.id() ;
    return stream;
}


QDataStream &operator<< ( QDataStream &stream, const Notification::Action &a)
{
    stream<<a.id<<a.id;
    return stream;
}

