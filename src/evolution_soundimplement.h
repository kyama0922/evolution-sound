#ifndef __EVOLUTION_SOUND_IMPLEMENT_H__
#define __EVOLUTION_SOUND_IMPLEMENT_H__

#include <evolution.h>
#include "../evolution_sound_interface.h"

//----------------------------------
// WAV
//----------------------------------
#include "evolution_wav_loader.h"

#include <dsound.h>
#include <list>
#include <map>

#ifndef NEW
#include <crtdbg.h>
#if _DEBUG
#define NEW  new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif


namespace EVOLUTION{
	namespace SOUND{

		class Sound :public ISound{
		private:
            InstanceCounter m_instance_counter;
			IDirectSoundBuffer* mp_soundbuffer;
			u32 m_max_time;
			u32 m_now_time;

			WAVEFORMATEX *mp_primary_format;
			IDirectSound *mp_sound_device;
		public:
            u32 AddRef();
            RESULT QueryInterface(EVOLUTION_IID riid, void **ppvObject);
            u32 Release();

			Sound(IDirectSound* direct_sound, WAVEFORMATEX* primary_format);
			~Sound();
			void Create(ISoundFileLoader* soundfile);
			ISound* Copy();//サウンドのコピー
			void Play(bool loop);//再生
			void Pause();//一時停止
			void Stop();//停止
			void SetPan(s32 pan);//パン
			void SetVolume(s32 volume);//ボリューム-10000 ~ 0
			void SetPlayPos(u32 second);//再生タイム秒
			void SetFrequency(float frequency);//周波数
			s32 GetVolume();//ボリューム取得
			float GetFrequency();	//周波数
			u32 GetPlayPosSecond();//再生タイム秒取得
			u32 GetPlayPosMillSecond();//再生タイムミリ秒取得
			u32 GetPlayPosBuffer();//バッファ再生場所取得
			u32 GetTime();
			bool IsPlay();
		};

        //拡張サウンド
        class WriteSound :public IWriteSound{
        private:
            InstanceCounter m_instance_counter;
            IDirectSoundBuffer* mp_soundbuffer;
            u32 m_max_time;
            u32 m_now_time;

            WAVEFORMATEX *mp_primary_format;
            IDirectSound *mp_sound_device;

            u32 m_buffer_size;
        public:
            u32 AddRef();
            RESULT QueryInterface(EVOLUTION_IID riid, void **ppvObject);
            u32 Release();

            WriteSound(IDirectSound* direct_sound, WAVEFORMATEX* primary_format);
            ~WriteSound();

            RESULT Create(u32 buffer_size, const WAVEFORMATEX& sound_format);

            ISound* Copy();//サウンドのコピー
            void Play(bool loop);//再生
            void Pause();//一時停止
            void Stop();//停止
            void SetPan(s32 pan);//パン
            void SetVolume(s32 volume);//ボリューム-10000 ~ 0
            void SetPlayPos(u32 second);//再生タイム秒
            void SetFrequency(float frequency);//周波数
            s32 GetVolume();//ボリューム取得
            f32 GetFrequency();	//周波数
            u32 GetPlayPosSecond();//再生タイム秒取得
            u32 GetPlayPosMillSecond();//再生タイムミリ秒取得
            u32 GetPlayPosBuffer();//バッファ再生場所取得
            u32 GetTime();
            bool IsPlay();

            u32 GetBufferSize()const;//バッファサイズの取得
            void WriteBuffer(u32 write_position, void* buffer, s32 buffer_size);//バッファを書き込む
        };

		class Sound3D : public ISound3D{
		private:
            InstanceCounter m_instance_counter;
			IDirectSoundBuffer* mp_soundbuffer;
			u32 m_max_time;
			u32 m_now_time;

			IDirectSound3DBuffer*	mp_sound3dbuffer;
			float x, y, z;

			WAVEFORMATEX *mp_primary_format;
			IDirectSound *mp_sound_device;
		public:
            u32 AddRef();
            RESULT QueryInterface(EVOLUTION_IID riid, void **ppvObject);
            u32 Release();

			Sound3D(IDirectSound* direct_sound, WAVEFORMATEX* primary_format);
			~Sound3D();
			void Create(ISoundFileLoader* soundfile);
			ISound3D* Copy();//サウンドのコピー
			void Play(bool loop);//再生
			void Pause();	//一時停止
			void Stop();//停止
			void SetPos(float x, float y, float z);//3D空間上の場所に配置
			void SetPlayPos(u32 second);//再生タイム秒
			void SetFrequency(float frequency);//周波数設定
			u32 GetPlayPos();//再生タイム秒取得
            f32 GetFrequency();//周波数取得
			u32 GetTime();
			bool IsPlay();
		};

		class StreamSoundManager;
		class StreamSound : public IStreamSound{
		private:
            InstanceCounter m_instance_counter;
			bool m_loop;

			u32 m_buffer_size;

			IDirectSoundBuffer* mp_soundbuffer;
			
			u32 m_max_time;
			u32 m_load_buffer_number;

            EVOLUTION::CORE::FILE::IFileRead* mp_tmp_file;//音楽データの一時展開データ

			WAVEFORMATEX *mp_primary_format;
			IDirectSound *mp_sound_device;
			StreamSoundManager* mp_stream_sound_manager;
		public:
            u32 AddRef();
            RESULT QueryInterface(EVOLUTION_IID riid, void **ppvObject);
            u32 Release();

