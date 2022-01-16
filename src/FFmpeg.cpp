/**********************************************************************

Audacium: A Digital Audio Editor

FFmpeg.cpp

Audacity(R) is copyright (c) 1999-2009 Audacity Team.
License: GPL v2.  See License.txt.

******************************************************************//**

\class FFmpegLibs
\brief Class used to dynamically load FFmpeg libraries

*//*******************************************************************/

#if defined(USE_FFMPEG)

// Store function pointers here when including FFmpeg.h
#define DEFINE_FFMPEG_POINTERS


#include "FFmpeg.h"

#include "FileNames.h"
#include "widgets/HelpSystem.h"
#include "widgets/AudacityMessageBox.h"

#include <wx/checkbox.h>
#include <wx/dynlib.h>
#include <wx/file.h>
#include <wx/log.h>
#include <wx/textctrl.h>

void FFmpegStartup()
{
   gPrefs->Write(wxT("/FFmpeg/Enabled"), true);
   gPrefs->Flush();
}

void av_log_wx_callback(void* ptr, int level, const char* fmt, va_list vl)
{
   //Most of this stuff is taken from FFmpeg tutorials and FFmpeg itself
   int av_log_level = AV_LOG_INFO;
   AVClass* avc = ptr ? *(AVClass**)ptr : NULL;
   if (level > av_log_level)
      return;
   wxString printstring(wxT(""));

   if (avc) {
      printstring.Append(wxString::Format(wxT("[%s @ %p] "), wxString::FromUTF8(avc->item_name(ptr)), avc));
   }

   wxString frm(fmt,wxConvLibc);

   printstring.Append(wxString::FormatV(frm,vl));
   wxString cpt;
   switch (level)
   {
   case 0: cpt = wxT("Error"); break;
   case 1: cpt = wxT("Info"); break;
   case 2: cpt = wxT("Debug"); break;
   default: cpt = wxT("Log"); break;
   }
   wxLogDebug(wxT("%s: %s"),cpt,printstring);
}

//======================= Unicode aware uri protocol for FFmpeg
// Code inspired from ffmpeg-users mailing list sample

static int ufile_read(void *opaque, uint8_t *buf, int size)
{
   int ret = (int)((wxFile *) opaque)->Read(buf, size);
   return ret;
}

static int ufile_write(void *opaque, uint8_t *buf, int size)
{
   auto bytes = (int) ((wxFile *) opaque)->Write(buf, size);
   if (bytes != size)
      return -ENOSPC;
   return bytes;
}

static int64_t ufile_seek(void *opaque, int64_t pos, int whence)
{
   wxSeekMode mode = wxFromStart;

#if !defined(AVSEEK_FORCE)
#define AVSEEK_FORCE 0
#endif

   switch (whence & ~AVSEEK_FORCE)
   {
   case (SEEK_SET):
      mode = wxFromStart;
      break;
   case (SEEK_CUR):
      mode = wxFromCurrent;
      break;
   case (SEEK_END):
      mode = wxFromEnd;
      break;
   case (AVSEEK_SIZE):
      return ((wxFile *) opaque)->Length();
   }

   return ((wxFile *) opaque)->Seek(pos, mode);
}

int ufile_close(AVIOContext *pb)
{
   std::unique_ptr<wxFile> f{ (wxFile *)pb->opaque };

   bool success = true;
   if (f) {
      if (pb->write_flag) {
         success = f->Flush();
      }
      if (success) {
         success = f->Close();
      }
      pb->opaque = nullptr;
   }

   // We're not certain that a close error is for want of space, but we'll
   // guess that
   return success ? 0 : -ENOSPC;

   // Implicitly destroy the wxFile object here
}

// Open a file with a (possibly) Unicode filename
int ufile_fopen(AVIOContext **s, const FilePath & name, int flags)
{
   wxFile::OpenMode mode;

   auto f = std::make_unique<wxFile>();
   if (!f) {
      return -ENOMEM;
   }

   if (flags == (AVIO_FLAG_READ | AVIO_FLAG_WRITE)) {
      return -EINVAL;
   } else if (flags == AVIO_FLAG_WRITE) {
      mode = wxFile::write;
   } else {
      mode = wxFile::read;
   }

   if (!f->Open(name, mode)) {
      return -ENOENT;
   }

   *s = avio_alloc_context((unsigned char*)av_malloc(32768), 32768,
                           flags & AVIO_FLAG_WRITE,
                           /*opaque*/f.get(),
                           ufile_read,
                           ufile_write,
                           ufile_seek);
   if (!*s) {
      return -ENOMEM;
   }

   f.release(); // s owns the file object now

   return 0;
}


// Detect type of input file and open it if recognized. Routine
// based on the av_open_input_file() libavformat function.
int ufile_fopen_input(std::unique_ptr<FFmpegContext> &context_ptr, FilePath & name)
{
   context_ptr.reset();
   auto context = std::make_unique<FFmpegContext>();

   wxFileName ff{ name };
   wxCharBuffer fname;
   const char *filename;
   int err;

   fname = ff.GetFullName().mb_str();
   filename = (const char *) fname;

   // Open the file to prepare for probing
   if ((err = ufile_fopen(&context->pb, name, AVIO_FLAG_READ)) < 0) {
      goto fail;
   }

   context->ic_ptr = avformat_alloc_context();
   context->ic_ptr->pb = context->pb;

   // And finally, attempt to associate an input stream with the file
   err = avformat_open_input(&context->ic_ptr, filename, NULL, NULL);
   if (err) {
      goto fail;
   }

   // success
   context_ptr = std::move(context);
   return 0;

fail:

   return err;
}

