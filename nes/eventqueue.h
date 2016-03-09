#pragma once

//
// Fast queue implementation for events.
//
// Currently used for communicating events that change the audio channels from the emulator thread
// to the audio callback thread.
//
template <class T>
class EventQueue
{
private:
    volatile u32 _readIndex;
    volatile u32 _writeIndex;
    volatile u32 _lockHeld;

    std::vector<T> _entries;
    u32 _size;

public:
    EventQueue(u32 size)
        : _readIndex(0)
        , _writeIndex(0)
        , _lockHeld(0)
        , _size(size)
    {
        _entries.resize(size);
    }


    bool EnqueueEvent(const T& event)
    {
        SpinLockHolder lockHolder(this);

        int nextIndex = NextIndex(_writeIndex);
        if (nextIndex == _readIndex)
            return false; // Queue is full

        memcpy(&_entries[_writeIndex], &event, sizeof(T));
        InterlockedExchange(&_writeIndex, nextIndex);

        return true;
    }

    bool DequeueEvent(T& event)
    {
        // Check for empty before taking lock
        if (IsEmpty())
            return false;

        SpinLockHolder lockHolder(this);

        // Check for empty again after taking lock
        // Shouldn't be empty unless there are multiple readers.
        if (IsEmpty())
            return false;

        memcpy(&event, &_entries[_readIndex], sizeof(T));
        InterlockedExchange(&_readIndex, NextIndex(_readIndex));

        return true;
    }

    bool IsEmpty()
    {
        // Index updates are atomic so no need to lock here
        return _readIndex == _writeIndex;
    }

private:
    int NextIndex(int index)
    {
        index++;
        if (index == _size)
            return 0;
        else
            return index;
    }

    class SpinLockHolder
    {
    public:
        // Lock should only be Held for very short periods of time and performance
        // is critical here so we spinlock.

        SpinLockHolder(EventQueue<T>* queue)
            : _queue(queue)
        {
            // Acquire lock
            int wasHeld;
            do
            {
                wasHeld = InterlockedCompareExchange(&_queue->_lockHeld, 1, 0);
            } while (wasHeld);
        }

        ~SpinLockHolder()
        {
            // Release lock
            InterlockedExchange(&_queue->_lockHeld, 0);
        }
    private:
        EventQueue<T>* _queue;
    };
};
