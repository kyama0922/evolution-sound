#include "evolution_wav_loader.h"
//-------------------------------------------------------------------------------------------------
//			WAVLoader
//-------------------------------------------------------------------------------------------------
using namespace EVOLUTION;
using namespace EVOLUTION::SOUND;

u32 SoundFile_WavLoader::AddRef(){
    return this->m_instance_counter.AddRef();
}

u32 SoundFile_WavLoader::Release(){
    u32 counter = this->m_instance_counter.Release();
    if (counter == 0){
        EVOLUTION_DELETE_ARRAY(this->mp_sound_buffer);
        delete this;
    }
    return counter;
}

RESULT SoundFile_WavLoader::QueryInterface(EVOLUTION::EVOLUTION_IID riid, void **ppvObject){
   
    if (EVOLUTION_EQUALGUID(riid, EVOLUTION_GUID::IID_IUnknown))
    {
        *ppvObject = static_cast<IUnknown *>(this);
        this->AddRef();
    }
    else if (EVOLUTION_EQUALGUID(riid, EVOLUTION_GUID::IID_ISoundFileLoader))
    {
        *ppvObject = static_cast<ISoundFileLoader*>(this);
        this->AddRef();
    }
    else if (EVOLUTION_EQUALGUID(riid, EVOLUTION_GUID::IID_SoundFile_WavLoader))
    {
        *ppvObject = static_cast<SoundFile_WavLoader*>(this);
        this->AddRef();
    }
    else
    {
        *ppvObject = nullptr;
        return RESULT::E_no_instance;
    }
    return RESULT::S_ok;
}

SoundFile_WavLoader::SoundFile_WavLoader(const char* file_name){
	HRESULT hr;
	MMCKINFO mSrcWaveFile, mSrcWaveFmt, mSrcWaveData;
	LPWAVEFORMATEX wf;

	HMMIO audiofile;
	audiofile = mmioOpenA((LPSTR)file_name, NULL, MMIO_ALLOCBUF | MMIO_READ | MMIO_COMPAT);
	//KLIB_ASSERT_LOG(audiofile);
	// WAVEチャンクチェック
	ZeroMemory(&mSrcWaveFile, sizeof(mSrcWaveFile));
	hr = mmioDescend(audiofile, &mSrcWaveFile, NULL, MMIO_FINDRIFF);
	if (mSrcWaveFile.fccType != mmioFOURCC('W', 'A', 'V', 'E')) {
		mmioClose(audiofile, 0);
	}
	// fmtチャンクチェック
	ZeroMemory(&mSrcWaveFmt, sizeof(mSrcWaveFmt));
	hr = mmioDescend(audiofile, &mSrcWaveFmt, &mSrcWaveFile, MMIO_FINDCHUNK);
	if (mSrcWaveFmt.ckid != mmioFOURCC('f', 'm', 't', ' ')) {
		mmioClose(audiofile, 0);
	}
	// ヘッダサイズの計算
	int iSrcHeaderSize = mSrcWaveFmt.cksize;
	if (iSrcHeaderSize < sizeof(WAVEFORMATEX))
		iSrcHeaderSize = sizeof(WAVEFORMATEX);

	// ヘッダメモリ確保
	wf = (LPWAVEFORMATEX)malloc(iSrcHeaderSize);
	if (!wf) {
		mmioClose(audiofile, 0);
	}

	ZeroMemory(wf, iSrcHeaderSize);

	// WAVEフォーマットのロード
	hr = mmioRead(audiofile, (char*)wf, mSrcWaveFmt.cksize);
	if (FAILED(hr)) {
		free(wf);
		mmioClose(audiofile, 0);
	}
	// fmtチャンクに戻る
	mmioAscend(audiofile, &mSrcWaveFmt, 0);

	// dataチャンクを探す
	while (1) {
		// 検索
		hr = mmioDescend(audiofile, &mSrcWaveData, &mSrcWaveFile, 0);
		if (FAILED(hr)) {
			free(wf);
			mmioClose(audiofile, 0);
		}
		if (mSrcWaveData.ckid == mmioStringToFOURCCA("data", 0)){
			break;
		}
		// 次のチャンクへ
		hr = mmioAscend(audiofile, &mSrcWaveData, 0);
	}


	// サウンドバッファの作成
	this->m_sound_buffer_size = mSrcWaveData.cksize;
	this->mp_sound_buffer = new u8[this->m_sound_buffer_size];
	mmioRead(audiofile, (char*)this->mp_sound_buffer, this->m_sound_buffer_size);
	memcpy(&this->m_wav_format, wf, sizeof(WAVEFORMATEX));
	// ヘッダ用メモリを開放
	free(wf);
	// WAVを閉じる
	mmioClose(audiofile, 0);
}

