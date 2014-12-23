#include "evolution_soundimplement.h"

using namespace EVOLUTION;
using namespace EVOLUTION::SOUND;

Sound3D::Sound3D(IDirectSound* direct_sound, WAVEFORMATEX* primary_format) :mp_sound_device(direct_sound), mp_primary_format(primary_format), mp_soundbuffer(nullptr), mp_sound3dbuffer(nullptr){

}

Sound3D::~Sound3D(){

}

u32 Sound3D::AddRef(){
    return this->m_instance_counter.AddRef();
}

u32 Sound3D::Release(){
    u32 counter = this->m_instance_counter.Release();
    if (counter == 0){
        EVOLUTION_RELEASE(this->mp_soundbuffer);
        EVOLUTION_RELEASE(this->mp_sound3dbuffer);
        delete this;
    }
    return counter;
}

RESULT Sound3D::QueryInterface(EVOLUTION::EVOLUTION_IID riid, void **ppvObject){

    if (IsEqualGUID(riid, EVOLUTION_GUID::IID_IUnknown))
    {
        *ppvObject = static_cast<IUnknown *>(this);
        this->AddRef();
    }
    else if (IsEqualGUID(riid, EVOLUTION_GUID::IID_ISound3D))
    {
        *ppvObject = static_cast<ISound3D*>(this);
        this->AddRef();
    }
    else if (IsEqualGUID(riid, EVOLUTION_GUID::IID_Sound3D))
    {
        *ppvObject = static_cast<Sound3D*>(this);
        this->AddRef();
    }
    else
    {
        *ppvObject = nullptr;
        return RESULT::E_no_instance;
    }
    return RESULT::S_ok;
}

void Sound3D::Create(ISoundFileLoader* soundfile){
	try{

		//サウンドバッファの設定
		DSBUFFERDESC dsdesc;
		ZeroMemory(&dsdesc, sizeof(DSBUFFERDESC));
		dsdesc.dwSize = sizeof(DSBUFFERDESC);
		dsdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRL3D | DSBCAPS_STICKYFOCUS;
		dsdesc.guid3DAlgorithm = DS3DALG_DEFAULT;

		dsdesc.dwBufferBytes = soundfile->GetSoundBufferSize();
		WAVEFORMATEX waveformatex = soundfile->GetSoundFormat();
		dsdesc.lpwfxFormat = &waveformatex;


		HRESULT hr;
		hr = this->mp_sound_device->CreateSoundBuffer(&dsdesc, &this->mp_soundbuffer, NULL);
		//KLIB_ASSERT_LOG(hr == S_OK);

		// ロック開始
		LPVOID pMem1, pMem2;
		u32 dwSize1, dwSize2;

		hr = this->mp_soundbuffer->Lock(0, soundfile->GetSoundBufferSize(), &pMem1, (LPDWORD)&dwSize1, &pMem2, (LPDWORD)&dwSize2, 0);
        EVOLUTION_ASSERT(hr == S_OK);

		//audioの長さを取得
		WAVEFORMATEX format;
		this->mp_soundbuffer->GetFormat((LPWAVEFORMATEX)&format, 0, nullptr);

		this->m_max_time = dwSize1 / format.nAvgBytesPerSec;

		TEMPLATE::Array<u8> buffer(soundfile->GetSoundBufferSize());
		//サウンドのコピー
        buffer.Set(soundfile->GetSoundBuffer(), soundfile->GetSoundBufferSize());
		// データ書き込み
		memcpy(pMem1, &buffer[0], dwSize1);
		memcpy(pMem2, &buffer[0] + dwSize1, dwSize2);

		// ロック解除
		this->mp_soundbuffer->Unlock(pMem1, dwSize1, pMem2, dwSize2);
        EVOLUTION_ASSERT(hr == S_OK);

		hr = this->mp_soundbuffer->QueryInterface(IID_IDirectSound3DBuffer, (LPVOID*)&this->mp_sound3dbuffer);
        EVOLUTION_ASSERT(hr == S_OK);
	}
	catch (...){

	}
}

