#include "booleancalcul.h"
#include <QDebug>
#include <QVector>
#include <QStringList>

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

        qSort(_variables.begin(), _variables.end());
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

        // comme on passe à un lot d'expressions plus complexes, on enleve les solutions déjà trouvées
        if (!result.isEmpty()) {
            truthtable &= ~BooleanCalcul(result).truthtable(variables());
        }

        QStringList exprlist;
        QList<BoolList> tablelist;
        QVector<int> counttable(_truthtable.size(), 0);
        BoolList expressiontables(_truthtable.size(), false);

        // une serie des bit pour determiner la forme de l'expression
        BoolList letters(variables().size(), false);
        while (!letters.isAllTrue()) {
            ++letters;
            // va de 000001 à 111111

            if (letters.countTrue() == exprsize) {
                QString exprOnlyLetters;
                for (int j = 0; j < letters.size(); ++j) {
                    if (letters[j])
                        exprOnlyLetters += _variables[j];
                }
                // une expression de tille [exprsize] est créée
                // exemple : ACD

                // serie de bit qui determine la négation de chaque lettre de l'expression
                BoolList negation(exprsize, true);
                while (!negation.isUnderflow()) {
                    QString expr = exprOnlyLetters;
                    for (int k = negation.size() - 1; k >= 0; --k) {
                        if (!negation[k])
                            expr.insert(k, '!');
                    }
                    // Exemple : expr == "A!CD" quand negation = {true, false, true}

                    // puis on calcule la table de verité de l'expression
                    BoolList exprtable = BooleanCalcul(expr).truthtable(variables());

                    if (((~_truthtable & exprtable).isAllFalse()) && (!(truthtable & exprtable).isAllFalse())) {
                        // Si l'expression est correcte et qu'elle est interessante

                        // On la rajoute dans la map
                        exprlist << expr;
                        tablelist << exprtable;

                        // On la rajoute dans la table de comptage
                        counttable += exprtable;
                        expressiontables |= exprtable;
                    }

                    --negation;
                    // va de 111111 à 000000 (aucun barré à tous barrés)
                }
            }
        }

        // Une fois toutes les expression de tille [exprsize] testées on regarde notre map
        while (counttable.count(0) != counttable.size()) {

            // Si la table de comptes contient un '1' il est evidant que l'expression qui l'a trouvé est indispensable
            if (counttable.contains(1)) {
                // La position du '1' sert à trouver l'expression qui lui correspond
                int pos = counttable.indexOf(1);

                // On cherche l'expression qui resoud le plus de cases
                int jj = 0;
                int best = 0;
                for (int j = 0; j < tablelist.size(); ++j) {
                    if (tablelist[j][pos]) {                        
                        int score = (expressiontables & tablelist[j]).countTrue();

                        if (score > best) {
                            best = score;
                            jj = j;
                        }
                    }
                }

                // On rajoute l'expression dans le résultat
                result += (result.isEmpty()?"":"+")+exprlist[jj];

                // On clear les cases qui n'on plus besoin d'être representées par une expression
                counttable &= ~tablelist[jj];
                expressiontables &= ~tablelist[jj];

                tablelist.removeAt(jj);
                exprlist.removeAt(jj);

            } else {
                // Si aucune case ne vaut 1 (exemple : !B!C!D+A!BC+BCD+!AB!C)
                // On supprime arbitrairement une expression de la liste
                counttable -= tablelist.last();
                tablelist.removeLast();
                exprlist.removeLast();
            }
        }

        if (BooleanCalcul(result).truthtable(variables()) == _truthtable)
            break;
    }

    return result;
}

BooleanCalcul::Error BooleanCalcul::error() const
{
    return _error;
}
