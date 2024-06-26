/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2003                *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

  function: example SDL player application; plays Ogg Theora files (with
            optional Vorbis audio second stream)
  last mod: $Id: theora.c,v 1.4 2003/11/06 02:16:33 jon Exp $

 ********************************************************************/

/* far more complex than most Ogg 'example' programs.  The complexity
   of maintaining A/V sync is pretty much unavoidable.  It's necessary
   to actually have audio/video playback to make the hard audio clock
   sync actually work.  If there's audio playback, there might as well
   be simple video playback as well...

   A simple 'demux and write back streams' would have been easier,
   it's true. */

#define _GNU_SOURCE
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include "theora/theora.h"
#include "vorbis/codec.h"

#include "core.h"

#include <SDL/SDL.h>

/* yes, this makes us OSS-specific for now. None of SDL, libao, libao2
   give us any way to determine hardware timing, and since the
   hard/kernel buffer is going to be most of or > a second, that's
   just a little bit important */
#if defined(__FreeBSD__)
#include <machine/soundcard.h>
#define AUDIO_DEVICE "/dev/audio"
#elif defined(__NetBSD__) || defined(__OpenBSD__)
#include <soundcard.h>
#define AUDIO_DEVICE "/dev/audio"
#else
#include <sys/soundcard.h>
#define AUDIO_DEVICE "/dev/dsp"
#endif
#include <sys/ioctl.h>

/* Helper; just grab some more compressed bitstream and sync it for
   page extraction */
int buffer_data(file_t *in, ogg_sync_state *oy)
{
  char *buffer=ogg_sync_buffer(oy,4096);
  int bytes=in->read(buffer,1,4096,in);
  ogg_sync_wrote(oy,bytes);
  return(bytes);
}

/* never forget that globals are a one-way ticket to Hell */
/* Ogg and codec state for demux/decode */
static ogg_sync_state   oy;
static ogg_page         og;
static ogg_stream_state vo;
static ogg_stream_state to;
static theora_info      ti;
static theora_comment   tc;
static theora_state     td;
static vorbis_info      vi;
static vorbis_dsp_state vd;
static vorbis_block     vb;
static vorbis_comment   vc;
static ogg_packet 		op;
static bitmap_t			*bitmap = NULL;

static int              theora_p=0;
static int              vorbis_p=0;
static int              stateflag=0;

/* SDL Video playback structures */
static SDL_Surface *screen;
static SDL_Overlay *yuv_overlay;
static SDL_Rect rect;

/* single frame video buffering */
static int          videobuf_ready=0;
static ogg_int64_t  videobuf_granulepos=-1;
static double       videobuf_time=0;

static file_t 		*infile;

/* single audio fragment audio buffering */
static int          audiobuf_fill=0;
static int          audiobuf_ready=0;
static ogg_int16_t *audiobuf;
static ogg_int64_t  audiobuf_granulepos=0; /* time position of last sample */

/* audio / video synchronization tracking:

Since this will make it to Google at some point and lots of people
search for how to do this, a quick rundown of a practical A/V sync
strategy under Linux [the UNIX where Everything Is Hard].  Naturally,
this works on other platforms using OSS for sound as well.

In OSS, we don't have reliable access to any precise information on
the exact current playback position (that, of course would have been
too easy; the kernel folks like to keep us app people working hard
doing simple things that should have been solved once and abstracted
long ago).  Hopefully ALSA solves this a little better; we'll probably
use that once ALSA is the standard in the stable kernel.

We can't use the system clock for a/v sync because audio is hard
synced to its own clock, and both the system and audio clocks suffer
from wobble, drift, and a lack of accuracy that can be guaranteed to
add a reliable percent or so of error.  After ten seconds, that's
100ms.  We can't drift by half a second every minute.

Although OSS can't generally tell us where the audio playback pointer
is, we do know that if we work in complete audio fragments and keep
the kernel buffer full, a blocking select on the audio buffer will
give us a writable fragment immediately after playback finishes with
it.  We assume at that point that we know the exact number of bytes in
the kernel buffer that have not been played (total fragments minus
one) and calculate clock drift between audio and system then (and only
then).  Damp the sync correction fraction, apply, and walla: A
reliable A/V clock that even works if it's interrupted. */

long         audiofd_totalsize=-1;
int          audiofd_fragsize;      /* read and write only complete fragments
                                       so that SNDCTL_DSP_GETOSPACE is
                                       accurate immediately after a bank
                                       switch */