			StreamSound(StreamSoundManager* stream_sound_manager, IDirectSound* direct_sound, WAVEFORMATEX* primary_format);
			~StreamSound();
			void Create(ISoundFileLoader* soundfile);
			IStreamSound* Copy();//サウンドのコピー
			void Play(bool loop);//再生
			void Pause();//一時停止
			void Stop();//停止
			void SetPan(s32 pan);//パン
			void SetVolume(s32 volume);//ボリューム-10000 ~ 0
			void SetPlayPos(u32 second);//再生タイム秒
			void SetFrequency(float frequency);//周波数
			s32 GetVolume();//ボリューム取得
            f32 GetFrequency();	//周波数
			u32 GetPlayPosSecond();//再生タイム秒取得
			u32 GetPlayPosMillSecond();//再生タイムミリ秒取得
			u32 GetTime();

			u32 GetPlayPosBuffer();//バッファ再生場所取得

			void LoadStream();//バッファの読み込み

			bool IsPlay();
			bool IsLoop();
		};

#ifdef DELETE
#undef DELETE
#endif
		class StreamSoundManager :public IStreamSoundManager{
		private:
            InstanceCounter m_instance_counter;
			HANDLE m_mutex;
			HANDLE m_thread_handle;
			u32    m_flag;
			enum _FLAG{
				EXECUTE = 0x00000001L,

				ADD = 0x00000010L,
				DELETE = 0x00000100L,
			};

			std::map<StreamSound* , u32> m_queue;
			std::list<StreamSound*> m_sound_list;

			WAVEFORMATEX *mp_primary_format;
			IDirectSound *mp_sound_device;
		public:
            u32 AddRef();
            RESULT QueryInterface(EVOLUTION_IID riid, void **ppvObject);
            u32 Release();

			StreamSoundManager(IDirectSound* direct_sound, WAVEFORMATEX* primary_format);
			void CreateStreamSound(IStreamSound** pp_stream_sound, ISoundFileLoader* sound_file);

			void ThreadRun();
			void ThreadExitSignal();
			void Execute();

			void Add(StreamSound* stream_sound);
			void Delete(StreamSound* stream_sound);
		};

		class SoundFactory : public ISoundFactory{
		private:
            InstanceCounter m_instance_counter;
			WAVEFORMATEX m_primary_format;
			IDirectSound *mp_sound_device;
			IDirectSoundBuffer* mp_primary_buffer;
		public:
            u32 AddRef();
            RESULT QueryInterface(EVOLUTION_IID riid, void **ppvObject);
            u32 Release();

			SoundFactory();
			~SoundFactory();
			void Create(HWND hwnd);
			void CreateStreamSoundManager(IStreamSoundManager** pp_stream_sound_manager);
			void CreateSound(ISound** pp_sound, ISoundFileLoader* sound_file);
            void CreateWriteSound(IWriteSound** pp_write_sound, u32 buffer_size, const WAVEFORMATEX& sound_format);
			void CreateSound3D(ISound3D** pp_sound3d, ISoundFileLoader* sound_file);
			void CreateSoundFile(ISoundFileLoader** p_sound_file, const char* file_name);
		};

	}
    namespace EVOLUTION_GUID{
        // {050CF4C2-792C-41d2-BE5F-DD91538C3120}
        static const EVOLUTION_IID IID_Sound =
        { 0x50cf4c2, 0x792c, 0x41d2, { 0xbe, 0x5f, 0xdd, 0x91, 0x53, 0x8c, 0x31, 0x20 } };

        // {57E0DB57-2588-473e-B08D-67ABFF514C28}
        static const EVOLUTION_IID IID_WriteSound =
        { 0x57e0db57, 0x2588, 0x473e, { 0xb0, 0x8d, 0x67, 0xab, 0xff, 0x51, 0x4c, 0x28 } };


        // {506E8C7C-FFE7-46b8-9941-7B5833D0B9A1}
        static const EVOLUTION_IID IID_Sound3D =
        { 0x506e8c7c, 0xffe7, 0x46b8, { 0x99, 0x41, 0x7b, 0x58, 0x33, 0xd0, 0xb9, 0xa1 } };

        // {13083C05-6FD0-40ae-96BB-4016A5D19FB5}
        static const EVOLUTION_IID IID_StreamSound =
        { 0x13083c05, 0x6fd0, 0x40ae, { 0x96, 0xbb, 0x40, 0x16, 0xa5, 0xd1, 0x9f, 0xb5 } };

        // {BFB21937-F6C9-4688-A91E-614A2882B369}
        static const EVOLUTION_IID IID_StreamSoundManager =
        { 0xbfb21937, 0xf6c9, 0x4688, { 0xa9, 0x1e, 0x61, 0x4a, 0x28, 0x82, 0xb3, 0x69 } };

        // {DC9E3FD1-47F5-43ea-9EF5-14AFA4E74696}
        static const EVOLUTION_IID IID_SoundFactory =
        { 0xdc9e3fd1, 0x47f5, 0x43ea, { 0x9e, 0xf5, 0x14, 0xaf, 0xa4, 0xe7, 0x46, 0x96 } };

    }
}

#endif //!__EVOLUTION_SOUND_INTERFACE_H__
