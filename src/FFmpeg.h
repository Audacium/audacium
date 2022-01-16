/**********************************************************************

Audacium: A Digital Audio Editor

FFmpeg.h

******************************************************************//**

Describes shared object that is used to access FFmpeg libraries.

*//*******************************************************************/

#if !defined(__AUDACITY_FFMPEG__)
#define __AUDACITY_FFMPEG__

#include "widgets/wxPanelWrapper.h" // to inherit

#if defined(__WXMSW__)
#include <wx/msw/registry.h> // for wxRegKey
#endif

// TODO: Determine whether the libav* headers come from the FFmpeg or libav
// project and set IS_FFMPEG_PROJECT depending on it.
#define IS_FFMPEG_PROJECT 1

/* FFmpeg is written in C99. It uses many types from stdint.h. Because we are
 * compiling this using a C++ compiler we have to put it in extern "C".
 * __STDC_CONSTANT_MACROS is defined to make <stdint.h> behave like it
 * is actually being compiled with a C99 compiler.
 *
 * The symptoms are that INT64_C is not a valid type, which tends to break
 * somewhere down in the implementations using this file */

#if defined(USE_FFMPEG)
extern "C"
{
   // Include errno.h before the ffmpeg includes since they depend on
   // knowing the value of EINVAL...see bottom of avcodec.h.  Not doing
   // so will produce positive error returns when they should be < 0.
   #include <errno.h>

   #include <libavcodec/avcodec.h>
   #include <libavformat/avformat.h>
   #include <libavutil/error.h>
   #include <libavutil/fifo.h>
   #include <libavutil/mathematics.h>
   #include <libavutil/channel_layout.h>

   #if defined(DISABLE_DYNAMIC_LOADING_FFMPEG)
      #if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55, 45, 101)
      #define av_frame_alloc avcodec_alloc_frame
      #define av_frame_free avcodec_free_frame
      #endif

      #if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(54, 59, 100)
      inline void avcodec_free_frame(AVFrame **frame) {
         av_free(*frame);
      }
      #endif

      #if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(54, 51, 100)
      #define AVCodecID CodecID
      #define AV_CODEC_ID_AAC CODEC_ID_AAC
      #define AV_CODEC_ID_AC CODEC_ID_AC
      #define AV_CODEC_ID_AC3 CODEC_ID_AC3
      #define AV_CODEC_ID_ADPCM CODEC_ID_ADPCM
      #define AV_CODEC_ID_ADPCM_CT CODEC_ID_ADPCM_CT
      #define AV_CODEC_ID_ADPCM_G726 CODEC_ID_ADPCM_G726
      #define AV_CODEC_ID_ADPCM_IMA_QT CODEC_ID_ADPCM_IMA_QT
      #define AV_CODEC_ID_ADPCM_IMA_WAV CODEC_ID_ADPCM_IMA_WAV
      #define AV_CODEC_ID_ADPCM_MS CODEC_ID_ADPCM_MS
      #define AV_CODEC_ID_ADPCM_SWF CODEC_ID_ADPCM_SWF
      #define AV_CODEC_ID_ADPCM_YAMAHA CODEC_ID_ADPCM_YAMAHA
      #define AV_CODEC_ID_ALAC CODEC_ID_ALAC
      #define AV_CODEC_ID_AMR CODEC_ID_AMR
      #define AV_CODEC_ID_AMR_NB CODEC_ID_AMR_NB
      #define AV_CODEC_ID_AMR_WB CODEC_ID_AMR_WB
      #define AV_CODEC_ID_ATRAC CODEC_ID_ATRAC
      #define AV_CODEC_ID_ATRAC3 CODEC_ID_ATRAC3
      #define AV_CODEC_ID_DTS CODEC_ID_DTS
      #define AV_CODEC_ID_DVAUDIO CODEC_ID_DVAUDIO
      #define AV_CODEC_ID_FLAC CODEC_ID_FLAC
      #define AV_CODEC_ID_GSM CODEC_ID_GSM
      #define AV_CODEC_ID_GSM_MS CODEC_ID_GSM_MS
      #define AV_CODEC_ID_IMC CODEC_ID_IMC
      #define AV_CODEC_ID_MACE CODEC_ID_MACE
      #define AV_CODEC_ID_MACE3 CODEC_ID_MACE3
      #define AV_CODEC_ID_MACE6 CODEC_ID_MACE6
      #define AV_CODEC_ID_MP CODEC_ID_MP
      #define AV_CODEC_ID_MP2 CODEC_ID_MP2
      #define AV_CODEC_ID_MP3 CODEC_ID_MP3
      #define AV_CODEC_ID_NELLYMOSER CODEC_ID_NELLYMOSER
      #define AV_CODEC_ID_NONE CODEC_ID_NONE
      #define AV_CODEC_ID_PCM CODEC_ID_PCM
      #define AV_CODEC_ID_PCM_ALAW CODEC_ID_PCM_ALAW
      #define AV_CODEC_ID_PCM_MULAW CODEC_ID_PCM_MULAW
      #define AV_CODEC_ID_PCM_S16BE CODEC_ID_PCM_S16BE
      #define AV_CODEC_ID_PCM_S16LE CODEC_ID_PCM_S16LE
      #define AV_CODEC_ID_PCM_S24BE CODEC_ID_PCM_S24BE
      #define AV_CODEC_ID_PCM_S24LE CODEC_ID_PCM_S24LE
      #define AV_CODEC_ID_PCM_S32BE CODEC_ID_PCM_S32BE
      #define AV_CODEC_ID_PCM_S32LE CODEC_ID_PCM_S32LE
      #define AV_CODEC_ID_PCM_S8 CODEC_ID_PCM_S8
      #define AV_CODEC_ID_PCM_U8 CODEC_ID_PCM_U8
      #define AV_CODEC_ID_QCELP CODEC_ID_QCELP
      #define AV_CODEC_ID_QDM CODEC_ID_QDM
      #define AV_CODEC_ID_QDM2 CODEC_ID_QDM2
      #define AV_CODEC_ID_ROQ CODEC_ID_ROQ
      #define AV_CODEC_ID_ROQ_DPCM CODEC_ID_ROQ_DPCM
      #define AV_CODEC_ID_SONIC CODEC_ID_SONIC
      #define AV_CODEC_ID_SONIC_LS CODEC_ID_SONIC_LS
      #define AV_CODEC_ID_TRUESPEECH CODEC_ID_TRUESPEECH
      #define AV_CODEC_ID_VORBIS CODEC_ID_VORBIS
      #define AV_CODEC_ID_VOXWARE CODEC_ID_VOXWARE
      #define AV_CODEC_ID_WMAPRO CODEC_ID_WMAPRO
      #define AV_CODEC_ID_WMAV CODEC_ID_WMAV
      #define AV_CODEC_ID_WMAV1 CODEC_ID_WMAV1
      #define AV_CODEC_ID_WMAV2 CODEC_ID_WMAV2
      #define AV_CODEC_ID_WMAVOICE CODEC_ID_WMAVOICE
      #endif

      #if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(54, 8, 100)
      inline bool av_codec_is_encoder(AVCodec *codec) {
         return codec != NULL && (codec->encode != NULL || codec->encode2 != NULL);
      }
      #endif
   #endif

   #if LIBAVCODEC_VERSION_MAJOR < 58
      #ifndef AV_CODEC_FLAG_QSCALE
         #define AV_CODEC_FLAG_QSCALE CODEC_FLAG_QSCALE
      #endif
      #ifndef AV_CODEC_FLAG_GLOBAL_HEADER
         #define AV_CODEC_FLAG_GLOBAL_HEADER CODEC_FLAG_GLOBAL_HEADER
      #endif
      #ifndef AV_CODEC_CAP_SMALL_LAST_FRAME
         #define AV_CODEC_CAP_SMALL_LAST_FRAME CODEC_CAP_SMALL_LAST_FRAME
      #endif
   #endif

}
#endif