int          audiofd=-1;
ogg_int64_t  audiofd_timer_calibrate=-1;

static bool open_audio()
{
	audio_buf_info info;
	int format=AFMT_S16_NE; /* host endian */
	int channels=vi.channels;
	int rate=vi.rate;
	int ret;

	audiofd=open(AUDIO_DEVICE,O_RDWR);
	if (audiofd<0)
	{
    	Console_Printf("Could not open audio device " AUDIO_DEVICE ".\n");
    	return false;
	}

	ret=ioctl(audiofd,SNDCTL_DSP_SETFMT,&format);
	if (ret)
	{
    	Console_Printf("Could not set 16 bit host-endian playback\n");
    	return false;
	}

	ret=ioctl(audiofd,SNDCTL_DSP_CHANNELS,&channels);
	if (ret)
	{
    	Console_Printf("Could not set %d channel playback\n",channels);
    	return false;
	}

	ret=ioctl(audiofd,SNDCTL_DSP_SPEED,&rate);
	if (ret)
	{
    	Console_Printf("Could not set %d Hz playback\n",rate);
    	return false;
	}

	ioctl(audiofd,SNDCTL_DSP_GETOSPACE,&info);
	audiofd_fragsize=info.fragsize;
	audiofd_totalsize=info.fragstotal*info.fragsize;

	audiobuf=malloc(audiofd_fragsize);

	return true;
}

static void audio_close(void)
{
  	if(audiofd>-1)
  	{
    	ioctl(audiofd,SNDCTL_DSP_RESET,NULL);
    	close(audiofd);
    	free(audiobuf);
  	}
}

/* call this only immediately after unblocking from a full kernel
   having a newly empty fragment or at the point of DMA restart */
void audio_calibrate_timer(int restart)
{
  	struct timeval tv;
  	ogg_int64_t current_sample;
  	ogg_int64_t new_time;

  	gettimeofday(&tv,0);
  	new_time=tv.tv_sec*1000+tv.tv_usec/1000;

  	if(restart)
  	{
  		current_sample=audiobuf_granulepos-audiobuf_fill/2/vi.channels;
  	}
  	else
  	{
    	current_sample=audiobuf_granulepos-
      		(audiobuf_fill+audiofd_totalsize-audiofd_fragsize)/2/vi.channels;
  	}

  	new_time-=1000*current_sample/vi.rate;

  	audiofd_timer_calibrate=new_time;
}

/* get relative time since beginning playback, compensating for A/V
   drift */
double get_time()
{
  	static ogg_int64_t last=0;
  	//static ogg_int64_t up=0;
  	ogg_int64_t now;
  	struct timeval tv;

  	gettimeofday(&tv,0);
  	now=tv.tv_sec*1000+tv.tv_usec/1000;

  	if (audiofd_timer_calibrate==-1)
		audiofd_timer_calibrate=last=now;

  	if (audiofd<0)
  	{
  		/* no audio timer to worry about, we can just use the system clock */
    	/* only one complication: If the process is suspended, we should
       	   reset timing to account for the gap in play time.  Do it the
       	   easy/hack way */
    	if (now-last>1000)
			audiofd_timer_calibrate+=(now-last);
    	last=now;
  	}

  	/*
  	if(now-up>200)
	{
    	double timebase=(now-audiofd_timer_calibrate)*.001;
    	int hundredths=timebase*100-(long)timebase*100;
    	int seconds=(long)timebase%60;
    	int minutes=((long)timebase/60)%60;
    	int hours=(long)timebase/3600;

    	Console_Printf("   Playing: %d:%02d:%02d.%02d                       \r",
    	        hours,minutes,seconds,hundredths);
    	up=now;
  	}
  	*/

  	return (now-audiofd_timer_calibrate)*.001;
}

/* write a fragment to the OSS kernel audio API, but only if we can
   stuff in a whole fragment without blocking */
void audio_write_nonblocking(void)
{
  	if (audiobuf_ready)
  	{
   		audio_buf_info info;
    	long bytes;

    	ioctl(audiofd,SNDCTL_DSP_GETOSPACE,&info);
    	bytes=info.bytes;
    	if (bytes>=audiofd_fragsize)
		{
      		if (bytes==audiofd_totalsize)
				audio_calibrate_timer(1);

      		while (1)
			{
       			bytes = write(audiofd,audiobuf+(audiofd_fragsize-audiobuf_fill), audiofd_fragsize);
				
        		if (bytes>0)
				{
        
          			if (bytes!=audiobuf_fill)
					{
            			/* shouldn't actually be possible... but eh */
            			audiobuf_fill-=bytes;
          			}
					else
					{
            			break;
					}
        		}
      		}

      		audiobuf_fill=0;
      		audiobuf_ready=0;
    	}
  	}
}

