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
    std::atomic<u32> _readIndex;
    std::atomic<u32> _writeIndex;
    std::atomic_flag _lock;

    std::vector<T> _entries;
    u32 _size;

public:
    EventQueue(u32 size)
        : _readIndex(0)
        , _writeIndex(0)
        , _size(size)
    {
        _entries.resize(size);
        _lock.clear();
    }


    bool EnqueueEvent(const T& event)
    {
        SpinLockHolder lockHolder(this);

        int nextIndex = NextIndex(_writeIndex);
        if (nextIndex == _readIndex)
            return false; // Queue is full

        memcpy(&_entries[_writeIndex], &event, sizeof(T));
        _writeIndex.exchange(nextIndex);

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
        _readIndex.exchange(NextIndex(_readIndex));

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
            while (_queue->_lock.test_and_set(std::memory_order_acquire));
        }

        ~SpinLockHolder()
        {
            // Release lock
            _queue->_lock.clear(std::memory_order_release);
        }
    private:
        EventQueue<T>* _queue;
    };
};
