#pragma once

template <class T>
class NPtr
{
public:
    NPtr()
    {
        _p = nullptr;
    }

    NPtr(const NPtr<T> &np) : NPtr(np._p)
    {
    }

    NPtr(T* p)
    {
        _p = p;
        if (_p != nullptr)
            _p->AddRef();
    }

    ~NPtr()
    {
        if (_p != nullptr)
            _p->Release();
    }

    // Public methods
public:
    void Attach(T* p)
    {
        if (_p != nullptr)
            _p->Release();

        _p = p;
    }

    T* Detach()
    {
        T* p = _p;
        _p = nullptr;

        return p;
    }

    void Release()
    {
        T* p = _p;
        if (_p != nullptr)
        {
            _p = nullptr;
            p->Release();
        }
    }

    // Operators
public:
    operator T*() const
    {
        return _p;
    }

    T& operator*() const
    {
        return *_p;
    }

    T* operator->() const
    {
        return _p;
    }

    T** operator&()
    {
        return &_p;
    }

    T* operator=(T* p)
    {
        return Assign(p);
    }

    T* operator=(const NPtr<T>& src)
    {
        return Assign(src._p);
    }

private:
    T* _p;

    T* Assign(T* p)
    {
        if (_p != p)
        {
            if (p != nullptr)
                p->AddRef();

            if (_p != nullptr)
                _p->Release();

            _p = p;
        }

        return _p;
    }
};