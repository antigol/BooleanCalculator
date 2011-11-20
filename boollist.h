#ifndef BOOLLIST_H
#define BOOLLIST_H

typedef unsigned int uint;

class BoolList
{
public:
    BoolList(int size = 0, bool defaultValue = false);
    BoolList(const BoolList &bl);
    ~BoolList();

    int size() const;
    void append(bool b);
    bool operator [](int i) const;
    void operator ()(int i, bool b);
    BoolList operator &(const BoolList &bl) const;
    BoolList &operator &=(const BoolList &bl);
    BoolList operator |(const BoolList &bl) const;
    BoolList operator ~() const;
    BoolList &operator ++();
    BoolList &operator --();
    bool operator ==(const BoolList &bl) const;

    bool isAllFalse() const;
    bool isAllTrue() const;
    int countTrue() const;
    bool isUnderflow() const;
    bool isOverflow() const;

private:
    uint *_data;
    int _datasize;
    int _size;

    int _state;
};

#include <QVector>

template <typename T>
QVector<T>& operator +=(QVector<T>& a, const BoolList& b)
{
    Q_ASSERT(a.size() == b.size());

    for (int i = 0; i < a.size(); ++i) {
        if (b[i])
            ++a[i];
    }

    return a;
}

template <typename T>
QVector<T>& operator -=(QVector<T>& a, const BoolList& b)
{
    Q_ASSERT(a.size() == b.size());

    for (int i = 0; i < a.size(); ++i) {
        if (b[i])
            --a[i];
    }

    return a;
}

template <typename T>
QVector<T>& operator &=(QVector<T>& a, const BoolList& b)
{
    Q_ASSERT(a.size() == b.size());

    for (int i = 0; i < a.size(); ++i) {
        if (!b[i])
            a[i] = 0;
    }

    return a;
}

#endif // BOOLLIST_H
