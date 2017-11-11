#include "users.h"
#include "sql_helper.h"

#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <QDateTime>
#include <QSqlDatabase>
#include <QJsonDocument>
#include <QRegExp>
#include <QSqlError>
#include <QDebug>

#include "authstoresql.h"

using namespace Cutelyst;

/// \brief Default constructor
users::users(QObject *parent) : 
    Controller(parent),
    m_usernameval("^[\\w\\d\\_\\-]{1,20}$")
{}

/// \brief Query all available users
QVariantList users::listUsers() {
    QSqlQuery query = CPreparedSqlQueryThreadForDB(
        "SELECT username, valid_to, active, is_admin "
        "FROM users "
        "ORDER BY is_admin DESC, "
        " active   DESC, "
        " username ", 
        "MyDB");
    
    QList<QVariantMap> entries;
    QVariantList result;
    
    if (sql_helper::exec_table(query, entries)) {
        for (auto &entry : entries) {
            if (!entry["valid_to"].isNull()) {
                auto time = QDate::fromString(entry["valid_to"].toString(), 
                                            Qt::ISODate);
                
                if (time < QDate::currentDate()) {
                    entry["active"] = 0;
                }
                
                entry["valid_to"] = time.toString(Qt::ISODate);
            }
            
            entry["selected"] = false;
            result.append(entry);
        }
    }
    
    return result;
}

/// \brief Index action
void users::index(Context *c)
{
    c->stash({
        { "entries",  QJsonDocument::fromVariant(listUsers()).toJson() },
        { "template", "userlist.html" }
    });

    htmlPage(c);
    noCache (c);
}

/// \brief Display all users list
void users::list(Context *c) {
    c->setStash(QStringLiteral("model"), listUsers());
    c->setView ("json");
    noCache    (c);
}

/// \brief Create user entity
void users::create(Cutelyst::Context* c)
{
    // Retrieve the values from the form
    QString username = c->request()->param("username", "");
    QString password = c->request()->param("password", "");
    QString valid_to = c->request()->param("valid_to", "");
    QString active   = c->request()->param("active",   "0");
    QString is_admin = c->request()->param("is_admin", "0");
    
    QStringList vlmsg;
    
    int isadmin  = is_admin.toInt();
    int isactive = 0;
    
    bool success = true;
    
    if (username.isEmpty()) {
        vlmsg   << "Имя пользователя должно быть указано";
        success = false;
    } else {
        if (!m_usernameval.exactMatch(username)) {
            vlmsg   << "Имя пользователя не подходит по формату";
            success = false;
        }
    }
    
    if (password.isEmpty()) {
        vlmsg   << "Пароль обязателен";
        success = false;
    }

    if (active == "1") {
        if (valid_to != "null") {
            QDate dateVal = QDate::fromString(valid_to, 
                                              Qt::ISODate);
            
            if (dateVal < QDate::currentDate()) {
                vlmsg << "Нельзя назначать дату активности раньше нынешней";
                success = false;
            } else {
                isactive = 1;
                valid_to = dateVal.toString(Qt::ISODate);
            }
        } else {
            isactive = 1;
            valid_to.clear();
        }
    } else {
        valid_to.clear();
        isactive = 0;
    } 
        
    if (success) {
        QSqlQuery query = CPreparedSqlQueryThreadForDB(
            "INSERT INTO users "
                   "( username,  password,  active,  valid_to,  is_admin) "
            "VALUES (:username, :password, :active, :valid_to, :is_admin)", 
            "MyDB");
        
        qDebug() << "binded " << username << password << isactive << valid_to << isadmin ;
        
        query.bindValue(":username", username);
        query.bindValue(":password", AuthStoreSql::hashFor(password));
        query.bindValue(":active",   isactive);
        query.bindValue(":valid_to", valid_to.isEmpty() ? QVariant() : valid_to);
        query.bindValue(":is_admin", isadmin);
        
        if (!query.exec()) {
            success = false;
            vlmsg << query.lastError().text();
        }
    }
    
    if (success) {
        c->setStash(QStringLiteral("model"), listUsers());
    }
    
    noCache(c);
    c->setStash(QStringLiteral("success"), success);
    c->setStash(QStringLiteral("message"), vlmsg);
    c->setView("json");
}

