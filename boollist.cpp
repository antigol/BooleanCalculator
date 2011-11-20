#include "boollist.h"
#include <cstring>
#include <QtGlobal>

#define SIZEOFINT ((int)(8 * sizeof (int)))

BoolList::BoolList(int size, bool defaultValue)
    : _data(0), _datasize(0), _size(size), _state(0)
{
    if (_size > 0) {
        _datasize = (_size + SIZEOFINT - 1) / SIZEOFINT;
        _data = new uint[_datasize];

        for (int i = 0; i < _datasize; ++i)
            _data[i] = defaultValue ? ~0x0 : 0x0;
    }
}

BoolList::BoolList(const BoolList &bl)
    : _datasize(bl._datasize), _size(bl._size), _state(bl._state)
{
    _data = new uint[_datasize];
    for (int i = 0; i < _datasize; ++i) {
        _data[i] = bl._data[i];
    }
}

BoolList::~BoolList()
{
    if (_data)
        delete[] _data;
}

int BoolList::size() const
{
    return _size;
}

void BoolList::append(bool b)
{
    if (_size < 0)
        _size = 0;
    _size++;
    if ((_size + SIZEOFINT - 1) / SIZEOFINT > _datasize) {
        uint *data = new uint[_datasize + 1];
        memcpy(data, _data, _datasize);
        delete[] _data;
        _data = data;
        _datasize++;
    }

    this->operator ()(_size - 1, b);
    _state = 0;
}

bool BoolList::operator [](int i) const
{
    int pos = i / SIZEOFINT;
    int mask = 1 << (i % SIZEOFINT);
    return mask & _data[pos];
}

void BoolList::operator ()(int i, bool b)
{
    int pos = i / SIZEOFINT;
    int mask = 1 << (i % SIZEOFINT);
    if (b)
        _data[pos] |= mask;
    else
        _data[pos] &= ~mask;

    _state = 0;
}

BoolList BoolList::operator &(const BoolList &bl) const
{
    BoolList ret(*this);

    ret &= bl;
    return ret;
}

BoolList & BoolList::operator &=(const BoolList &bl)
{
    if (&bl != this) {
        Q_ASSERT(_size == bl._size && _datasize == bl._datasize);

        for (int i = 0; i < _datasize; ++i) {
            _data[i] &= bl._data[i];
        }
    }

    return *this;
}

BoolList BoolList::operator |(const BoolList &bl) const
{
    Q_ASSERT(_size == bl._size && _datasize == bl._datasize);

    BoolList ret(_size);
    for (int i = 0; i < _datasize; ++i) {
        ret._data[i] = _data[i] | bl._data[i];
    }

    return ret;
}

BoolList BoolList::operator ~() const
{
    BoolList ret(_size);
    for (int i = 0; i < _datasize; ++i) {
        ret._data[i] = ~_data[i];
    }

    return ret;
}

BoolList & BoolList::operator ++()
{
    // 1 1 0 0 1 0 0 0  ---->   0 0 1 0 1 0 0 0
    // 1 1 1 1 1 1 1 1  ---->   0 0 0 0 0 0 0 0

    for (int i = 0; i < _datasize && ++_data[i] == 0; ++i) {
    }

    if (isAllFalse())
        _state = 1;
    else
        _state = 0;

    return *this;
}

BoolList & BoolList::operator --()
{
    for (int i = 0; i < _datasize && --_data[i] == ~((uint)0); ++i) {
    }

    if (isAllTrue())
        _state = -1;
    else
        _state = 0;

    return *this;
}

bool BoolList::operator ==(const BoolList &bl) const
{
    if (_size != bl.size())
        return false;

    int i;

    if (_size % SIZEOFINT == 0) {
        i = _datasize - 1;
    } else {
        i = _datasize - 2;

        if (_size > 0) {
            for (int i = SIZEOFINT * (_datasize - 1); i < _size; ++i) {
                if ((*this)[i] != bl[i])
                    return false;
            }
        }
    }

    for (; i >= 0; --i) {
        if (_data[i] != bl._data[i]) {
            return false;
        }
    }
    return true;
}

bool BoolList::isAllFalse() const
{
    int i;

    if (_size % SIZEOFINT == 0) {
        i = _datasize - 1;
    } else {
        i = _datasize - 2;

        if (_size > 0) {
            for (int i = SIZEOFINT * (_datasize - 1); i < _size; ++i) {
                if ((*this)[i])
                    return false;
            }
        }
    }

    for (; i >= 0; --i) {
        if (_data[i] != (uint)0x0) {
            return false;
        }
    }
    return true;
}

bool BoolList::isAllTrue() const
{
    int i;

    if (_size % SIZEOFINT == 0) {
        i = _datasize - 1;
    } else {
        i = _datasize - 2;

        if (_size > 0) {
            for (int i = SIZEOFINT * (_datasize - 1); i < _size; ++i) {
                if (!(*this)[i])
                    return false;
            }
        }
    }

    for (; i >= 0; --i) {
        if (_data[i] != ~((uint)0x0)) {
            return false;
        }
    }
    return true;
}

int BoolList::countTrue() const
{
    int count = 0;
    for (int i = 0; i < _size; ++i) {
        if (this->operator [](i))
            ++count;
    }
    return count;
}

bool BoolList::isUnderflow() const
{
    return _state == -1;
}

bool BoolList::isOverflow() const
{
    return _state == 1;
}