ISound3D* Sound3D::Copy(){
	Sound3D* work = new Sound3D(this->mp_sound_device, this->mp_primary_format);
	work->m_max_time = this->m_max_time;
	work->m_now_time = this->m_now_time;

	WAVEFORMATEX format;
	auto ht = this->mp_soundbuffer->GetFormat(&format, sizeof(WAVEFORMATEX), NULL);

	void* srcmem1, *srcmem2;
	u32 srcsize1, srcsize2;
	this->mp_soundbuffer->Lock(0, 0, &srcmem1, (LPDWORD)&srcsize1, &srcmem2, (LPDWORD)&srcsize2, DSBLOCK_ENTIREBUFFER);


	//サウンドバッファの設定
	DSBUFFERDESC dsdesc;
	ZeroMemory(&dsdesc, sizeof(DSBUFFERDESC));
	dsdesc.dwSize = sizeof(DSBUFFERDESC);
	dsdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRL3D | DSBCAPS_STICKYFOCUS;
	dsdesc.guid3DAlgorithm = DS3DALG_DEFAULT;

	dsdesc.dwBufferBytes = srcsize1 + srcsize2;
	dsdesc.lpwfxFormat = &format;

	this->mp_sound_device->CreateSoundBuffer(&dsdesc, &work->mp_soundbuffer, NULL);

	void* destmem1, *destmem2;
	u32 destsize1, destsize2;
	work->mp_soundbuffer->Lock(0, dsdesc.dwBufferBytes, &destmem1, (LPDWORD)&destsize1, &destmem2, (LPDWORD)&destsize2, 0);

	memcpy(destmem1, srcmem1, srcsize1);
	memcpy(destmem2, srcmem2, srcsize2);

	work->mp_soundbuffer->Unlock(destmem1, destsize1, destmem2, destsize2);
	this->mp_soundbuffer->Unlock(srcmem1, srcsize1, srcmem2, srcsize2);

	work->mp_soundbuffer->QueryInterface(IID_IDirectSound3DBuffer, (LPVOID*)&work->mp_sound3dbuffer);
	work->x = this->x;
	work->y = this->y;
	work->z = this->z;

	work->mp_sound3dbuffer->SetPosition(work->x, work->y, work->z, DS3D_IMMEDIATE);
	return work;
}

void Sound3D::Play(bool loop){
	u32 flg = (loop) ? DSBPLAY_LOOPING : 0;
	this->mp_soundbuffer->Play(0, 0, flg);
}

void Sound3D::Pause(){
	this->mp_soundbuffer->Stop();
}

void Sound3D::Stop(){
	this->mp_soundbuffer->Stop();
	this->mp_soundbuffer->SetCurrentPosition(0);
}

void Sound3D::SetPos(float x, float y, float z){
	this->x = x;
	this->y = y;
	this->z = z;

	this->mp_sound3dbuffer->SetPosition(this->x, this->y, this->z, DS3D_IMMEDIATE);
}

void Sound3D::SetPlayPos(u32 second){
	this->mp_soundbuffer->SetCurrentPosition(second * this->mp_primary_format->nAvgBytesPerSec);
}

void Sound3D::SetFrequency(float frequency){
	u32 f = (u32)(this->mp_primary_format->nSamplesPerSec * frequency);
	this->mp_soundbuffer->SetFrequency(f);
}

u32 Sound3D::GetPlayPos(){
	u32 pos;
	this->mp_soundbuffer->GetCurrentPosition((LPDWORD)&pos, NULL);
	return pos / this->mp_primary_format->nAvgBytesPerSec;
}

float Sound3D::GetFrequency(){
	u32 fer;
	this->mp_soundbuffer->GetFrequency((LPDWORD)&fer);
	return (float)fer / this->mp_primary_format->nSamplesPerSec;
}

u32 Sound3D::GetTime(){
	return this->m_max_time;
}

bool Sound3D::IsPlay(){
	u32 state;
	this->mp_soundbuffer->GetStatus((LPDWORD)&state);
	return state & DSBSTATUS_PLAYING;
}