SoundFile_WavLoader::SoundFile_WavLoader(const wchar_t* file_name){
	HRESULT hr;
	MMCKINFO mSrcWaveFile, mSrcWaveFmt, mSrcWaveData;
	LPWAVEFORMATEX wf;

	HMMIO audiofile;
	audiofile = mmioOpenW((LPWCH)file_name, NULL, MMIO_ALLOCBUF | MMIO_READ | MMIO_COMPAT);
	//KLIB_ASSERT_LOG(audiofile);
	// WAVEチャンクチェックig
	ZeroMemory(&mSrcWaveFile, sizeof(mSrcWaveFile));
	hr = mmioDescend(audiofile, &mSrcWaveFile, NULL, MMIO_FINDRIFF);
	if (mSrcWaveFile.fccType != mmioFOURCC('W', 'A', 'V', 'E')) {
		mmioClose(audiofile, 0);
	}
	// fmtチャンクチェック
	ZeroMemory(&mSrcWaveFmt, sizeof(mSrcWaveFmt));
	hr = mmioDescend(audiofile, &mSrcWaveFmt, &mSrcWaveFile, MMIO_FINDCHUNK);
	if (mSrcWaveFmt.ckid != mmioFOURCC('f', 'm', 't', ' ')) {
		mmioClose(audiofile, 0);
	}
	// ヘッダサイズの計算
	int iSrcHeaderSize = mSrcWaveFmt.cksize;
	if (iSrcHeaderSize < sizeof(WAVEFORMATEX))
		iSrcHeaderSize = sizeof(WAVEFORMATEX);

	// ヘッダメモリ確保
	wf = (LPWAVEFORMATEX)malloc(iSrcHeaderSize);
	if (!wf) {
		mmioClose(audiofile, 0);
	}

	ZeroMemory(wf, iSrcHeaderSize);

	// WAVEフォーマットのロード
	hr = mmioRead(audiofile, (char*)wf, mSrcWaveFmt.cksize);
	if (FAILED(hr)) {
		free(wf);
		mmioClose(audiofile, 0);
	}
	// fmtチャンクに戻る
	mmioAscend(audiofile, &mSrcWaveFmt, 0);

	// dataチャンクを探す
	while (1) {
		// 検索
		hr = mmioDescend(audiofile, &mSrcWaveData, &mSrcWaveFile, 0);
		if (FAILED(hr)) {
			free(wf);
			mmioClose(audiofile, 0);
		}
		if (mSrcWaveData.ckid == mmioStringToFOURCCW(L"data", 0)){
			break;
		}
		// 次のチャンクへ
		hr = mmioAscend(audiofile, &mSrcWaveData, 0);
	}


	// サウンドバッファの作成
	this->m_sound_buffer_size = mSrcWaveData.cksize;
	this->mp_sound_buffer = new u8[this->m_sound_buffer_size];
	mmioRead(audiofile, (char*)this->mp_sound_buffer, this->m_sound_buffer_size);
	memcpy(&this->m_wav_format, wf, sizeof(WAVEFORMATEX));
	// ヘッダ用メモリを開放
	free(wf);
	// WAVを閉じる
	mmioClose(audiofile, 0);
}


const u8* SoundFile_WavLoader::GetSoundBuffer()const{
    return this->mp_sound_buffer;
}

u32 SoundFile_WavLoader::GetSoundBufferSize()const{
	return this->m_sound_buffer_size;
}

const WAVEFORMATEX& SoundFile_WavLoader::GetSoundFormat()const{
	return this->m_wav_format;
}
