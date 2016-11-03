#include "store.h"

#include <QtGui/QGuiApplication>

Store::Store(QQuickItem* parent):QQuickItem(parent)
{
    _loaded = false;

    //_autoLoad = true;
    //_autoSave = true;

    _name = "";

    QString sub;

    if(QGuiApplication::organizationName() != "")
        sub += QGuiApplication::organizationName() + QDir::separator();
    if(QGuiApplication::organizationDomain() != "")
        sub += QGuiApplication::organizationDomain() + QDir::separator();
    if(QGuiApplication::applicationName() != "")
        sub += QGuiApplication::applicationName() + QDir::separator();
    #ifdef QAK_STORE_VERSION_PATH
    if(QGuiApplication::applicationVersion() != "")
        sub += QGuiApplication::applicationVersion() + QDir::separator() ;
    #endif
    if(sub == "") {
         qWarning() << "Store warning: Couldn't resolve" << sub << "for storing values. Using generic directory: \"Qak\"";
         sub = "Qak";
    }

    while(sub.endsWith( QDir::separator() )) sub.chop(1);

    _store_path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+QDir::separator()+sub;

    _ensureStorePath();
    #ifdef QAK_DEBUG
    qDebug() << "Store using" << _store_path;
    #endif

    //qDebug() << "Setting objectName";
    //this->setObjectName(QString("Store"));

    QQuickItem *qi = new QQuickItem();

    // Run through all the properties
    // This is done to get a list of the QuickItems normal attributes which we don't want to save
    _blacklist.append("name");
    //_blacklist.append("autoLoad");
    //_blacklist.append("autoSave");
    _blacklist.append("isLoaded");
    const QMetaObject *metaobject = qi->metaObject();
    int count = metaobject->propertyCount();
    for (int i=0; i<count; ++i) {
        QMetaProperty metaproperty = metaobject->property(i);
        const char *name = metaproperty.name();
        _blacklist.append(name);
    }
    delete qi;
    qi = 0;
}
/*
Store::~Store()
{
    if( _autoSave ) {
        #ifdef QAK_DEBUG
        qDebug() << "Store auto saving";
        #endif
        save();
    }
}
*/

QString Store::name()
{
    return _name;
}
/*
bool Store::autoLoad()
{
    return _autoLoad;
}

bool Store::autoSave()
{
    return _autoSave;
}
*/

bool Store::isLoaded()
{
    return _loaded;
}


bool Store::existOnDisk()
{
    QFileInfo check(fullPath());
    return check.exists() && check.isFile();
}

QString Store::fullPath()
{
    return _store_path + QDir::separator() + _name;
}

void Store::setName(const QString &n)
{
    if (n != _name) {
        _name = n;

        /*
        if( _autoLoad ) {
            #ifdef QAK_DEBUG
            qDebug() << "Store auto loading" << _name;
            #endif
            load();
        }
        */
        if(_name.contains("/")) {
            QStringList parts = _name.split('/');
            if(!parts.isEmpty())
                parts.removeLast();
            QString new_path = _store_path + QDir::separator() + parts.join(QDir::separator());
            _ensurePath(new_path);
            #ifdef QAK_DEBUG
            qDebug() << "Store using name fragments as paths" << _name << new_path;
            #endif

        }
        emit nameChanged();
    }
}
/*
void Store::setAutoLoad(const bool &v)
{
    if (v != _autoLoad) {
        _autoLoad = v;
        emit autoLoadChanged();
    }
}

void Store::setAutoSave(const bool &v)
{
    if (v != _autoSave) {
        _autoSave = v;
        emit autoSaveChanged();
    }
}
*/
void Store::save()
{
    if(_name == "")
    {
        qCritical() << "Store error: Property \"name\" not set";
        emit error("Store error: Property \"name\" not set");
        return;
    }

    _ensureStorePath();

    QString path = _store_path + QDir::separator() + _name;
    QJsonDocument json = QJsonDocument();
    QJsonObject rootItem = QJsonObject();

    emit saving();
    // Run through all the properties
    const QMetaObject *metaobject = this->metaObject();
    int count = metaobject->propertyCount();
    for (int i=0; i<count; ++i) {
        QMetaProperty metaproperty = metaobject->property(i);
        const char *name = metaproperty.name();

        bool blacklisted = false;
        for (int i = 0; i < _blacklist.size(); ++i) {
            //qDebug() << _quick_item_names.at(i).toLocal8Bit().constData();
            if(_blacklist.at(i).toLocal8Bit().constData() == QString(name))
                blacklisted = true;
        }

        if(!blacklisted) {
            QVariant value = this->property(name);

            if(value.canConvert<QJSValue>())
                value = value.value<QJSValue>().toVariant();

            #ifdef QAK_DEBUG
            qDebug() << "Store" << _name << "property" << name << "stored" << value;
            #endif

            QJsonValue jValue;
            jValue = jValue.fromVariant(value);
            rootItem.insert(name,jValue);
        }
    }
    json.setObject(rootItem);
    QByteArray bJson = json.toJson(QJsonDocument::Indented);
    QFile file(path);
    if(file.open(QIODevice::WriteOnly))
        file.write(bJson);
    else
        qWarning() << "Store" << _name << "warning: Couldn't save to" << path;
    file.close();

    //qDebug() << "Store saved" << bJson << "in" << path;
    emit saved();
}