/// \brief Remove selected
void users::remove(Context *c) {
    QStringList vlmsg;
    bool success = true;
    
    QStringList users = c->request()->param("users", "").split(',');
    
    auto user = Authentication::user(c);
    
    users.removeAll(user.id());
    users.removeDuplicates();
    
    if (success) {
        QStringList queryBuilder;
        QString     queryString;
        queryBuilder << "DELETE FROM users WHERE username IN (";
        
        for (auto i = 0; i < users.size(); i++) {
            if (i)
                queryBuilder << ",";
            
            queryBuilder << "?";
        }
        
        queryBuilder << ")";
        queryString = queryBuilder.join("");
        
        QSqlQuery query = QSqlQuery(QSqlDatabase::database(
                                    Cutelyst::Sql::databaseNameThread("MyDB")));
        
        query.prepare(queryString);
        
        qDebug() << queryString;
        
        for (const auto &username : users) {
            query.addBindValue(username);
            qDebug() << "bind" << username;
        }
        
        if (!query.exec()) {
            success = false;
            vlmsg << query.lastError().text();
        }
    }
    
    if (success) {
        c->setStash(QStringLiteral("model"), listUsers());
    }
    
    noCache(c);
    c->setStash(QStringLiteral("success"), success);
    c->setStash(QStringLiteral("message"), vlmsg);
    c->setView("json");
}

/// \brief Remove inactive users
void users::removeInactive(Context *c) {
    QStringList vlmsg;
    bool success = true;
    
    if (success) {
        QSqlQuery query = CPreparedSqlQueryThreadForDB(
            "DELETE FROM users WHERE active = 0 OR valid_to < :now",
            "MyDB");
        
        query.bindValue(":now", QDate::currentDate().toString(Qt::ISODate));
        
        if (!query.exec()) {
            success = false;
            vlmsg << query.lastError().text();
        }
    }
    
    if (success) {
        c->setStash(QStringLiteral("model"), listUsers());
    }
    
    noCache(c);
    c->setStash(QStringLiteral("success"), success);
    c->setStash(QStringLiteral("message"), vlmsg);
    c->setView("json");
}

/// \brief Remove all users except administrator
void users::removeAll(Context *c) {
    QStringList vlmsg;
    bool success = true;
       
    if (success) {
        QSqlQuery query = CPreparedSqlQueryThreadForDB(
            "DELETE FROM users WHERE NOT username = :name",
            "MyDB");
        
        auto user = Authentication::user(c);
        
        query.bindValue(":name", user.id());
        
        if (!query.exec()) {
            success = false;
            vlmsg << query.lastError().text();
        }
    }
    
    if (success) {
        c->setStash(QStringLiteral("model"), listUsers());
    }
    
    noCache(c);
    c->setStash(QStringLiteral("success"), success);
    c->setStash(QStringLiteral("message"), vlmsg);
    c->setView("json");
}