static void close_video(void)
{
	SDL_FreeSurface(screen);
}

static bool open_video(void)
{
#ifdef STANDALONE
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
			    Console_Printf( "Unable to init SDL: %s\n", SDL_GetError());
				    exit(1);
					  }

	screen = SDL_SetVideoMode(ti.frame_width, ti.frame_height, 0, SDL_SWSURFACE);
#else
	screen = SDL_CreateRGBSurface(SDL_SWSURFACE, ti.frame_width, ti.frame_height, 32, 0, 0, 0, 0);
#endif
  	if ( screen == NULL ) 
	{
    	Console_Printf( "Unable to set %dx%d video: %s\n",
    	        ti.frame_width,ti.frame_height,SDL_GetError());
    	return false;
  	}

  	yuv_overlay = SDL_CreateYUVOverlay(ti.frame_width, ti.frame_height,
                                     SDL_YV12_OVERLAY,
                                     screen);
  	if ( yuv_overlay == NULL ) 
	{
    	Console_Printf( "SDL: Couldn't create SDL_yuv_overlay: %s\n",
        	    SDL_GetError());
    	return false;
  	}
  	rect.x = 0;
  	rect.y = 0;
  	rect.w = ti.frame_width;
  	rect.h = ti.frame_height;

  	SDL_DisplayYUVOverlay(yuv_overlay, &rect);
	return true;
}

static void video_write(SDL_Surface *screen)
{
  	int i;
  	yuv_buffer yuv;
  	int crop_offset;
  	theora_decode_YUVout(&td,&yuv);

  	/* Lock SDL_yuv_overlay */
  	if ( SDL_MUSTLOCK(screen) ) 
	{
    	if ( SDL_LockSurface(screen) < 0 ) return;
  	}
  	if (SDL_LockYUVOverlay(yuv_overlay) < 0) return;

  	/* let's draw the data (*yuv[3]) on a SDL screen (*screen) */
  	/* deal with border stride */
  	/* reverse u and v for SDL */
  	/* and crop input properly, respecting the encoded frame rect */
  	crop_offset=ti.offset_x+yuv.y_stride*ti.offset_y;
  	for(i=0;i<yuv_overlay->h;i++)
    	memcpy(yuv_overlay->pixels[0]+yuv_overlay->pitches[0]*i,
           yuv.y+crop_offset+yuv.y_stride*i,
           yuv_overlay->w);
  	crop_offset=(ti.offset_x/2)+(yuv.uv_stride)*(ti.offset_y/2);
  	for(i=0;i<yuv_overlay->h/2;i++)
	{
    	memcpy(yuv_overlay->pixels[1]+yuv_overlay->pitches[1]*i,
           yuv.v+crop_offset+yuv.uv_stride*i,
           yuv_overlay->w/2);
    	memcpy(yuv_overlay->pixels[2]+yuv_overlay->pitches[2]*i,
           yuv.u+crop_offset+yuv.uv_stride*i,
           yuv_overlay->w/2);
  	}

  	/* Unlock SDL_yuv_overlay */
  	if ( SDL_MUSTLOCK(screen) ) 
	{
    	SDL_UnlockSurface(screen);
  	}
  	SDL_UnlockYUVOverlay(yuv_overlay);

  	/* Show, baby, show! */
  	SDL_DisplayYUVOverlay(yuv_overlay, &rect);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, screen->w, screen->h, 
							GL_BGRA, GL_UNSIGNED_BYTE, screen->pixels);
}

/* dump the theora (or vorbis) comment header */
static int dump_comments(theora_comment *tc){
  int i, len;
  char *value;

  Console_DPrintf("Encoded by %s\n",tc->vendor);
  if(tc->comments){
    Console_DPrintf("theora comment header:\n");
    for(i=0;i<tc->comments;i++){
      if(tc->user_comments[i]){
        len=tc->comment_lengths[i];
        value=malloc(len+1);
        memcpy(value,tc->user_comments[i],len);
        value[len]='\0';
        Console_DPrintf("\t%s\n", value);
        free(value);
      }
    }
  }
  return(0);
}

