#include "booleancalcul.h"
#include <cmath>

BooleanCalcul::BooleanCalcul(const QString &calcul)
    : _string(calcul), _variables("\n"), _truthtable(-1), _error(None)
{
}

const QString & BooleanCalcul::variables()
{
    if (_variables.startsWith('\n')) {
        _variables.clear();

        for (int i = 0; i < _string.size(); ++i) {
            if (_string[i].isLetter()) {
                if (!_variables.contains(_string[i]))
                    _variables.append(_string[i]);
            }
        }
    }

    return _variables;
}

QChar BooleanCalcul::calculate(const QString &variables, const BoolList &values)
{
    QString string = _string;
    string.remove("\\s");

    Q_ASSERT(variables.size() == values.size());

    for (int i = 0; i < variables.size(); ++i) {
        string.replace(variables[i], values[i] ? '1':'0');
    }

    while (string.contains('(')) {
        int open = string.lastIndexOf('(');
        int close = string.indexOf(')', open);
        close = close == -1 ? string.size() - 1 : close;

        BooleanCalcul subcalcul(string.mid(open + 1, close - open - 1));
        string.replace(open, close - open + 1, subcalcul.calculate(variables, values));
    }

    QString old;

    do {
        old = string;
        string.replace("!0", "1");
        string.replace("!1", "0");
        string.replace("~0", "1");
        string.replace("~1", "0");
    } while (string != old);

    do {
        old = string;
        string.replace("0&0", "0");
        string.replace("0&1", "0");
        string.replace("1&0", "0");
        string.replace("1&1", "1");
        string.replace("0*0", "0");
        string.replace("0*1", "0");
        string.replace("1*0", "0");
        string.replace("1*1", "1");
        string.replace("00", "0");
        string.replace("01", "0");
        string.replace("10", "0");
        string.replace("11", "1");
    } while (string != old);

    do {
        old = string;
        string.replace("0+0", "0");
        string.replace("0+1", "1");
        string.replace("1+0", "1");
        string.replace("1+1", "1");
        string.replace("0|0", "0");
        string.replace("0|1", "1");
        string.replace("1|0", "1");
        string.replace("1|1", "1");
    } while (string != old);

    if (!(string.size() == 1 && string[0].isDigit())) {
        _error = Calcul;
    }

    return string[0];
}

BoolList BooleanCalcul::truthtable(const QString &variables)
{
    if (_truthtable.size() == -1) {
        BoolList values(variables.size(), false);

        while (!values.isOverflow()) {
            _truthtable.append(calculate(variables, values) == '1');

            ++values;
        }
    }

    return _truthtable;
}

QString BooleanCalcul::karnaugh()
{
    QString result;
    BoolList truthtable = this->truthtable(variables());

    if (truthtable.isAllFalse())
        return "0";
    if (truthtable.isAllTrue())
        return "1";

    /*
      Le calcul se base sur deux tables de verité (une constante et une qui est modifiée)
      T et U

      Je test si des expressions (tout dabors simples puis de plus en plus complexe)
      créées à partir des variables.
      Exemple : AB+!ABC
      Les expressions généres seront : A, !A, B, !B, C, !C, AB, A!B, !AB, !A!B, AC, ...

      Pour chaque taille d'expression {1, 2, ... nombre de variables dans le calcul}
      Je commance par supprimer les valeurs déjà découvertes précédament (par les expressions plus simples)
      U &= ~[déjà trouvé]

      il reste plus qu'a trouver les expressions qui sont contenue dans la "vraie table de verité" (celle qui ne varie pas)
      et qui correspondent aumoins à une valeur encore recherchée.
      (~T & [expression]) == 0  &&  (U & [expression]) != 0
                |                             |
                |                             +--- L'expression est interessante (elle aide à la résolution)
                +--- La verité contient l'expression
      */

    for (int exprsize = 1; exprsize <= variables().size(); ++exprsize) {
        if (!result.isEmpty()) {
            truthtable &= ~BooleanCalcul(result).truthtable(variables());
        }

        BoolList chooser(variables().size(), true);
        while (!chooser.isAllFalse()) {
            if (chooser.countTrue() == exprsize) {
                QString expr;
                for (int j = 0; j < chooser.size(); ++j) {
                    if (chooser[j])
                        expr += _variables[j];
                }

                BoolList statelist(exprsize, true);

                while (!statelist.isUnderflow()) {
                    QString calculexpr = expr;
                    for (int k = statelist.size() - 1; k >= 0; --k) {
                        if (!statelist[k])
                            calculexpr.insert(k, '!');
                    }
                    BoolList exprtable = BooleanCalcul(calculexpr).truthtable(variables());

                    if (((~_truthtable & exprtable).isAllFalse()) && (!(truthtable & exprtable).isAllFalse())) {
                        if (!result.isEmpty())
                            result += '+';
                        result += calculexpr;

                        BoolList testtruth = BooleanCalcul(result).truthtable(variables());
                        if (_truthtable == testtruth) {
                            return result;
                        }
                    }

                    --statelist;
                }
            }
            --chooser;
        }
    }

    return result;
}

BooleanCalcul::Error BooleanCalcul::error() const
{
    return _error;
}