/* rather earlier than normal, but pulls in config*.h and other program stuff
 * we need for the next bit */
#include "ShuttleGui.h"
#include "Prefs.h"

#include "Identifier.h"

// if you needed them, any other audacity header files would go here

/// Callback function to catch FFmpeg log messages.
void av_log_wx_callback(void* ptr, int level, const char* fmt, va_list vl);

/* from here on in, this stuff only applies when ffmpeg is available */
#if defined(USE_FFMPEG)

//----------------------------------------------------------------------------
// Attempt to load and enable/disable FFmpeg at startup
//----------------------------------------------------------------------------
void FFmpegStartup();

// This object allows access to the AVFormatContext,
// and its destructor cleans up memory and file handles
struct FFmpegContext {
   FFmpegContext() {}
   ~FFmpegContext();

   AVIOContext *pb{};
   AVFormatContext *ic_ptr{};
};

int ufile_fopen(AVIOContext **s, const FilePath & name, int flags);
int ufile_fopen_input(std::unique_ptr<FFmpegContext> &context_ptr, FilePath & name);
int ufile_close(AVIOContext *pb);

struct streamContext;
// common utility functions
// utility calls that are shared with ImportFFmpeg
streamContext *import_ffmpeg_read_next_frame(AVFormatContext* formatContext,
                                             streamContext** streams,
                                             unsigned int numStreams);

int import_ffmpeg_decode_frame(streamContext *sc, bool flushing);

#endif

