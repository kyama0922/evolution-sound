#ifndef __EVOLUTION_SOUND_INTERFACE_H__
#define __EVOLUTION_SOUND_INTERFACE_H__

#include <evolution.h>
#include <evolution_type.h>

#include <Windows.h>

namespace EVOLUTION{
    namespace SOUND{

        namespace CONFIG{
            static const s32 Num_Stream_Sound_Queue = 32; 

            //一時ファイル作成パス
            static const c8* Sound_Temp_Dir = "soundtemp/";

            //サウンドストリーム時間
            static const u32 Stream_Sec = 2;

            //サウンドストリーム時間
            static const u32 Stream_Sleep_millisecond = Stream_Sec * 1000 / 4;
        }

        //音楽ファイル
        class ISoundFileLoader :public IUnknown{
        public:
            virtual const u8* GetSoundBuffer()const = 0;
            virtual u32 GetSoundBufferSize()const = 0;
            virtual const WAVEFORMATEX& GetSoundFormat()const = 0;
        };

        //サウンドクラス
        class ISound :public IUnknown{
			EVOLUTION_NOT_DESTRUCTOR(ISound);
		public:
			virtual ISound* Copy() = 0;//サウンドのコピー
			virtual void Play(bool loop) = 0;//再生
			virtual void Pause() = 0;//一時停止
			virtual void Stop() = 0;//停止
			virtual void SetPan(s32 pan) = 0;//パン
			virtual void SetVolume(s32 volume) = 0;//ボリューム-10000 ~ 0
			virtual void SetPlayPos(u32 second) = 0;//再生タイム秒
            virtual void SetFrequency(f32 frequency) = 0;//周波数
			virtual s32 GetVolume() = 0;//ボリューム取得
            virtual f32 GetFrequency() = 0;	//周波数
			virtual u32 GetPlayPosSecond() = 0;//再生タイム秒取得
			virtual u32 GetPlayPosMillSecond() = 0;//再生タイムミリ秒取得
			virtual u32 GetPlayPosBuffer() = 0;//バッファ再生場所取得
			virtual u32 GetTime() = 0;
			virtual bool IsPlay() = 0;
		};

        //拡張サウンド
        class IWriteSound :public ISound{
            EVOLUTION_NOT_DESTRUCTOR(IWriteSound);
        public:
            virtual u32 GetBufferSize()const = 0;//バッファサイズの取得
            virtual void WriteBuffer(u32 write_position ,void* buffer , s32 buffer_size) = 0;//バッファを書き込む
        };

        class ISound3D : public IUnknown{
			EVOLUTION_NOT_DESTRUCTOR(ISound3D);
		public:
			virtual ISound3D* Copy() = 0;//サウンドのコピー
			virtual void Play(bool loop) = 0;//再生
			virtual void Pause() = 0;//一時停止
			virtual void Stop() = 0;//停止
            virtual void SetPos(f32 x, f32 y, f32 z) = 0;//3D空間上の場所に配置
			virtual void SetPlayPos(u32 second) = 0;//再生タイム秒
            virtual void SetFrequency(f32 frequency) = 0;//周波数設定
			virtual u32 GetPlayPos() = 0;//再生タイム秒取得
            virtual f32 GetFrequency() = 0;//周波数取得
			virtual u32 GetTime() = 0;
			virtual bool IsPlay() = 0;
		};

        class IStreamSound : public IUnknown{
			EVOLUTION_NOT_DESTRUCTOR(IStreamSound);
		public:
			virtual IStreamSound* Copy() = 0;//サウンドのコピー
			virtual void Play(bool loop) = 0;//再生
			virtual void Pause() = 0;//一時停止
			virtual void Stop() = 0;//停止
			virtual void SetPan(s32 pan) = 0;//パン
			virtual void SetVolume(s32 volume) = 0;//ボリューム-10000 ~ 0
			virtual void SetPlayPos(u32 second) = 0;//再生タイム秒
            virtual void SetFrequency(f32 frequency) = 0;//周波数
			virtual s32 GetVolume() = 0;//ボリューム取得
            virtual f32 GetFrequency() = 0;	//周波数
			virtual u32 GetPlayPosSecond() = 0;//再生タイム秒取得
			virtual u32 GetPlayPosMillSecond() = 0;//再生タイムミリ秒取得

