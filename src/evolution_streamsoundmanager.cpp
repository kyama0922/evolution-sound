#include "evolution_soundimplement.h"
#include <process.h>

using namespace EVOLUTION;
using namespace EVOLUTION::SOUND;

StreamSoundManager::StreamSoundManager(IDirectSound* direct_sound, WAVEFORMATEX* primary_format) :m_thread_handle(NULL), mp_sound_device(direct_sound), mp_primary_format(primary_format){

#if _UNICODE
	wchar_t* str = new wchar_t[sizeof(this) * 2 + 1];
	wsprintf(str, TEXT("%p"), this);
	this->m_mutex = CreateMutex(NULL, FALSE, str);
	EVOLUTION_DELETE_ARRAY(str);
#else
	char* str = new char[sizeof(this) * 2 + 1];
	sprintf_s(str, TEXT("%p"), this);
	this->m_mutex = CreateMutex(NULL, FALSE, str);
	KLIB_DELETE_ARRAY(str);
#endif
	this->ThreadRun();
}

u32 StreamSoundManager::AddRef(){
    return this->m_instance_counter.AddRef();
}

u32 StreamSoundManager::Release(){
    u32 counter = this->m_instance_counter.Release();
    if (counter == 0){
        this->ThreadExitSignal();
        if (m_thread_handle != NULL){
            CloseHandle(this->m_thread_handle);
        }
        if (m_mutex != NULL){
            CloseHandle(this->m_mutex);
        }

        for (auto it = this->m_sound_list.begin(); it != this->m_sound_list.end(); it++)
        {
            StreamSound* stream_sound = (*it);
            delete stream_sound;
        }
        delete this;
    }
    return counter;
}

RESULT StreamSoundManager::QueryInterface(EVOLUTION::EVOLUTION_IID riid, void **ppvObject){

    if (IsEqualGUID(riid, EVOLUTION_GUID::IID_IUnknown))
    {
        *ppvObject = static_cast<IUnknown *>(this);
        this->AddRef();
    }
    else if (IsEqualGUID(riid, EVOLUTION_GUID::IID_IStreamSoundManager))
    {
        *ppvObject = static_cast<IStreamSoundManager*>(this);
        this->AddRef();
    }
    else if (IsEqualGUID(riid, EVOLUTION_GUID::IID_StreamSoundManager))
    {
        *ppvObject = static_cast<StreamSoundManager*>(this);
        this->AddRef();
    }
    else
    {
        *ppvObject = nullptr;
        return RESULT::E_no_instance;
    }
    return RESULT::S_ok;
}

void StreamSoundManager::CreateStreamSound(IStreamSound** pp_stream_sound, ISoundFileLoader* sound_file){
	*pp_stream_sound = nullptr;
	
	StreamSound* stream_sound = new StreamSound(this , this->mp_sound_device , this->mp_primary_format);
	stream_sound->Create(sound_file);
	*pp_stream_sound = stream_sound;
}

void StreamSoundManager::ThreadRun(){
	if (this->m_thread_handle != NULL){
		return;
	}
	
	this->m_thread_handle = (HANDLE)_beginthreadex(nullptr, 0, [](void* _this)->unsigned{
		StreamSoundManager* manager = (StreamSoundManager*)_this;
		manager->Execute();
		_endthreadex(0x00);
		return 0;
	}, this, 0, nullptr);
	
}

void StreamSoundManager::ThreadExitSignal(){
	if (this->m_thread_handle == NULL){
		return;
	}
	//FLAGの変更
	WaitForSingleObject(this->m_mutex, INFINITE);
    EVOLUTION_DISABLED_STATE(this->m_flag, StreamSoundManager::EXECUTE);
	ReleaseMutex(this->m_mutex);

	WaitForSingleObject(this->m_thread_handle, INFINITE);
	CloseHandle(this->m_thread_handle);
	this->m_thread_handle = NULL;
}

void StreamSoundManager::Execute(){

	//FLAGの変更
	WaitForSingleObject(this->m_mutex, INFINITE);
    EVOLUTION_ENABLE_STATE(this->m_flag, StreamSoundManager::EXECUTE);
	ReleaseMutex(this->m_mutex);

    while (EVOLUTION_IS_STATE(this->m_flag, StreamSoundManager::EXECUTE)){
		WaitForSingleObject(this->m_mutex, INFINITE);
		//QUEUEのデータを追加
		for (auto it = this->m_queue.begin(); it != this->m_queue.end(); it++)
		{
			u32 flg = (*it).second;
			StreamSound* stream_sound = (*it).first;
			if (StreamSoundManager::ADD == flg){
				this->m_sound_list.push_back(stream_sound);
			}
			else if (StreamSoundManager::DELETE == flg){
				this->m_sound_list.remove(stream_sound);
				delete stream_sound;
			}
		}
		this->m_queue.clear();
		ReleaseMutex(this->m_mutex);

		for (auto it = this->m_sound_list.begin(); it != this->m_sound_list.end(); it++)
		{
			StreamSound* stream_sound = (*it);
			
			if (stream_sound->IsPlay()){
				stream_sound->LoadStream();
			}
		}

        Sleep(CONFIG::Stream_Sleep_millisecond);
	}
}


void StreamSoundManager::Add(StreamSound* stream_sound){
	WaitForSingleObject(this->m_mutex, INFINITE);
	this->m_queue[stream_sound] = StreamSoundManager::ADD;
	ReleaseMutex(this->m_mutex);
}

void StreamSoundManager::Delete(StreamSound* stream_sound){
	WaitForSingleObject(this->m_mutex, INFINITE);
	this->m_queue[stream_sound] = StreamSoundManager::DELETE;
	ReleaseMutex(this->m_mutex);
}