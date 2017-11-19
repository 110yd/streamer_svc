#ifndef AUTHENTICATEDSTATIC_H
#define AUTHENTICATEDSTATIC_H

#include <Cutelyst/Plugin>
#include <Cutelyst/Context>
#include <Cutelyst/Application>
#include <QString>
#include <memory>

/// \brief Plugin for serving static content with authentication
class AuthenticatedStatic : public Cutelyst::Plugin
{
    Q_OBJECT
    
public:
    AuthenticatedStatic(Cutelyst::Application *app);
    ~AuthenticatedStatic();
    
    void setBasePath(const QString &basePath);
    void setStaticDirectory(const QString &staticDirectory);
    
    bool setup(Cutelyst::Application* app) override;

private slots:
    void beforePrepareAction(Cutelyst::Context *c, bool *skipMethod);
    void serveStaticFile    (Cutelyst::Context *c);
    void respondForbidden   (Cutelyst::Context *c);
    void respondNotFound    (Cutelyst::Context *c);
    void respondFile        (Cutelyst::Context *c, const QString &file);
    
private:
    QString m_basePath;
    QString m_staticDirectory;
};

#endif // AUTHENTICATEDSTATIC_H