FFmpegContext::~FFmpegContext()
{
    if (ic_ptr)
        avformat_close_input(&ic_ptr);
    av_log_set_callback(av_log_default_callback);

    if (pb)
    {
        ufile_close(pb);
        av_free(pb->buffer);
        av_free(pb);
    }
}

streamContext *import_ffmpeg_read_next_frame(AVFormatContext* formatContext,
                                             streamContext** streams,
                                             unsigned int numStreams)
{
   streamContext *sc = NULL;
   AVPacketEx pkt;

   if (av_read_frame(formatContext, &pkt) < 0)
   {
      return NULL;
   }

   // Find a stream to which this frame belongs
   for (unsigned int i = 0; i < numStreams; i++)
   {
      if (streams[i]->m_stream->index == pkt.stream_index)
         sc = streams[i];
   }

   // Off-stream packet. Don't panic, just skip it.
   // When not all streams are selected for import this will happen very often.
   if (sc == NULL)
   {
      return (streamContext*)1;
   }

   // Copy the frame to the stream context
   sc->m_pkt.emplace(std::move(pkt));

   sc->m_pktDataPtr = sc->m_pkt->data;
   sc->m_pktRemainingSiz = sc->m_pkt->size;

   return sc;
}

int import_ffmpeg_decode_frame(streamContext *sc, bool flushing)
{
   int      nBytesDecoded;
   wxUint8 *pDecode = sc->m_pktDataPtr;
   int      nDecodeSiz = sc->m_pktRemainingSiz;

   sc->m_frameValid = 0;

   if (flushing)
   {
      // If we're flushing the decoders we don't actually have any NEW data to decode.
      pDecode = NULL;
      nDecodeSiz = 0;
   }
   else
   {
      if (!sc->m_pkt || (sc->m_pktRemainingSiz <= 0))
      {
         //No more data
         return -1;
      }
   }

   AVPacketEx avpkt;
   avpkt.data = pDecode;
   avpkt.size = nDecodeSiz;

   AVFrameHolder frame{ av_frame_alloc() };
   int got_output = 0;

   nBytesDecoded = avcodec_receive_frame(sc->m_codecCtx, frame.get());
   if (nBytesDecoded == 0)
       got_output = true;
   if (nBytesDecoded == AVERROR(EAGAIN))
       nBytesDecoded = 0;
   if (nBytesDecoded == 0)
       nBytesDecoded = avcodec_send_packet(sc->m_codecCtx, &avpkt);
   if (nBytesDecoded == AVERROR(EAGAIN))
       nBytesDecoded = 0;
   else if (nBytesDecoded < 0)
   {
       // Decoding failed. Don't stop.
       return -1;
   }
   else
       nBytesDecoded = avpkt.size;

   sc->m_samplefmt = sc->m_codecCtx->sample_fmt;
   sc->m_samplesize = static_cast<size_t>(av_get_bytes_per_sample(sc->m_samplefmt));

   int channels = sc->m_codecCtx->channels;
   auto newsize = sc->m_samplesize * frame->nb_samples * channels;
   sc->m_decodedAudioSamplesValidSiz = newsize;
   // Reallocate the audio sample buffer if it's smaller than the frame size.
   if (newsize > sc->m_decodedAudioSamplesSiz )
   {
      // Reallocate a bigger buffer.  But av_realloc is NOT compatible with the returns of av_malloc!
      // So do this:
      sc->m_decodedAudioSamples.reset(static_cast<uint8_t *>(av_malloc(newsize)));
      sc->m_decodedAudioSamplesSiz = newsize;
      if (!sc->m_decodedAudioSamples)
      {
         //Can't allocate bytes
         return -1;
      }
   }
   if (frame->data[1]) {
      for (int i = 0; i<frame->nb_samples; i++) {
         for (int ch = 0; ch<channels; ch++) {
            memcpy(sc->m_decodedAudioSamples.get() + sc->m_samplesize * (ch + channels*i),
                  frame->extended_data[ch] + sc->m_samplesize*i,
                  sc->m_samplesize);
         }
      }
   } else {
      memcpy(sc->m_decodedAudioSamples.get(), frame->data[0], newsize);
   }

   // We may not have read all of the data from this packet. If so, the user can call again.
   // Whether or not they do depends on if m_pktRemainingSiz == 0 (they can check).
   sc->m_pktDataPtr += nBytesDecoded;
   sc->m_pktRemainingSiz -= nBytesDecoded;

   // At this point it's normally safe to assume that we've read some samples. However, the MPEG
   // audio decoder is broken. If this is the case then we just return with m_frameValid == 0
   // but m_pktRemainingSiz perhaps != 0, so the user can call again.
   if (sc->m_decodedAudioSamplesValidSiz > 0)
   {
      sc->m_frameValid = 1;
   }
   return 0;
}

#endif //USE_FFMPEG
