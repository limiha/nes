#pragma once

#ifndef DAC_BUILD
#error dac_api.h should only be included for the DAC build.
#endif

#include "..\src\debug.h"

#using <System.dll>

using namespace System;

namespace NesDac
{
	public interface class IDebugEngineCallback
	{
	public:
		int ReadMemory(u64 readAddress, int readBytes, void* buffer) = 0;
		void WriteMemory(u64 writeAddress, int writeBytes, const void* buffer) = 0;
		int GetNesExportRva(String^ name) = 0;
	};

	public interface class INesDebugEvent
	{
	};

	public interface class INesDebugLoadRomEvent : public INesDebugEvent
	{
	public:
		unsigned int GetMapperNumber();
	};

	ref class BaseEvent : INesDebugEvent
	{
	public:
		BaseEvent(const DebugEventInfo& dbgEvent)
			: _instance(dbgEvent._instance)
		{
		}

	protected:
		DebugService* GetRemoteDebugService()
		{
			return (DebugService*)_instance;
		}

	private:
		u64 _instance;
	};

	ref class LoadRomEvent : public BaseEvent, public INesDebugLoadRomEvent
	{
	public:
		LoadRomEvent(const DebugEventInfo& dbgEvent)
			: BaseEvent(dbgEvent)
		{
		}

		virtual unsigned int GetMapperNumber()
		{
			DPtr<DebugService> debugService(GetRemoteDebugService());
			return debugService->GetMapperNumber();
		}
	};

	public ref class Dac
	{
	public:
		static Dac^ AttachToProcess(u64 moduleBaseAddress, IDebugEngineCallback^ callback)
		{
			// Enable rom load events
			int enable = 1;
			int enableRva = callback->GetNesExportRva("g_dbgEnableLoadEvent");
			if (enableRva == 0)
			{
				// Missing export - cannot attach
				return nullptr;
			}

			callback->WriteMemory(moduleBaseAddress + enableRva, sizeof(int), &enable);

			_instance = gcnew Dac(callback);
			return _instance;
		}

		INesDebugEvent^ FilerException(u64 param0, u64 param1, u64 param2, u64 param3)
		{
			DebugEventInfo eventInfo;
			if (!_debugService->FilterException(param0, param1, param2, param3, &eventInfo))
			{
				// Not our exception.
				return nullptr;
			}

			switch (eventInfo._eventId)
			{
			case DEBUG_EVENT_LOAD_ROM:
				return gcnew LoadRomEvent(eventInfo);
			}

			return nullptr;
		}

	internal:
		static int ReadMemory(u64 readAddress, int readBytes, void* buffer)
		{
			if (_instance == nullptr)
				return 0;
			else
				return _instance->_callback->ReadMemory(readAddress, readBytes, buffer);
		}

	private:
		Dac(IDebugEngineCallback^ callback)
			: _callback(callback)
		{
			_debugService = new DebugService();
		}

	private:
		static Dac^ _instance;
		IDebugEngineCallback^ _callback;
		DebugService* _debugService;
	};
};

extern "C"
{
	int DacReadMemory(u64 readAddress, int readBytes, void* buffer)
	{
		return NesDac::Dac::ReadMemory(readAddress, readBytes, buffer);
	}
}