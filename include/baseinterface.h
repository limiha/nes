#pragma once

struct IBaseInterface
{
    virtual void AddRef() = 0;
    virtual void Release() = 0;
};