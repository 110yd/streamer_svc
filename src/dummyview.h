#ifndef DUMMYVIEW_H
#define DUMMYVIEW_H

#include <Cutelyst/View>

/// \brief Dummy view renderer
class dummyview : public Cutelyst::View
{
public:
    dummyview ( QObject* parent, const QString& name );
    virtual QByteArray render ( Cutelyst::Context* c ) const override;
};

#endif // DUMMYVIEW_H