/// \brief Update users activation context
void users::activate(Context *c) {
    QStringList vlmsg;
    bool success = true;
    
    QStringList users    = c->request()->param("users",   "").split(',');
    
    QString     valid_to = c->request()->param("valid_to", "unmodified");
    QString     activate = c->request()->param("activate", "unmodified");
    
    QStringList queryBuilder;

    bool set_valid_to = false;
    bool set_active   = false;
    
    QVariant validTo;
    int active = 0;
    
    queryBuilder << "UPDATE users SET ";
    
    if (valid_to != "unmodified") {
        queryBuilder << "valid_to = ? ";
        set_valid_to = true;
                
        if (valid_to != "null") {
            QDate date = QDate::fromString(valid_to, Qt::ISODate);
            
            if (date < QDate::currentDate())
            {
                success = false;
                vlmsg << "Невозможно поставить дату задним числом";
            } else {
                set_valid_to = true;
                validTo = date.toString(Qt::ISODate);
            } 
        } 
    }
    
    if (activate != "unmodified") {
        if (set_valid_to)
            queryBuilder << ", ";
        queryBuilder << "active = ? ";
        active = activate == "1";
        set_active = true;
    }
        
    users.removeDuplicates();
    
    if (success) {        
        QString     queryString;
        
        queryBuilder << " WHERE username IN (";
        
        for (auto i = 0; i < users.size(); i++) {
            if (i)
                queryBuilder << ",";
            
            queryBuilder << "?";
        }
        
        queryBuilder << ")";
        queryString = queryBuilder.join("");
        
        QSqlQuery query = QSqlQuery(QSqlDatabase::database(
                                    Cutelyst::Sql::databaseNameThread("MyDB")));
        
        query.prepare(queryString);
        
        qDebug() << queryString;
        
        if (set_valid_to)
            query.addBindValue(validTo);
        
        if (set_active)
            query.addBindValue(active);
        
        for (const auto &username : users) {
            query.addBindValue(username);
            qDebug() << "bind" << username;
        }
        
        if (!query.exec()) {
            success = false;
            vlmsg << query.lastError().text();
        }
    }
    
    if (success) {
        c->setStash(QStringLiteral("model"), listUsers());
    }
    
    noCache(c);
    c->setStash(QStringLiteral("success"), success);
    c->setStash(QStringLiteral("message"), vlmsg);
    c->setView("json");
}

/// \brief Create multiply users at once
void users::createMultiple(Context *c) {
    QStringList vlmsg;
    bool success = true;

    QString prefix   = c->request()->param("prefix",     "");
    QString valid_to = c->request()->param("valid_to",   "null");
    QString active   = c->request()->param("active",     "");
    QString count    = c->request()->param("create_cnt", "");
    
    int isactive  = 0;
    int createcnt = count.toInt();
    
    if (createcnt <= 0 || createcnt > 10) {
        success = false;
        vlmsg << "Количество создаваемых пользователей должно быть от 1 до 10";
    }
    
    if (active == "1") {
        if (valid_to != "null") {
            QDate dateVal = QDate::fromString(valid_to, 
                                              Qt::ISODate);
            
            if (dateVal < QDate::currentDate()) {
                vlmsg << "Нельзя назначать дату активности раньше нынешней";
                success = false;
            } else {
                isactive = 1;
                valid_to = dateVal.toString(Qt::ISODate);
            }
        } else {
            isactive = 1;
            valid_to.clear();
        }
    } else {
        valid_to.clear();
        isactive = 0;
    } 
    
    if (success) {
        QVariantList createdEntries;
        
        for (unsigned i = 0; i < createcnt; i++) {
            QByteArray passwddata;
            passwddata.resize(8);
            for (unsigned j = 0; j < 8; j++)
                passwddata[j] = qrand() % 0xff;
            
            auto password = passwddata.toBase64();
            unsigned uid  = i + 1;
            auto username = prefix + QString::number(uid);
            
            auto check_query = CPreparedSqlQueryThreadForDB(
                "SELECT COUNT(username) AS count FROM users "
                "WHERE username = :name",
                "MyDB");
            
            auto tries = 50;
            do {
                QVariantMap results;
                check_query.bindValue(":name", username);
                if (sql_helper::exec_single(check_query, results)) {
                    if (results["count"].toInt() == 0)
                        break;
                    uid = qrand();
                    username = prefix + QString::number(uid);
                } else {
                    success = false;
                    vlmsg << check_query.lastError().text();
                } 
            } while(--tries);
            
            if (tries == 0)
                continue;
            
            auto insert_query = CPreparedSqlQueryThreadForDB(
                "INSERT INTO users( username,  password,  active,  valid_to) "
                "VALUES "       " (:username, :password, :active, :valid_to)",
                "MyDB");
            
            insert_query.bindValue(":username", username);
            insert_query.bindValue(":password", AuthStoreSql::hashFor(password));
            insert_query.bindValue(":active",   isactive);
            insert_query.bindValue(":valid_to", valid_to);
            
            if (insert_query.exec()) {
                createdEntries << QVariantMap {
                    { "username", username },
                    { "password", password }
                };
            }
        }
        
        c->setStash(QStringLiteral("entries"), createdEntries);
    }
    
    if (success) {
        c->setStash(QStringLiteral("model"), listUsers());
    }
    
    noCache(c);
    c->setStash(QStringLiteral("success"), success);
    c->setStash(QStringLiteral("message"), vlmsg);
    c->setView("json");
}

