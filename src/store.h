#ifndef STORE_H
#define STORE_H

#include <QDebug>
#include <QObject>
#include <QJSValue>
#include <QVariant>
#include <QQmlListProperty>
#include <QVector>
#include <QMetaProperty>
#include <QStandardPaths>
#include <QDir>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>



#include "env_p.h"

class Store : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> content READ content)
    Q_CLASSINFO("DefaultProperty", "content")

    //Q_PROPERTY(bool autoLoad READ autoLoad WRITE setAutoLoad NOTIFY autoLoadChanged)
    //Q_PROPERTY(bool autoSave READ autoSave WRITE setAutoSave NOTIFY autoSaveChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY isLoadedChanged)
    Q_PROPERTY(bool onDisk READ onDisk NOTIFY onDiskChanged)
    Q_PROPERTY(QStringList skip READ skiplist WRITE setSkiplist NOTIFY skiplistChanged)

    public:
        explicit Store(QObject *parent = 0);

        QQmlListProperty<QObject> content();
    /*
        ~Store();

        bool autoLoad();
        void setAutoLoad(const bool &v);

        bool autoSave();
        void setAutoSave(const bool &v);
*/
        QString name();
        void setName(const QString &n);

        bool isLoaded();
        bool onDisk();
        void setOnDisk(bool onDisk);

        QStringList skiplist();
        void setSkiplist(const QStringList &skiplist);

        Q_INVOKABLE bool existOnDisk();
        Q_INVOKABLE QString fullPath();

    // Signals should only be defined in the header
    // as the implementation is autogenerated.
    // Also they cannot have return types, thus void
    signals:
        void cleared();
        void saving();
        void saved();
        void loaded();
        void error(const QString& msg);
        void nameChanged();
        void onDiskChanged();
        void skiplistChanged();
        //void autoLoadChanged();
        //void autoSaveChanged();
        void isLoadedChanged();

    public slots:
        void save();
        void load();
        void clear();
        void clear(const QString& name);
        void clearAll();

    private:
        QList<QObject *> _content;

        QString _name;
        QString _storePath;
        QStringList _blacklist;
        QStringList _skiplist;

        bool _onDisk;
        //bool _autoLoad;
        //bool _autoSave;
        bool _loaded;

        void _ensureStorePath();
        void _ensurePath(const QString& path);
};

#endif // STORE_H
