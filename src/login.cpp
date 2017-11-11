#include "login.h"

#include <QDebug>
#include <Cutelyst/Plugins/Authentication/authentication.h>

using namespace Cutelyst;

/// \brief Default constructor
login::login (QObject *parent) : Controller(parent) {}

/// \brief Handle login action
void login::index(Context *c)
{
    // Get the username and password from form
    QString username = c->request()->param("username");
    QString password = c->request()->param("password");
    
    c->setStash("stylesheets", QStringList {
        "/static/css/spectre.min.css",
        "/static/css/app.css",
    });
    
    bool user_invalid = false;
    
    // If the username and password values were found in form
    if (!username.isNull() && !password.isNull()) {
        // Attempt to log the user in
        if (Authentication::authenticate(c, 
                { 
                    {"username", username}, 
                    {"password", password} 
                })) 
        {
            auto user = Authentication::user(c);
            
            if (user["is_admin"].toInt() == 1) {
                // Navigate admin to users list        
                c->response()->redirect(
                    c->uriFor(c->controller("users")->actionFor("index"))
                );
            } else {
                // Navigate regular users to main page
                c->response()->redirect(
                    c->uriFor("/")
                );
            }
            
            return;
        } else {
            user_invalid = true;            
        }
    } else {
        if (!Authentication::userExists(c)) {
            user_invalid = true;
        } else {
            c->setStash("error_msg", "Укажите имя пользователя и пароль");    
        }
    }

    
    if (user_invalid)
        c->setStash("error_msg", "Неверное имя пользователя или пароль");
    
    // If either of above don't work out, send to the login page
    c->setStash("template", "login.html");
}

