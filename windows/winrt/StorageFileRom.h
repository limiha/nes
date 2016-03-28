#pragma once

#include "..\..\..\include\nes_api.h"

class StorageFileReadStream : public IReadStream, public NesObject
{
private:
    StorageFileReadStream(Windows::Storage::Streams::DataReader^ dataReader);

public:
    static bool Create(Windows::Storage::StorageFile^, IReadStream** stream);

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

public:
    int ReadBytes(unsigned char* buf, int count);

private:
    Windows::Storage::Streams::DataReader^ _dataReader;
};

class StorageFileWriteStream : public IWriteStream, public NesObject
{
public:
    DELEGATE_NESOBJECT_REFCOUNTING();

private:
    Windows::Storage::Streams::DataReader^ _dataReader;
};

class StorageFileRom : public IRomFile, public NesObject
{
public:
    StorageFileRom(Windows::Storage::StorageFile^ romFile);

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

public:
    bool GetRomFileStream(IReadStream** stream);
    bool GetSaveGameStream(IWriteStream** stream);
    bool GetLoadGameStream(IReadStream** stream);

private:
    Windows::Storage::StorageFile^ _romFile;
};