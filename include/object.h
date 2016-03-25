#pragma once

#define DELEGATE_NESOBJECT_REFCOUNTING() \
virtual void AddRef()       \
{                           \
    NesObject::AddRef();    \
}                           \
virtual void Release()      \
{                           \
    NesObject::Release();   \
}                           \

class NesObject
{
public:
    NesObject()
        : _refCount(1)
    {
    }

    virtual ~NesObject()
    {
    }

    virtual void AddRef()
    {
        _refCount++;
    }

    virtual void Release()
    {
        if (_refCount-- == 1)
        {
            delete this;
        }
    }

private:
    std::atomic<int> _refCount;
};