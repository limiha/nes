#include "pch.h"
#include "NesRuntimeComponent.h"
#include "StorageFileRom.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

//StorageFileRom::StorageFileRom(StorageFile^ romFile)
//    : RomBase("")
//    , _romFile(romFile)
//{
//}
//
//bool StorageFileRom::OpenRomFile()
//{
//    return create_task(_romFile->OpenReadAsync()).then([=](IRandomAccessStreamWithContentType^ stream)
//    {
//        if (!stream->CanRead)
//        {
//            return false;
//        }
//        else
//        {
//            _dataReader = ref new DataReader(stream);
//            create_task(_dataReader->LoadAsync(stream->Size)).wait();
//            return true;
//        }
//    }).get();
//}
//
//void StorageFileRom::CloseRomFile()
//{
//    _dataReader = nullptr;
//    _romFile = nullptr;
//}
//
//void StorageFileRom::ReadRomBytes(unsigned char* buf, long long count)
//{
//    _dataReader->ReadBytes(ArrayReference<unsigned char>(buf, (unsigned int)count));
//}

namespace NesRuntimeComponent
{
    StandardController::StandardController(::IStandardController* controller)
        :_controller(controller)
    {
    }

    void StandardController::A(bool state)
    {
        _controller->A(state);
    }

    void StandardController::B(bool state)
    {
        _controller->B(state);
    }

    void StandardController::Select(bool state)
    {
        _controller->Select(state);
    }

    void StandardController::Start(bool state)
    {
        _controller->Start(state);
    }

    void StandardController::Up(bool state)
    {
        _controller->Up(state);
    }

    void StandardController::Down(bool state)
    {
        _controller->Down(state);
    }

    void StandardController::Left(bool state)
    {
        _controller->Left(state);
    }

    void StandardController::Right(bool state)
    {
        _controller->Right(state);
    }

    Nes::Nes(::Nes* nes)
        : _nes(nes)
    {
    }

    Nes::~Nes()
    {
    }

    IAsyncOperation<Nes^>^ Nes::Create(StorageFile^ romFile)
    {
        return create_async([=]() {
            NPtr<StorageFileRom> rom(new StorageFileRom(romFile));
            NPtr<::Nes> nes;
            ::Nes::Create(static_cast<IRomFile*>(rom), nullptr, &nes);
            return ref new Nes(nes);
        });
    }

    void Nes::DoFrame(WriteOnlyArray<byte>^ screen)
    {
        _nes->DoFrame(screen->Data);
    }

    StandardController^ Nes::GetStandardController(unsigned int port)
    {
        ::IStandardController* controller = _nes->GetStandardController(port);
        return ref new StandardController(controller);
    }
}
