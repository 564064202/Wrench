/****************************************************************************************
 * Copyright (c) 2010-2012 Patrick von Reth <patrick.vonreth@gmail.com>                 *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#ifndef PARSER_H
#define PARSER_H
#include <QString>
#include <QHash>
#include "core/notification/notification.h"
#include <QSharedPointer>
#include <QObject>



class Parser:public QObject{
    Q_OBJECT
public:
    static class QByteArray download(const QUrl &address);

public:
    Parser(class SnarlNetworkFrontend* snarl);

     struct SnarlNotification parse(QString &msg,class QTcpSocket* client);

private:
    class SnarlNetworkFrontend *snarl;
    QString downloadIcon(const QString &address);
    enum snpTypes{
        TYPE,
        APP,
        VERSION,
        ACTION,
        REGISTER,
        ADD_CLASS,
        NOTIFICATION,
        UNREGISTER,
        CLASS,
        TITLE,
        TEXT,
        ICON,
        TIMEOUT,
        ERROR

    };

    QHash<QString,Parser::snpTypes> getSnpType;

};

#endif // PARSER_H