void Store::load()
{
    if(_name == "")
    {
        qCritical() << "Store error: Property \"name\" not set";
        emit error("Store error: Property \"name\" not set");
        return;
    }

    QString path = _store_path + QDir::separator() + _name;

    QString bJson;
    QFile file;
    file.setFileName(path);
    if(file.exists())
    {
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        bJson = file.readAll();
        file.close();
    }
    else
    {
        #ifdef QAK_DEBUG
        qWarning() << "Store" << _name << "warning: Couldn't load from" << path;
        #endif
        emit error("Could not load store from: \""+path+"\"");
        _loaded = true;
        emit isLoadedChanged();
        emit loaded();
        return;
    }

    QJsonDocument json = QJsonDocument::fromJson(bJson.toUtf8());
    QJsonObject rootItem = json.object();

    // Run through all the properties
    const QMetaObject *metaobject = this->metaObject();
    int count = metaobject->propertyCount();
    for (int i=0; i<count; ++i) {
        QMetaProperty metaproperty = metaobject->property(i);
        const char *name = metaproperty.name();
        //QVariant value = this->property(name);
        bool blacklisted = false;
        for (int i = 0; i < _blacklist.size(); ++i) {
            //qDebug() << _quick_item_names.at(i).toLocal8Bit().constData();
            if(_blacklist.at(i).toLocal8Bit().constData() == QString(name))
                blacklisted = true;
        }

        if(!blacklisted) {
            QJsonValue value = rootItem.value(QString(name));

            if(value != QJsonValue::Undefined)
            {
                this->setProperty(name,value.toVariant());
                #ifdef QAK_DEBUG
                qDebug() << "Store" << _name << "property" << name << "loaded" << value;
                #endif

            }
            else
                qWarning() << "Store" << _name << "warning: Property" << name << "not found";
        }
    }

    //qDebug() << "Store loaded" << bJson << "from" << path;
    _loaded = true;
    emit isLoadedChanged();
    emit loaded();
}

void Store::clear()
{
    if(_name == "")
    {
        qCritical() << "Store error: Property \"name\" not set";
        emit error("Property \"name\" not set");
        return;
    }

    QString path = _store_path + QDir::separator() + _name;
    bool result = QFile::remove(path);

    if(!result)
        emit error("Failed removing store directory "+path);
    #ifdef QAK_DEBUG
    else
        qDebug() << "Store clear" << path << result;
    #endif

    emit cleared();
}

void Store::clear(const QString &name)
{
    if(name == "")
    {
        qCritical() << "Store error: Argument \"name\" not set";
        emit error("Argument \"name\" not set");
        return;
    }

    QString path = _store_path + QDir::separator() + name;
    bool result = QFile::remove(path);

    if(!result)
        emit error("Failed removing store directory "+path);
    #ifdef QAK_DEBUG
    else
        qDebug() << "Store" << name << "clear" << path << result;
    #endif

    emit cleared();
}

void Store::clearAll()
{
    QDir dir(_store_path);
    if (dir.exists()) {
        dir.removeRecursively();

        #ifdef QAK_DEBUG
        qDebug() << "Store clearAll" << _store_path;
        #endif

        emit cleared();
    }
}

void Store::_ensureStorePath()
{
    // TODO fix silent fail?
    // This function is used in object constructor where _name is not yet set
    if(_name == "")
        return;

    _ensurePath(_store_path);
}

void Store::_ensurePath(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        if(!dir.mkpath("."))
            emit error("Failed creating store directory "+path);
        #ifdef QAK_DEBUG
        else
            qDebug() << "Store created directory" << path;
        #endif
    }

}