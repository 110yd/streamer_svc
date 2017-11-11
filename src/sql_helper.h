#ifndef SQL_HELPER_H
#define SQL_HELPER_H

#include <QSqlQuery>
#include <QVariantMap>

/// \brief Sql operation helper methods
class sql_helper
{
public:
    static bool exec_single(QSqlQuery &source, QVariantMap  &target);
    static bool exec_table (QSqlQuery &source, QList<QVariantMap> &target);
};

#endif // SQL_HELPER_H
