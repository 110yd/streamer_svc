#include "logout.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>

using namespace Cutelyst;

/// \brief Default controller
logout::logout (QObject *parent) : Controller(parent) {}

/// \brief Handle logout action
void logout::index(Context *c)
{
    // Clear the user's state
    Authentication::logout(c);
    
    c->setStash("stylesheets", QStringList {
        "/static/css/spectre.min.css",
        "/static/css/app.css",
    });
    
    c->setStash("error_msg", "Осуществлен выход из системы");
    c->setStash("template", "login.html");
}