			virtual u32 GetTime() = 0;
			virtual bool IsPlay() = 0;
		};

        class IStreamSoundManager :public IUnknown{
			EVOLUTION_NOT_DESTRUCTOR(IStreamSoundManager);
		public:
			virtual void CreateStreamSound(IStreamSound** pp_stream_sound, ISoundFileLoader* sound_file) = 0;
		};

        class ISoundFactory : public IUnknown{
			EVOLUTION_NOT_DESTRUCTOR(ISoundFactory);
		public:
			virtual void CreateStreamSoundManager(IStreamSoundManager** pp_stream_sound_manager) = 0;
            virtual void CreateSound(ISound** pp_sound, ISoundFileLoader* sound_file) = 0;
            virtual void CreateWriteSound(IWriteSound** pp_write_sound, u32 buffer_size, const WAVEFORMATEX& sound_format) = 0;
			virtual void CreateSound3D(ISound3D** pp_sound3d, ISoundFileLoader* sound_file) = 0;
			virtual void CreateSoundFile(ISoundFileLoader** p_sound_file, const char* file_name) = 0;
		};

	}

    namespace FUNCTION{
        extern void CreateSoundFactory(SOUND::ISoundFactory** pp_sound_factory, void *hwnd);
    }

    namespace EVOLUTION_GUID{

        // {470E764B-8D55-42a4-B49E-E87087116546}
        static const EVOLUTION_IID IID_ISoundFileLoader =
        { 0x470e764b, 0x8d55, 0x42a4, { 0xb4, 0x9e, 0xe8, 0x70, 0x87, 0x11, 0x65, 0x46 } };

        // {4FF27B9E-2125-4621-A93B-39B5DC0FCA21}
        static const EVOLUTION_IID IID_ISound =
        { 0x4ff27b9e, 0x2125, 0x4621, { 0xa9, 0x3b, 0x39, 0xb5, 0xdc, 0xf, 0xca, 0x21 } };

        // {0BBD5B25-9C3A-4282-BC6A-780419FD94C2}
        static const EVOLUTION_IID IID_IWriteSound =
        { 0xbbd5b25, 0x9c3a, 0x4282, { 0xbc, 0x6a, 0x78, 0x4, 0x19, 0xfd, 0x94, 0xc2 } };

        // {D04DE498-9CDD-4cd6-A682-B39FCB9B0510}
        static const EVOLUTION_IID IID_ISound3D =
        { 0xd04de498, 0x9cdd, 0x4cd6, { 0xa6, 0x82, 0xb3, 0x9f, 0xcb, 0x9b, 0x5, 0x10 } };

        // {641F755A-9E09-479c-A240-80003192C54D}
        static const EVOLUTION_IID IID_IStreamSound =
        { 0x641f755a, 0x9e09, 0x479c, { 0xa2, 0x40, 0x80, 0x0, 0x31, 0x92, 0xc5, 0x4d } };

        // {3B0C756B-F38C-4eda-B56F-CAC5A1EC9AAA}
        static const EVOLUTION_IID IID_IStreamSoundManager =
        { 0x3b0c756b, 0xf38c, 0x4eda, { 0xb5, 0x6f, 0xca, 0xc5, 0xa1, 0xec, 0x9a, 0xaa } };

        // {D21730C6-A767-454b-B309-1C83637ECBE6}
        static const EVOLUTION_IID IID_ISoundFactory =
        { 0xd21730c6, 0xa767, 0x454b, { 0xb3, 0x9, 0x1c, 0x83, 0x63, 0x7e, 0xcb, 0xe6 } };

    }
}

#endif // !__SOUND_INTERFACE_H__