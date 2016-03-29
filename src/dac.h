#pragma once

#ifdef DAC_BUILD

extern "C"
{
	int DacReadMemory(unsigned long long readAddress, int readBytes, void* buffer);
}

template <class T>
class DPtr
{
public:
	DPtr()
	{
	}

	DPtr(const DPtr<T> &np)
	{
	}

	DPtr(T* p)
	{
		_p = p;
	}

	~DPtr()
	{
	}

	// Public methods
public:
	void Attach(T* p)
	{
		// No Op
	}

	T* Detach()
	{
		// No Op
	}

	void Release()
	{
		// No Op
	}

	// Operators
public:
	operator T*() const
	{
		return NotAvailable();
	}

	T& operator*() const
	{
		return *NotAvailable();
	}

	T* operator->() const
	{
		T* result = (T*)malloc(sizeof(T));
		DacReadMemory((unsigned long long)_p, sizeof(T), result);
		return result;
	}

	T** operator&()
	{
		return (T**)NotAvailable();
	}

	T* operator=(T* p)
	{
		return NotAvailable();
	}

	T* operator=(const DPtr<T>& src)
	{
		return NotAvailable();
	}

private:
	void* _p; // Size must match nes.dll implementation

	T* NotAvailable() const
	{
		// Operation not available from DAC
		__debugbreak();

		return (T*)_p;
	}
};

template <class T>
class NPtr
{
public:
    NPtr()
    {
    }

    NPtr(const NPtr<T> &np)
    {
    }

    NPtr(T* p)
    {
    }

    ~NPtr()
    {
    }

    // Public methods
public:
    void Attach(T* p)
    {
    }

    T* Detach()
    {
        return NotAvailable();
    }

    void Release()
    {
    }

    // Operators
public:
    operator T*() const
    {
        return NotAvailable();
    }

    T& operator*() const
    {
        return *NotAvailable();
    }

    T* operator->() const
    {
        return NotAvailable();
    }

    T** operator&()
    {
        return (T**)NotAvailable();
    }

    T* operator=(T* p)
    {
        return NotAvailable();
    }

    T* operator=(const NPtr<T>& src)
    {
        return NotAvailable();
    }

private:
    void* _p; // Size must match nes.dll implementation

    T* NotAvailable() const
    {
        // Operation not available from DAC
        __debugbreak();

        return (T*)_p;
    }
};

#else

#define DPtr NPtr

#endif
