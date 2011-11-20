#ifndef BOOLEANCALCUL_H
#define BOOLEANCALCUL_H

#include <QString>

#include "boollist.h"

class BooleanCalcul
{
public:
    enum Error {
        Variable,
        Calcul,
        Truth,
        Karnaugh,
        None
    };

    BooleanCalcul(const QString &calcul);
    const QString &variables();
    QChar calculate(const QString &variables, const BoolList &values);
    BoolList truthtable(const QString &variables);
    QString karnaugh();
    enum Error error() const;

private:
    QString _string;
    QString _variables;
    BoolList _truthtable;
    enum Error _error;
};

#endif // BOOLEANCALCUL_H