// Attach some C++ lifetime management to the struct, which owns some memory resources.
struct AVPacketEx : public AVPacket
{
   AVPacketEx()
   {
      av_init_packet(this);
      data = nullptr;
      size = 0;
   }
   AVPacketEx(const AVPacketEx &) PROHIBITED;
   AVPacketEx& operator= (const AVPacketEx&) PROHIBITED;
   AVPacketEx(AVPacketEx &&that)
   {
      steal(std::move(that));
   }
   AVPacketEx &operator= (AVPacketEx &&that)
   {
      if (this != &that) {
         reset();
         steal(std::move(that));
      }
      return *this;
   }

   ~AVPacketEx ()
   {
      reset();
   }

   void reset()
   {
      // This does not deallocate the pointer, but it frees side data.
      av_packet_unref(this);
   }

private:
   void steal(AVPacketEx &&that)
   {
      memcpy(this, &that, sizeof(that));
      av_init_packet(&that);
      that.data = nullptr;
      that.size = 0;
   }
};

// utilities for RAII:

// Deleter adaptor for functions like av_free that take a pointer

/// \brief AV_Deleter is part of FFmpeg support.  It's used with the RAII
/// idiom.
template<typename T, typename R, R(*Fn)(T*)> struct AV_Deleter {
   inline R operator() (T* p) const
   {
      R result{};
      if (p)
         result = Fn(p);
      return result;
   }
};

// Specialization of previous for void return
template<typename T, void(*Fn)(T*)>
struct AV_Deleter<T, void, Fn> {
   inline void operator() (T* p) const
   {
      if (p)
         Fn(p);
   }
};

// Deleter adaptor for functions like av_freep that take a pointer to a pointer
template<typename T, typename R, R(*Fn)(T**)> struct AV_Deleterp {
   inline void operator() (T* p) const
   {
      if (p)
         Fn(&p);
   }
};

using AVFrameHolder = std::unique_ptr<
   AVFrame, AV_Deleterp<AVFrame, void, av_frame_free>
>;
using AVFifoBufferHolder = std::unique_ptr<
   AVFifoBuffer, AV_Deleter<AVFifoBuffer, void, av_fifo_free>
>;
using AVFormatContextHolder = std::unique_ptr<
   AVFormatContext, AV_Deleter<AVFormatContext, void, avformat_free_context>
>;
using AVCodecContextHolder = std::unique_ptr<
    AVCodecContext, AV_Deleter<AVCodecContext, int, avcodec_close>
>;
using AVDictionaryCleanup = std::unique_ptr<
   AVDictionary*, AV_Deleter<AVDictionary*, void, av_dict_free>
>;

/// \brief FFmpeg structure to hold a file pointer and provide a return 
/// value when closing the file.
struct UFileHolder : public std::unique_ptr<
   AVIOContext, ::AV_Deleter<AVIOContext, int, ufile_close>
>
{
   UFileHolder() = default;
   UFileHolder( UFileHolder &&that )
   : std::unique_ptr< AVIOContext, ::AV_Deleter<AVIOContext, int, ufile_close> >(
        std::move(that) )
   {
   }

   // Close explicitly, not ignoring return values.
   int close()
   {
      auto result = get_deleter() ( get() );
      release();
      return result;
   }
};

template<typename T> using AVMallocHolder = std::unique_ptr<
   T, AV_Deleter<void, void, av_free>
>;

struct streamContext
{
   bool                 m_use{};                           // TRUE = this stream will be loaded into Audacity
   AVStream            *m_stream{};                        // an AVStream *
   AVCodecContext      *m_codecCtx{};                      // pointer to m_stream->codecpar

   Optional<AVPacketEx>    m_pkt;                           // the last AVPacket we read for this stream
   uint8_t             *m_pktDataPtr{};                    // pointer into m_pkt.data
   int                  m_pktRemainingSiz{};

   int64_t              m_pts{};                           // the current presentation time of the input stream
   int64_t              m_ptsOffset{};                     // packets associated with stream are relative to this

   int                  m_frameValid{};                    // is m_decodedVideoFrame/m_decodedAudioSamples valid?
   AVMallocHolder<uint8_t> m_decodedAudioSamples;           // decoded audio samples stored here
   unsigned int         m_decodedAudioSamplesSiz{};        // current size of m_decodedAudioSamples
   size_t               m_decodedAudioSamplesValidSiz{};   // # valid bytes in m_decodedAudioSamples
   int                  m_initialchannels{};               // number of channels allocated when we begin the importing. Assumes that number of channels doesn't change on the fly.

   size_t               m_samplesize{};                    // input sample size in bytes
   AVSampleFormat       m_samplefmt{ AV_SAMPLE_FMT_NONE  }; // input sample format

   size_t               m_osamplesize{};                   // output sample size in bytes
   sampleFormat         m_osamplefmt{ floatSample };                    // output sample format

   streamContext() { memset(this, 0, sizeof(*this)); }
   ~streamContext()
   {
   }
};

using Scs = ArrayOf<std::unique_ptr<streamContext>>;
using ScsPtr = std::shared_ptr<Scs>;

#endif // __AUDACITY_FFMPEG__