/// \brief Modify user record
void users::modify(Context *c) {
    QStringList vlmsg;
    bool success = true;
    
    auto currentUser = Authentication::user(c);
        
    QString username = c->request()->param("username", "");
    QString password = c->request()->param("password", "");
    QString valid_to = c->request()->param("valid_to", "");
    QString active   = c->request()->param("active",   "");
    QString is_admin = c->request()->param("is_admin", "");

    QSqlQuery readquery = CPreparedSqlQueryThreadForDB(
        "SELECT valid_to, active, is_admin, password FROM users WHERE username = :name",
        "MyDB");
    
    readquery.bindValue(":name", username);
    
    QVariantMap entity;
    
    if (sql_helper::exec_single( readquery, entity)) {
        if (!password.isEmpty()) {
            entity["password"] = AuthStoreSql::hashFor(password);
        }
        
        if (!valid_to.isEmpty()) {
            if (valid_to == "null") {
                entity["valid_to"] = QVariant();
            } else {
                QDate date = QDate::fromString(valid_to, Qt::ISODate);
                
                if (date < QDate::currentDate())
                {
                    success = false;
                    vlmsg << "Невозможно поставить дату задним числом";
                } else {
                    entity["valid_to"] = date.toString(Qt::ISODate);
                } 
            } 
        }
        
        if (!active.isEmpty()) {
            auto updvalue = active == "1" ? 1 : 0;
            
            if (username != currentUser.id())
                entity["active"] = updvalue;
        }
        
        if (!is_admin.isEmpty()) {
            auto updvalue = is_admin == "1" ? 1: 0;
            
            if (username != currentUser.id())
                entity["is_admin"] = updvalue;
        }
        
        QSqlQuery writequery = CPreparedSqlQueryThreadForDB(
            "UPDATE users SET "
            "valid_to = :valid_to,"
            "active = :active,"
            "is_admin = :is_admin, "
            "password = :password "
            "WHERE username = :username",
            "MyDB");
        
        writequery.bindValue(":valid_to", entity["valid_to"]);
        writequery.bindValue(":active",   entity["active"]);
        writequery.bindValue(":is_admin", entity["is_admin"]);
        writequery.bindValue(":password", entity["password"]);
        writequery.bindValue(":username", username);
        
        success &= writequery.exec();
        if (!success) {
            vlmsg << writequery.lastError().text();
        }
        
    } else {
        success = false;
        vlmsg << readquery.lastError().text();
    }
    
    if (success) {
        c->setStash(QStringLiteral("model"), listUsers());
    }
    
    noCache(c);
    c->setStash(QStringLiteral("success"), success);
    c->setStash(QStringLiteral("message"), vlmsg);
    c->setView("json");
}

/// \brief Set web page context
void users::htmlPage(Context *c) {
    c->setStash("stylesheets", QStringList {
        "/static/css/spectre.min.css",
        "/static/css/spectre-exp.min.css",
        "/static/css/spectre-icons.min.css",
        "/static/css/app.css",
    });
    
    c->setStash("scripts", QStringList {
        "/static/js/alight.js",
        "/static/js/nanojax.js",
        "/static/js/app.js"
    });
}

/// \brief Disable caching
void users::noCache(Context *c) {
    c->response()->setHeader("Cache-Control", "no-cache");
}

/// \brief Check if user can perform actions
bool users::Auto(Context *c) {
    auto usr = Authentication::user(c);
    
    if (usr["is_admin"].toInt(nullptr) != 1)
    {
        c->response()->setStatus(403);
        c->response()->body() = "You are not allowed to access this page";
        c->setView("dummy");
        return false;
    }
    
    return true;
}
