#ifndef STREAMING_SVC_H
#define STREAMING_SVC_H

#include <Cutelyst/Application>

using namespace Cutelyst;

/// \brief Application instance
class streaming_svc : public Application
{
    Q_OBJECT
    CUTELYST_APPLICATION(IID "streaming_svc")
public:
    Q_INVOKABLE explicit streaming_svc(QObject *parent = 0);

    bool init() override;
   
    virtual bool postFork() override;
};

#endif //STREAMING_SVC_H

