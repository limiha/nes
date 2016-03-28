#include "pch.h"
#include "StorageFileRom.h"

using namespace Platform;

using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

StorageFileReadStream::StorageFileReadStream(DataReader^ dataReader)
    : _dataReader(dataReader)
{
}

bool StorageFileReadStream::Create(StorageFile^ file, IReadStream** stream)
{
    *stream = nullptr;
    bool ret = false;
    return create_task(file->OpenReadAsync()).then([=](IRandomAccessStreamWithContentType^ fileStream) {
        DataReader^ reader = ref new DataReader(fileStream);
        return create_task(reader->LoadAsync(fileStream->Size)).then([=](unsigned int count)
        {
            *stream = new StorageFileReadStream(reader);
            return true;
        }).get();
    }).get();
}

int StorageFileReadStream::ReadBytes(unsigned char* buf, int count)
{
    _dataReader->ReadBytes(ArrayReference<unsigned char>(buf, count));
    return count;
}

StorageFileRom::StorageFileRom(StorageFile^ file)
    : _romFile(file)
{
}

bool StorageFileRom::GetRomFileStream(IReadStream** stream)
{
    return StorageFileReadStream::Create(_romFile, stream);
}

bool StorageFileRom::GetSaveGameStream(IWriteStream** stream)
{
    return false;
}

bool StorageFileRom::GetLoadGameStream(IReadStream** stream)
{
    return false;
}