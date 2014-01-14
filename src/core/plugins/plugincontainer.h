/*
    SnoreNotify is a Notification Framework based on Qt
    Copyright (C) 2013-2014  Patrick von Reth <vonreth@kde.org>


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

#ifndef PLUGINCONTAINER_H
#define PLUGINCONTAINER_H
#include "../snore_exports.h"
#include "../snore_p.h"

#include <QPointer>
#include <QSettings>
#include <QFlag>
#include <QPluginLoader>



namespace Snore{
class SnoreCore;
class SnorePlugin;
class SnoreFrontend;
class SnoreBackend;
class SnoreSecondaryBackend;

class SNORE_EXPORT PluginContainer{
public:
    static QHash<QString,PluginContainer*> pluginCache();

    PluginContainer(QString fileName,QString pluginName,SnorePlugin::PluginType type);
    ~PluginContainer();
    SnorePlugin *load();
    void unload();
    const QString &file();
    const QString &name();
    SnorePlugin::PluginType type();


    static SnorePlugin::PluginType typeFromString(const QString &t);
    static const QStringList &types();

private:
    void static updatePluginCache();
    static QHash<QString,PluginContainer*> s_pluginCache;

    static inline QSettings &cache()
    {
        static QSettings *_cache = NULL;
        if(_cache == NULL)
        {
            _cache = new QSettings("SnoreNotify","libsnore");
            _cache->beginGroup( SnoreCorePrivate::computeHash(SnoreCorePrivate::pluginDir().absolutePath().toLatin1()));
        }
        return *_cache;
    }

    QString m_pluginFile;
    QString m_pluginName;
    SnorePlugin::PluginType m_pluginType;
    QPluginLoader m_loader;
};
}


#endif//PLUGINCONTAINER_H
