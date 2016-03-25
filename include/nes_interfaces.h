#pragma once

// Forward Declarations
struct IBaseInterface;
struct IReadStream;
struct IWriteStream;
struct IRomFile;
struct INes;
struct IAudioProvider;
struct IStandardController;

struct IBaseInterface
{
    virtual void AddRef() = 0;
    virtual void Release() = 0;
};

struct IReadStream : public IBaseInterface
{
    // Read bytes from the stream
    // buf: the buffer that bytes will be read into
    // count: the maximum number of bytes to read
    // returns: the total number of bytes read
    virtual int ReadBytes(unsigned char* buf, int count) = 0;
};

struct IWriteStream : public IBaseInterface
{

    // Write bytes to the stream
    // buf: the buffer containing bytse to be written
    // count: the maximum number of bytes to write
    // returns: the total number of bytes written
    virtual int WriteBytes(unsigned char* buf, int count) = 0;
};

struct IRomFile : public IBaseInterface
{
    virtual bool GetRomFileStream(IReadStream** stream) = 0;
    virtual bool GetSaveGameStream(IWriteStream** stream) = 0;
    virtual bool GetLoadGameStream(IReadStream** stream) = 0;
};

struct INes : public IBaseInterface
{
    virtual void Dispose() = 0;

    virtual void DoFrame(unsigned char screen[]) = 0;
    virtual IStandardController* GetStandardController(unsigned int port) = 0;
    virtual void SaveState() = 0;
    virtual void LoadState() = 0;
};

// Audio interface (implemented by host)
typedef void AudioCallback(void *userdata, unsigned char *stream, int len);
struct IAudioProvider : public IBaseInterface
{
    // Called by nes with its callback when it's ready to begin audio rendering
    virtual void Initialize(AudioCallback* callback, void* callbackData) = 0;

    // The following calls will only be made after Initialize is called
    virtual void PauseAudio() = 0;
    virtual void UnpauseAudio() = 0;
    virtual int GetSampleRate() = 0;
    virtual int GetBitsPerSample() = 0;
    virtual int GetSilenceValue() = 0;
};

// Standard Controller Interface
struct IStandardController : public IBaseInterface
{
    virtual void A(bool state) = 0;
    virtual void B(bool state) = 0;
    virtual void Select(bool state) = 0;
    virtual void Start(bool state) = 0;
    virtual void Up(bool state) = 0;
    virtual void Down(bool state) = 0;
    virtual void Left(bool state) = 0;
    virtual void Right(bool state) = 0;
};