/* Report the encoder-specified colorspace for the video, if any.
   We don't actually make use of the information in this example;
   a real player should attempt to perform color correction for
   whatever display device it supports. */
static void report_colorspace(theora_info *ti)
{
    switch(ti->colorspace){
      case not_specified:
        /* nothing to report */
        break;;
      case ITU_Rec_601:
        Console_Printf("  encoder specified ITU Rec 601 color.\n");
        break;;
      case CIE_Rec_709:
        Console_Printf("  encoder specified CIE Rec 709 color.\n");
        break;;
      default:
        Console_Printf("warning: encoder specified unknown colorspace (%d).\n",
            ti->colorspace);
        break;;
    }
}

/* helper: push a page into the appropriate steam */
/* this can be done blindly; a stream won't accept a page
                that doesn't belong to it */
static int queue_page(ogg_page *page){
  if(theora_p)ogg_stream_pagein(&to,&og);
  if(vorbis_p)ogg_stream_pagein(&vo,&og);
  return 0;
}

static int nextPowerOfTwo(int x)
{
	int n;

	//check that it isn't already a power of two
	for (n = 0; n < 32; n++)
	{
		if (x == (1 << n))
			return x;
	}

	//find the highest bit
	n = 0;
	while (x)
	{
		x >>= 1;
		n++;
	}

	return (1 << n);
}

int     Theora_Load(const char *filename, 
				bitmap_t *_bitmap, 
				bool preload)
{
  	infile = File_Open(filename, "rb");
	Console_DPrintf("Attempting to open file %s\n", filename);
  	if (!infile)
		return -1;

  	/* start up Ogg stream synchronization layer */
  	ogg_sync_init(&oy);

  	/* init supporting Vorbis structures needed in header parsing */
 	vorbis_info_init(&vi);
  	vorbis_comment_init(&vc);

  	/* init supporting Theora structures needed in header parsing */
  	theora_comment_init(&tc);
  	theora_info_init(&ti);

  	/* Ogg file open; parse the headers */
  	/* Only interested in Vorbis/Theora streams */
  	while(!stateflag)
	{
   		int ret = buffer_data(infile,&oy);
    	if (ret==0)
			break;
    	
		while (ogg_sync_pageout(&oy,&og)>0)
		{
      		ogg_stream_state test;

      		/* is this a mandated initial header? If not, stop parsing */
      		if(!ogg_page_bos(&og))
			{
        		/* don't leak the page; get it into the appropriate stream */
        		queue_page(&og);
        		stateflag=1;
        		break;
      		}

      		ogg_stream_init(&test,ogg_page_serialno(&og));
      		ogg_stream_pagein(&test,&og);
      		ogg_stream_packetout(&test,&op);

      		/* identify the codec: try theora */
      		if (!theora_p && theora_decode_header(&ti,&tc,&op)>=0)
			{
        		/* it is theora */
        		memcpy(&to,&test,sizeof(test));
        		theora_p=1;
      		}
			else if(!vorbis_p && vorbis_synthesis_headerin(&vi,&vc,&op)>=0)
			{
        		/* it is vorbis */
        		memcpy(&vo,&test,sizeof(test));
        		vorbis_p=1;
      		}
			else
			{
        		/* whatever it is, we don't care about it */
        		ogg_stream_clear(&test);
      		}
    	}
    	/* fall through to non-bos page parsing */
  	}

  	/* we're expecting more header packets. */
  	while ((theora_p && theora_p<3) || (vorbis_p && vorbis_p<3))
	{
    	int ret;

    	/* look for further theora headers */
    	while (theora_p && (theora_p<3) && (ret=ogg_stream_packetout(&to,&op)))
		{
      		if (ret<0)
			{
        		Console_Printf("Error parsing Theora stream headers; corrupt stream?\n");
        		return -1;
     		}
      		if (theora_decode_header(&ti,&tc,&op))
			{
        		Console_Printf("Error parsing Theora stream headers; corrupt stream?\n");
        		return -1;
      		}
      		theora_p++;
      		if (theora_p==3)
				break;
    	}

    	/* look for more vorbis header packets */
    	while(vorbis_p && (vorbis_p<3) && (ret=ogg_stream_packetout(&vo,&op)))
		{
      		if (ret<0)
			{
        		Console_Printf("Error parsing Vorbis stream headers; corrupt stream?\n");
        		return -1;
      		}
      		if (vorbis_synthesis_headerin(&vi,&vc,&op))
			{
        		Console_Printf("Error parsing Vorbis stream headers; corrupt stream?\n");
        		return -1;
      		}
      		vorbis_p++;
      		if(vorbis_p==3)
				break;
    	}

    	/* The header pages/packets will arrive before anything else we
       	   care about, or the stream is not obeying spec */

    	if (ogg_sync_pageout(&oy,&og)>0)
		{
      		queue_page(&og); /* demux into the appropriate stream */
    	}
		else
		{
      		int ret=buffer_data(infile,&oy); /* someone needs more data */
     		if(ret==0)
			{
        		Console_Printf("End of file while searching for codec headers.\n");
        		return -1;
      		}
    	}
  	}

  	/* and now we have it all.  initialize decoders */
 	if(theora_p)
	{
    	theora_decode_init(&td,&ti);
    	Console_Printf("Ogg logical stream %x is Theora %dx%d %.02f fps video\n",
           to.serialno,ti.width,ti.height, 
           (double)ti.fps_numerator/ti.fps_denominator);
    	if (ti.width!=ti.frame_width || ti.height!=ti.frame_height)
      		Console_Printf("  Frame content is %dx%d with offset (%d,%d).\n",
           		ti.frame_width, ti.frame_height, ti.offset_x, ti.offset_y);
    	report_colorspace(&ti);
    	dump_comments(&tc);
  	}
	else
	{
    	/* tear down the partial theora setup */
    	theora_info_clear(&ti);
    	theora_comment_clear(&tc);
		return -1;
  	}

  	if (vorbis_p)
	{
    	vorbis_synthesis_init(&vd,&vi);
    	vorbis_block_init(&vd,&vb);
    	Console_Printf("Ogg logical stream %x is Vorbis %d channel %d Hz audio.\n",
            vo.serialno,vi.channels,vi.rate);
  	}
	else
	{
   		/* tear down the partial vorbis setup */
    	vorbis_info_clear(&vi);
    	vorbis_comment_clear(&vc);
		//return -1; don't abort just because there's no audio
  	}

	/* open audio */
	if(vorbis_p)
	{
		if (!open_audio())
		{
			return -1; //don't play back a video with no audio
		}
	}
			

	/* open video */
	if(theora_p)
	{
		if (!open_video())
		{
			return -1; //don't play back a video with no audio
		}
	}

	stateflag=0; /* playback has not begun */

	_bitmap->bmptype = BITMAP_RGB; //currently theora doesn't support alpha and it comes in 16bpp YUV format
	_bitmap->mode = 24;
	_bitmap->width = nextPowerOfTwo(ti.width);
	_bitmap->height = nextPowerOfTwo(ti.height);
	_bitmap->translucent = false;
	_bitmap->data = NULL;

	bitmap = Tag_Malloc(sizeof(bitmap_t), MEM_THEORA);
	Mem_Copy(bitmap, _bitmap, sizeof(bitmap_t));

	bitmap->data = Tag_Malloc((bitmap->width * (bitmap->mode / 8)) * bitmap->height, MEM_THEORA);
	memset(bitmap->data, 0, sizeof((bitmap->width * (bitmap->mode / 8)) * bitmap->height));

  	return 1;  //we'll use other indexes here once we don't have globals hell
}

void    Theora_GetFrame(shader_t *shader)
{
	int i, j;
	bool done = false;

	/* on to the main decode loop.  We assume in this example that audio
     and video start roughly together, and don't begin playback until
     we have a start frame for both.  This is not necessarily a valid
     assumption in Ogg A/V streams! It will always be true of the
     enxample_encoder (and most streams) though. */

	while (!done)
	{

    	/* we want a video and audio frame ready to go at all times.  If
    	   we have to buffer incoming, buffer the compressed data (ie, let
     	   ogg do the buffering) */
    	while (vorbis_p && !audiobuf_ready)
		{
			int ret;
			float **pcm;

      		/* if there's pending, decoded audio, grab it */
      		if ((ret=vorbis_synthesis_pcmout(&vd,&pcm))>0)
			{
        		int count=audiobuf_fill/2;
        		int maxsamples=(audiofd_fragsize-audiobuf_fill)/2/vi.channels;
        	
				for(i=0;i<ret && i<maxsamples;i++)
          			for(j=0;j<vi.channels;j++)
					{
            			int val=rint(pcm[j][i]*32767.f);
            			if(val>32767)val=32767;
            			if(val<-32768)val=-32768;
            			audiobuf[count++]=val;
          			}

        		vorbis_synthesis_read(&vd,i);
        		audiobuf_fill+=i*vi.channels*2;
        		if (audiobuf_fill==audiofd_fragsize)
					audiobuf_ready=1;
        		if (vd.granulepos>=0)
          			audiobuf_granulepos=vd.granulepos-ret+i;
        		else
          			audiobuf_granulepos+=i;
        
      		} 
			else
			{
        
        		/* no pending audio; is there a pending packet to decode? */
        		if(ogg_stream_packetout(&vo,&op)>0)
				{
          			if(vorbis_synthesis(&vb,&op)==0) /* test for success! */
            			vorbis_synthesis_blockin(&vd,&vb);
        		}
				else   /* we need more data; break out to suck in another page */
          			break;
      		}
    	}

    	while(theora_p && !videobuf_ready)
		{
      		/* theora is one in, one out... */
      		if(ogg_stream_packetout(&to,&op)>0)
	  		{

        		theora_decode_packetin(&td,&op);
        		videobuf_granulepos=td.granulepos;
        
        		videobuf_time=theora_granule_time(&td,videobuf_granulepos);

        		/* is it already too old to be useful?  This is only actually
        		   useful cosmetically after a SIGSTOP.  Note that we have to
        		   decode the frame even if we don't show it (for now) due to
        		   keyframing.  Soon enough libtheora will be able to deal
        		   with non-keyframe seeks.  */

        		if (videobuf_time>=get_time())
				{
        			videobuf_ready=1;
				}
      		}
	  		else
      			break;
    	}

    	if (!videobuf_ready && !audiobuf_ready 
			&& infile->eof(infile)
			)
		{
			shader->numplays++; //mark the movie as done being played
			return;
		}

    	if (!videobuf_ready || !audiobuf_ready)
		{
    		/* no data yet for somebody.  Grab another page */
    		int ret=buffer_data(infile,&oy);
    		while(ogg_sync_pageout(&oy,&og)>0)
			{
    			queue_page(&og);
    		}
    	}

    	/* If playback has begun, top audio buffer off immediately. */
    	if (stateflag) 
			audio_write_nonblocking();

    	/* are we at or past time for this video frame? */
    	if (stateflag && videobuf_ready && videobuf_time<=get_time())
		{
    		video_write(screen);
      		videobuf_ready=0;
    	}

    	/* if our buffers either don't exist or are ready to go,
    	   we can begin playback */
    	if ((!theora_p || videobuf_ready) &&
    		(!vorbis_p || audiobuf_ready))
			stateflag=1;

    	/* same if we've run out of input */
    	if (infile->eof(infile))
			stateflag=1;

    	if (stateflag &&
       		(audiobuf_ready || !vorbis_p) &&
       		(videobuf_ready || !theora_p) &&
       		!done)
		{
    		/* we have an audio frame ready (which means the audio buffer is
    		   full), it's not time to play video, so keep reading in data */
			
			//this used to get called when the select call came back not-timed-out
			//audio_calibrate_timer(0);
			
			//draw the current frame to the screen and exit this function
			done = true;
   		}

	}
	return;
}

void    Theora_Unload(shader_t *shader)
{
  	/* tear it all down */

  	audio_close();
  	close_video();

  	if (vorbis_p)
	{
    	ogg_stream_clear(&vo);
    	vorbis_block_clear(&vb);
    	vorbis_dsp_clear(&vd);
    	vorbis_comment_clear(&vc);
    	vorbis_info_clear(&vi);
  	}
  	
	if (theora_p)
	{
    	ogg_stream_clear(&to);
    	theora_clear(&td);
    	theora_comment_clear(&tc);
    	theora_info_clear(&ti);
  	}
  	ogg_sync_clear(&oy);

  	if (infile)
		File_Close(infile);

  	Console_Printf( "Theora Unloading - Done.\n");

	Tag_Free(bitmap->data);
	Tag_Free(bitmap);
	bitmap = NULL;
}

void    Theora_Initialize()
{ //no action necessary
}

void    Theora_ShutDown()
{ //no action necessary
}


void    Theora_Stop(shader_t *shader)
{
}

void    Theora_Restart(shader_t *shader)
{
}

void    Theora_Continue(shader_t *shader)
{
}
