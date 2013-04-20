#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "mp3info.h"
#include "objects.h"
#include "multimedia.h"
#include "zsfunctions.h"
#include "helpfunctions.h"

#include "avi.h"
#include "audio.h"
#include "video.h"

#ifdef HAVE_FLAC_HEADERS

#include <sys/stat.h>
#include <FLAC/metadata.h>
#include <FLAC/format.h>

#endif

char *genre_s[] = {
	"Blues", "Classic Rock", "Country", "Dance",
	"Disco", "Funk", "Grunge", "Hip-Hop",
	"Jazz", "Metal", "New Age", "Oldies",
	"Other", "Pop", "R&B", "Rap",
	"Reggae", "Rock", "Techno", "Industrial",
	"Alternative", "Ska", "Death Metal", "Pranks",
	"Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop",
	"Vocal", "Jazz+Funk", "Fusion", "Trance",
	"Classical", "Instrumental", "Acid", "House",
	"Game", "Sound Clip", "Gospel", "Noise",
	"AlternRock", "Bass", "Soul", "Punk",
	"Space", "Meditative", "Instrumental Pop", "Instrumental Rock",
	"Ethnic", "Gothic", "Darkwave", "Techno-Industrial",
	"Electronic", "Pop-Folk", "Eurodance", "Dream",
	"Southern Rock", "Comedy", "Cult", "Gangsta",
	"Top 40", "Christian Rap", "Pop_Funk", "Jungle",
	"Native American", "Cabaret", "New Wave", "Psychadelic",
	"Rave", "Showtunes", "Trailer", "Lo-Fi",
	"Tribal", "Acid Punk", "Acid Jazz", "Polka",
	"Retro", "Musical", "Rock & Roll", "Hard Rock",
	"Folk", "Folk-Rock", "National Folk", "Swing",
	"Fast Fusion", "Bebob", "Latin", "Revival",
	"Celtic", "Bluegrass", "Avantgarde", "Gothic Rock",
	"Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock",
	"Big Band", "Chorus", "Easy Listening", "Acoustic",
	"Humour", "Speech", "Chanson", "Opera",
	"Chamber Music", "Sonata", "Symphony", "Booty Bass",
	"Primus", "Porn Groove", "Satire", "Slow Jam",
	"Club", "Tango", "Samba", "Folklore",
	"Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle",
	"Duet", "Punk Rock", "Drum Solo", "A cappella",
	"Euro-House", "Dance Hall", "Goa", "Drum & Bass",
	"Club House", "Hardcore", "Terror", "Indie",
	"BritPop", "Negerpunk", "Polsk Punk", "Beat",
	"Christian Gangsta Rap", "Heavy Metal", "Black Metal", "Crossover",
	"Contemporary Christian", "Christian Rock", "Merengue", "Salsa",
	"Thrash Metal", "Anime", "JPop", "Synthpop",
	"Unknown"
};
unsigned char genre_count=149;

char *fps_s[] = {"Unknown", "23.976", "24", "25", "29.97", "30", "50", "59.94", "60"};
char *layer_s[] = {"Unknown", "Layer III", "Layer II", "Layer I", "None"};
char *codec_s[] = {"Mpeg 2.5", "Unknown", "Mpeg 2", "Mpeg 1", "FLAC"};
char *chanmode_s[] = {"Stereo", "Joint Stereo", "Dual Channel", "Single Channel", "Unknown"};
char *flac_chanmode_s[] = {"Unknown", "1 Channel", "2 Channels", "3 Channels", "4 Channels", "5 Channels", "6 Channels", "7 Channels", "8 Channels"};

char           *
get_preset(char vbr_header[4])
{
	int		preset;
	static char	returnval[10];
	memset(returnval, 0, 10);

	preset = ((unsigned char)vbr_header[0] & 7) * 256 + (unsigned char)vbr_header[1];

	strcpy(returnval, "NA");
	switch (preset) {
	case 1000:
		strcpy(returnval, "APR");
		break;		/* r3mix         */
	case 1001:
		strcpy(returnval, "APS");
		break;		/* standard      */
	case 1002:
		strcpy(returnval, "APE");
		break;		/* extreme       */
	case 1003:
		strcpy(returnval, "API");
		break;		/* insane        */
	case 1004:
		strcpy(returnval, "FAPS");
		break;		/* fast standard */
	case 1005:
		strcpy(returnval, "FAPE");
		break;		/* fast extreme  */
	case 1006:
		strcpy(returnval, "APM");
		break;		/* medium        */
	case 1007:
		strcpy(returnval, "FAPM");
		break;		/* fast medium   */
	case 320:
		strcpy(returnval, "INSANE");
		break;		/* insane        */
	case 410:
		strcpy(returnval, "V9");
		break;          /* V9   */
	case 420:
		strcpy(returnval, "V8");
		break;          /* V8   */
	case 430:
		strcpy(returnval, "V7");
		break;          /* V7   */
	case 440:
		strcpy(returnval, "V6");
		break;          /* V6   */
	case 450:
		strcpy(returnval, "V5");
		break;          /* V5   */
	case 460:
		strcpy(returnval, "V4");
		break;          /* V4   */
	case 470:
		strcpy(returnval, "V3");
		break;          /* V3   */
	case 480:
		strcpy(returnval, "V2");
		break;          /* V2   */
	case 490:
		strcpy(returnval, "V1");
		break;          /* V1   */
	case 500:
		strcpy(returnval, "V0");
		break;          /* V0   */
	}
	return returnval;
}


/*
 * First Version:	 20111207	Sked
 * Modified:		20130117	Sked
 * Description: Umbrella for the different get_*_audio_info functions where
 *		the correct function is chosen based on the fileextension.
 *		This makes it easier to add new audioformats.
 */
void
get_audio_info(char *f, struct audio *audio)
{
	char *ext;

	if (f != NULL && (ext = find_last_of(f, ".")) != NULL) {
		if (!strcasecmp(".mp3", ext))
			get_mpeg_audio_info(f, audio);

#ifdef HAVE_FLAC_HEADERS
		else if (!strcasecmp(".flac", ext))
			get_flac_audio_info(f, audio);
#endif

		else {
			d_log("multimedia.c: get_audio_info() - Received %s as fileextension but no libs present to get metadata.\n", ext);
			return;
		}

		/* cleanup id3_artist/title/album */
		/* remove prefixing whitespace chars (space, formfeed,
		 * newline, carriage return, horizontal and vertical tab)
		 */
		strcpy(audio->id3_artist, prestrip_chars(audio->id3_artist, WHITESPACE_STR));
		strcpy(audio->id3_title, prestrip_chars(audio->id3_title, WHITESPACE_STR));
		strcpy(audio->id3_album, prestrip_chars(audio->id3_album, WHITESPACE_STR));
		/* remove trailing whitespace chars (same as prefixing) */
		tailstrip_chars(audio->id3_artist, WHITESPACE_STR);
		tailstrip_chars(audio->id3_title, WHITESPACE_STR);
		tailstrip_chars(audio->id3_album, WHITESPACE_STR);
		/* remove bad chars from the complete string (like above but without space and including backspace) */
		strip_chars(audio->id3_artist, BAD_STR);
		strip_chars(audio->id3_title, BAD_STR);
		strip_chars(audio->id3_album, BAD_STR);
	}

}

/*
 * Updated     : 01.22.2002 Author      : Dark0n3
 * 
 * Description : Reads MPEG header from file and stores info to 'audio'.
 */
void 
get_mpeg_audio_info(char *f, struct audio *audio)
{
	int		fd;
	int		n;
	int		tag_ok = 0;
	unsigned char	header[4];
	unsigned char	vbr_header[4];
	unsigned char	xing_header1[4], xing_header2[4], xing_header3[4];
	unsigned char	fraunhofer_header[4];
	unsigned char	id3v2_header[10];
	unsigned char	version;
	unsigned char	layer;
	unsigned char	protected = 1;
	unsigned char	t_genre;
	unsigned char	t_bitrate;
	unsigned char	t_samplingrate;
	unsigned char	channelmode;
	short int	bitrate = 0;
	short int	br_v1_l3[] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
	short int	br_v1_l2[] = {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0};
	short int	br_v1_l1[] = {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0};
	short int	br_v2_l1[] = {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0};
	short int	br_v2_l23[] = {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0};
	unsigned int	samplingrate = 0;
	unsigned int	sr_v1[] = {44100, 48000, 32000, 0};
	unsigned int	sr_v2[] = {22050, 24000, 16000, 0};
	unsigned int	sr_v25[] = {11025, 12000, 8000, 0};
	int		vbr_offset = 0;
	int		t1;
	unsigned char	vbr_misc;

	d_log("multimedia.c: get_mpeg_audio_info() - starting: %s\n", f);

	fd = open(f, O_RDONLY);
	if (fd < 0)
	{
		d_log("multimedia.c: get_mpeg_audio_info() - could not open file '%s': %s\n", f, strerror(errno));
		strcpy(audio->id3_year, "0000");
		strcpy(audio->id3_title, "Unknown");
		strcpy(audio->id3_artist, "Unknown");
		strcpy(audio->id3_album, "Unknown");
		audio->id3_genre = genre_s[genre_count - 1];

		return;
	}

	n = 2;
	while (read(fd, header + 2 - n, n) == n) {
		if (*header == 255) {
			n = 2;
			if (*(header + 1) >= 224) {
				n = 0;
				break;
			} else {
				n = 2;
			}
		} else {
			if (*(header + 1) == 255) {
				*header = *(header + 1);
				n = 1;
			} else {
				n = 2;
			}
		}
	}

	/*
	 * mp3 header: AAAAAAAA AAABBCCD EEEEFFGH IIJJKLMM A - Frame sync B -
	 * MPEG audio version (version) C - Layer (layer) D - Protected by
	 * CRC (protected) E - Bitrate (t_bitrate) F - Sampling rate
	 * (t_samplingrate) G - Padding H - Private bit I - Channel mode
	 * (channelmode) J - Mode extension, K - Copyright L - Original, M -
	 * Emphasis
	 */
	if (n == 0) {
		*(header + 1) -= 224;

		if (read(fd, header + 2, 2) == -1) {
			d_log("multimedia.c: get_mpeg_audio_info() - read() failed - may lead to unexpected result.\n");
		}

		version = (*(header + 1)) >> 3;
		layer = (*(header + 1) >> 1) & ((1 << 2) - 1);	/* Nasty code, keeps CC
								 * in 'layer'. (layer =
								 * (*(header + 1) -
								 * (version << 3)) >> 1) */
		protected = (*(header + 1)) & 1;
		t_bitrate = (*(header + 2)) >> 4;
		t_samplingrate = (*(header + 2) >> 2) & ((1 << 2) - 1);	/* Nasty code, keeps FF
									 * in 't_samplingrate'.
									 * (t_samplingrate =
									 * *(header + 2) -
									 * (t_bitrate << 4) >>
									 * 2) */

		switch (version) {
		case 0:
			samplingrate = sr_v25[t_samplingrate];
		case 2:
			if (!samplingrate)
				samplingrate = sr_v2[t_samplingrate];
			switch (layer) {
			case 3:
				bitrate = br_v2_l1[t_bitrate];
				break;
			case 1:
			case 2:
				bitrate = br_v2_l23[t_bitrate];
				break;
			}
			break;
		case 3:
			samplingrate = sr_v1[t_samplingrate];
			switch (layer) {
			case 1:
				bitrate = br_v1_l3[t_bitrate];
				break;
			case 2:
				bitrate = br_v1_l2[t_bitrate];
				break;
			case 3:
				bitrate = br_v1_l1[t_bitrate];
				break;
			}
			break;
		}
		channelmode = (*(header + 3)) >> 6;

		sprintf(audio->samplingrate, "%i", samplingrate);
		sprintf(audio->bitrate, "%i", bitrate);
		audio->codec = codec_s[version];
		audio->layer = layer_s[layer];
		audio->channelmode = chanmode_s[channelmode];

		/* LAME VBR TAG */
		lseek(fd, 0, SEEK_SET);
		if (read(fd, id3v2_header, 10) == -1) {
			d_log("multimedia.c: get_mpeg_audio_info() - read() for id3v2_header failed - may lead to unexpected result.\n");
		}

		if (memcmp(id3v2_header, "ID3", 3) == 0) {
			/*
			 * The ID3V2 tag is prepended to the mp3file, so we
			 * must adjust the vbr_offset accordingly. ID3V2 uses
			 * synchsafe integers hence this bitmanipulation.
			 * Reference :
			 * http://www.id3.org/id3v2.4.0-structure.txt
			 */
			vbr_offset = (id3v2_header[8] >> 1) * 256 + ((id3v2_header[8] & 1) * 128) + id3v2_header[9] + 10;
		}
		lseek(fd, 13 + vbr_offset, SEEK_SET);
		if (read(fd, xing_header1, 4) == -1) {
			d_log("multimedia.c: get_mpeg_audio_info() - read() for xing_header1 failed - may lead to unexpected result.\n");
		}
		lseek(fd, 21 + vbr_offset, SEEK_SET);
		if (read(fd, xing_header2, 4) == -1) {
			d_log("multimedia.c: get_mpeg_audio_info() - read() for xing_header2 failed - may lead to unexpected result.\n");
		}
		lseek(fd, 36 + vbr_offset, SEEK_SET);
		if (read(fd, xing_header3, 4) == -1) {
			d_log("multimedia.c: get_mpeg_audio_info() - read() for xing_header3 failed - may lead to unexpected result.\n");
		}
		lseek(fd, 36 + vbr_offset, SEEK_SET);
		if (read(fd, fraunhofer_header, 4) == -1) {
			d_log("multimedia.c: get_mpeg_audio_info() - read() for fraunhofer_header failed - may lead to unexpected result.\n");
		}

		if (memcmp(xing_header1, "Xing", 4) == 0 ||
		    memcmp(xing_header2, "Xing", 4) == 0 ||
		    memcmp(xing_header2, "LAME", 4) == 0 ||
		    memcmp(xing_header3, "Xing", 4) == 0 ||
		    memcmp(fraunhofer_header, "VBRI", 4) == 0) {

			lseek(fd, 165 + vbr_offset, SEEK_SET);
			if (read(fd, &audio->vbr_oldnew, 1) == -1) {
				d_log("multimedia.c: get_mpeg_audio_info() - read() for audio->vbr_oldnew failed - may lead to unexpected result.\n");
			}
			audio->vbr_oldnew = (audio->vbr_oldnew & 4) >> 2; /* vbr method (vbr-old, vbr-new) */

			lseek(fd, 180 + vbr_offset, SEEK_SET);
			if (read(fd, &vbr_misc, 1) == -1) {
				d_log("multimedia.c: get_mpeg_audio_info() - read() for vbr_misc failed - may lead to unexpected result.\n");
			}
			audio->vbr_noiseshaping = (vbr_misc & 3); /* vbr noiseshaping */
			if (((vbr_misc & 28) >> 2) == 0)
				sprintf(audio->vbr_stereo_mode, "Mono");
			else if (((vbr_misc & 28) >> 2) == 1)
				sprintf(audio->vbr_stereo_mode, "Stereo");
			else if (((vbr_misc & 28) >> 2) == 2)
				sprintf(audio->vbr_stereo_mode, "Dual");
			else if (((vbr_misc & 28) >> 2) == 3)
				sprintf(audio->vbr_stereo_mode, "Joint");
			else if (((vbr_misc & 28) >> 2) == 4)
				sprintf(audio->vbr_stereo_mode, "Force");
			else if (((vbr_misc & 28) >> 2) == 5)
				sprintf(audio->vbr_stereo_mode, "Auto");
			else if (((vbr_misc & 28) >> 2) == 6)
				sprintf(audio->vbr_stereo_mode, "Intensity");
			else
				sprintf(audio->vbr_stereo_mode, "Undefined");
/*			audio->vbr_stereo_mode = (vbr_misc & 28) >> 2; // vbr stereo mode */
			if (((vbr_misc & 32) >> 5))
				sprintf(audio->vbr_unwise, "Yes");
			else
				sprintf(audio->vbr_unwise, "No");
/*			audio->vbr_unwise = (vbr_misc & 32) >> 5;      // vbr unwise setting */
			if (((vbr_misc & 192) >> 6) == 0)
				sprintf(audio->vbr_source, "<32.000Hz");
			else if (((vbr_misc & 192) >> 6) == 1)
				sprintf(audio->vbr_source, "44.100Hz");
			else if (((vbr_misc & 192) >> 6) == 2)
				sprintf(audio->vbr_source, "48.000Hz");
			else
				sprintf(audio->vbr_source, ">48.000Hz");
/*			audio->vbr_source = (*vbr_misc & 192) >> 6;     // vbr source sample frequency */

			lseek(fd, 176 + vbr_offset, SEEK_SET);
			if (read(fd, &audio->vbr_minimum_bitrate, 1) == -1) { /* minimumvbr bitrate, or abr bitrate */
				d_log("multimedia.c: get_mpeg_audio_info() - read() for audio->vbr_minimum_bitrate failed - may lead to unexpected result.\n");
			}

			lseek(fd, 155 + vbr_offset, SEEK_SET);
			if (read(fd, &audio->vbr_quality, 1) == -1) { /* vbr quality setting */
				d_log("multimedia.c: get_mpeg_audio_info() - read() for audio->vbr_quality failed - may lead to unexpected result.\n");
			}

			lseek(fd, 156 + vbr_offset, SEEK_SET);
			if (read(fd, audio->vbr_version_string, 9) == -1) { /* vbr version, short string */
				d_log("multimedia.c: get_mpeg_audio_info() - read() for audio->vbr_version_string failed - may lead to unexpected result.\n");
			}
			audio->vbr_version_string[9] = 0;
			for (t1 = 9; t1 > 0; t1--) {
				if (audio->vbr_version_string[t1] > 32) {
					break;
				}
				audio->vbr_version_string[t1] = 0;
			}

			audio->is_vbr = 1;
			if (memcmp(audio->vbr_version_string, "LAME", 4) == 0) {
				lseek(fd, 182 + vbr_offset, SEEK_SET);
				if (read(fd, vbr_header, 2) == -1) {
					d_log("multimedia.c: get_mpeg_audio_info() - read() for vbr_header failed - may lead to unexpected result.\n");
				}
				sprintf(audio->vbr_preset, "%s", get_preset((char *)vbr_header));

				if (audio->vbr_version_string[4] == 32)
					audio->vbr_version_string[4] = 0;

				/* strcpy(audio->bitrate, "VBR"); */
			} else {
				strcpy(audio->vbr_version_string, "Not LAME");
				strcpy(audio->vbr_preset, "NA");
			}

		} else {
			audio->is_vbr = 0;
			strcpy(audio->vbr_version_string, "NA");
			strcpy(audio->vbr_preset, "NA");
		}

		if (memcmp(fraunhofer_header, "VBRI", 4) == 0) {
			strcpy(audio->vbr_version_string, "FHG");
		}
		/* ID3 TAG */
		lseek(fd, -128, SEEK_END);
		if (read(fd, header, 3) == -1) {
			d_log("multimedia.c: get_mpeg_audio_info() - read() for header failed - may lead to unexpected result.\n");
		}
		if (memcmp(header, "TAG", 3) == 0) {	/* id3 tag */
			tag_ok = 1;
			if (read(fd, audio->id3_title, 30) == -1) {
				d_log("multimedia.c: get_mpeg_audio_info() - read() for audio->id3_title failed - may lead to unexpected result.\n");
			}
			if (read(fd, audio->id3_artist, 30) == -1) {
				d_log("multimedia.c: get_mpeg_audio_info() - read() for audio->id3_artist failed - may lead to unexpected result.\n");
			}
			if (read(fd, audio->id3_album, 30) == -1) {
				d_log("multimedia.c: get_mpeg_audio_info() - read() for audio->id3_album failed - may lead to unexpected result.\n");
			}
			if (read(fd, audio->id3_year, 4) == -1) {
				d_log("multimedia.c: get_mpeg_audio_info() - read() for audio->id3_year failed - may lead to unexpected result.\n");
			}
			if (tolower(audio->id3_year[1]) == 'k') {
				memcpy(header, audio->id3_year, 3);
				sprintf(audio->id3_year, "%c00%c", *header, *(header + 2));
			}
			lseek(fd, -1, SEEK_END);
			if (read(fd, &t_genre, 1) == -1) {
				d_log("multimedia.c: get_mpeg_audio_info() - read() for t_genre failed - may lead to unexpected result.\n");
			}
			if (t_genre > genre_count - 1)
				t_genre = genre_count - 1;

			audio->id3_genre = genre_s[t_genre];
			audio->id3_year[4] =
				audio->id3_artist[30] =
				audio->id3_title[30] =
				audio->id3_album[30] = 0;
		}
	} else {		/* header is broken, shouldnt crc fail? */
		strcpy(audio->samplingrate, "0");
		strcpy(audio->bitrate, "0");
		audio->codec = codec_s[1];
		audio->layer = layer_s[0];
		audio->channelmode = chanmode_s[4];
	}

	if (tag_ok == 0) {
		strcpy(audio->id3_year, "0000");
		strcpy(audio->id3_title, "Unknown");
		strcpy(audio->id3_artist, "Unknown");
		strcpy(audio->id3_album, "Unknown");
		audio->id3_genre = genre_s[genre_count - 1];
	}
	close(fd);

	get_mp3_info(f, audio);

	d_log("multimedia.c: get_mpeg_audio_info() - values: id3_artist: %s, id3_title: %s, id3_album: %s, id3_year: %s, bitrate: %s, samplingrate: %s\n",
			audio->id3_artist, audio->id3_title, audio->id3_album, audio->id3_year, audio->bitrate, audio->samplingrate);
	d_log("multimedia.c: get_mpeg_audio_info() - values: id3_genre: %s, layer: %s, codec: %s, channelmode: %s, vbr_version_string: %s, vbr_preset: %s\n",
			audio->id3_genre, audio->layer, audio->codec, audio->channelmode, audio->vbr_version_string, audio->vbr_preset);
	d_log("multimedia.c: get_mpeg_audio_info() - values: is_vbr: %d, vbr_oldnew: %d, vbr_quality: %d, vbr_minimum_bitrate: %d, vbr_noiseshaping: %d\n",
			audio->is_vbr, audio->vbr_oldnew, audio->vbr_quality, audio->vbr_minimum_bitrate, audio->vbr_noiseshaping);
	d_log("multimedia.c: get_mpeg_audio_info() - values: vbr_stereo_mode: %s, vbr_unwise: %s, vbr_source: %s\n", audio->vbr_stereo_mode, audio->vbr_unwise, audio->vbr_source);
	(void) protected;
}

/*
 * First Version: 2011.01.30	io
 * Last update	: 2011.12.10	Sked
 * Description	: Reads FLAC header from file and stores info to 'audio'.
 */
#ifdef HAVE_FLAC_HEADERS
void
get_flac_audio_info(char *f, struct audio *audio)
{
	int fd;
	FLAC__StreamMetadata *temp_meta; /* union that holds all vorbis meta data */

	d_log("multimedia.c: get_flac_audio_info() - starting: %s\n", f);

	/* defaults */
	strcpy(audio->id3_year, "0000");
	strcpy(audio->id3_title, "Unknown");
	strcpy(audio->id3_artist, "Unknown");
	strcpy(audio->id3_album, "Unknown");
	strcpy(audio->vbr_version_string, "Unknown");
	audio->id3_genre = genre_s[genre_count - 1];
	audio->is_vbr = 1;
	audio->codec = codec_s[4];
	audio->channelmode = flac_chanmode_s[0];
	audio->layer = layer_s[4];

	fd = open(f, O_RDONLY);
	if (fd < 0) {
		d_log("multimedia.c: get_flac_audio_info() - could not open file '%s': %s\n", f, strerror(errno));
		return;
	}

	temp_meta = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);

	if (FLAC__metadata_get_tags(f, &temp_meta)) {
		FLAC__uint32 i, j, k = 0;

		d_log("multimedia.c: get_flac_audio_info() - read metadata -- vendor string length: %d, string: %s\n", temp_meta->data.vorbis_comment.vendor_string.length, temp_meta->data.vorbis_comment.vendor_string.entry);
		if (temp_meta->data.vorbis_comment.vendor_string.length > 0) {
			if (!strncmp((char *)temp_meta->data.vorbis_comment.vendor_string.entry, "reference libFLAC", 17))
				k = 10;
			for (i = 0; i < NAME_MAX - 1 && i < temp_meta->data.vorbis_comment.vendor_string.length; ++i)
				audio->vbr_version_string[i] = temp_meta->data.vorbis_comment.vendor_string.entry[i+k];
			audio->vbr_version_string[i] = '\0';
		}

		for (i = 0; i < temp_meta->data.vorbis_comment.num_comments; ++i) {
			d_log("multimedia.c: get_flac_audio_info() - comment #%d: length = %d, data = %s\n", i, temp_meta->data.vorbis_comment.comments[i].length, temp_meta->data.vorbis_comment.comments[i].entry);

			j = 0;
			while (j < temp_meta->data.vorbis_comment.comments[i].length && temp_meta->data.vorbis_comment.comments[i].entry[j] != '=')
				++j;

			if (!strncasecmp((char *)temp_meta->data.vorbis_comment.comments[i].entry, "ARTIST", j)) {
				++j;
				if (temp_meta->data.vorbis_comment.comments[i].entry[j]) {
					k = 0;
					while (k < NAME_MAX - 1 && temp_meta->data.vorbis_comment.comments[i].entry[j]) {
						audio->id3_artist[k] = temp_meta->data.vorbis_comment.comments[i].entry[j];
						++k;
						++j;
					}
					audio->id3_artist[k] = '\0';
				}

			} else if (!strncasecmp((char *)temp_meta->data.vorbis_comment.comments[i].entry, "TITLE", j)) {
				++j;
				if (temp_meta->data.vorbis_comment.comments[i].entry[j]) {
					k = 0;
					while (k < NAME_MAX - 1 && temp_meta->data.vorbis_comment.comments[i].entry[j]) {
						audio->id3_title[k] = temp_meta->data.vorbis_comment.comments[i].entry[j];
						++k;
						++j;
					}
					audio->id3_title[k] = '\0';
				}

			} else if (!strncasecmp((char *)temp_meta->data.vorbis_comment.comments[i].entry, "ALBUM", j)) {
				++j;
				if (temp_meta->data.vorbis_comment.comments[i].entry[j]) {
					k = 0;
					while (k < NAME_MAX - 1 && temp_meta->data.vorbis_comment.comments[i].entry[j]) {
						audio->id3_album[k] = temp_meta->data.vorbis_comment.comments[i].entry[j];
						++k;
						++j;
					}
					audio->id3_album[k] = '\0';
				}

			} else if (!strncasecmp((char *)temp_meta->data.vorbis_comment.comments[i].entry, "DATE", j)) {
				++j;
				if (temp_meta->data.vorbis_comment.comments[i].entry[j]) {
					k = 0;
					while (k < 4 && temp_meta->data.vorbis_comment.comments[i].entry[j]) {
						audio->id3_year[k] = temp_meta->data.vorbis_comment.comments[i].entry[j];
						++k;
						++j;
					}
					audio->id3_year[k] = '\0';
				}

			} else if (!strncasecmp((char *)temp_meta->data.vorbis_comment.comments[i].entry, "GENRE", j)) {
				++j;
				if (temp_meta->data.vorbis_comment.comments[i].entry[j]) {
					k = 0;
					/* (unsigned)1 to get rid of compiler warning */
					while (k < genre_count - (unsigned)1 && strcmp(safe_genre((char *)temp_meta->data.vorbis_comment.comments[i].entry + j), genre_s[k]))
						++k;
					audio->id3_genre = genre_s[k];
				}

			} else
				d_log("multimedia.c: get_flac_audio_info() - Nonindexed tag found: %s\n", temp_meta->data.vorbis_comment.comments[i].entry);

		}

		d_log("multimedia.c: get_flac_audio_info() - TAG audio info: --- artist: %s, album: %s, title: %s, year: %s, genre: %s\n", audio->id3_artist, audio->id3_album, audio->id3_title, audio->id3_year, audio->id3_genre);

	} else
		d_log("multimedia.c: get_flac_audio_info() - failed getting comment meta data\n");

	if(temp_meta != NULL)
		FLAC__metadata_object_delete(temp_meta);
	temp_meta = FLAC__metadata_object_new(FLAC__METADATA_TYPE_STREAMINFO);

	/* now get technical info */
	if (FLAC__metadata_get_streaminfo(f, temp_meta)) {
		struct stat st;

		sprintf(audio->samplingrate, "%d",  temp_meta->data.stream_info.sample_rate);

		if (temp_meta->data.stream_info.channels < 9)
			audio->channelmode = flac_chanmode_s[temp_meta->data.stream_info.channels];

		/* bitrate = (filesize in bits / duration in seconds) -> bps
		 * duration = total samples / samplerate in Hz -> s
		 * --> bitrate = (filesize in bytes * samplerate in Hz)/(125 * total samples) -> kbps
		 */
		stat(f, &st);

		if (temp_meta->data.stream_info.total_samples)
			sprintf(audio->bitrate, "%llu", (st.st_size * temp_meta->data.stream_info.sample_rate) / (125 * temp_meta->data.stream_info.total_samples));

		d_log("multimedia.c: get_flac_audio_info() - stream info: %d Hz, %d channel(s), %d bits per sample, %d total samples\n", temp_meta->data.stream_info.sample_rate,
														temp_meta->data.stream_info.channels,
														temp_meta->data.stream_info.bits_per_sample,
														temp_meta->data.stream_info.total_samples);
		d_log("multimedia.c: get_flac_audio_info() - filesize: %llu, bitrate: %s\n", st.st_size, audio->bitrate);

	} else {
		d_log("multimedia.c: get_flac_audio_info() - failed getting stream meta data\n");
		strcpy(audio->samplingrate, "0");
		strcpy(audio->bitrate, "0");
	}

	if(temp_meta != NULL)
		FLAC__metadata_object_delete(temp_meta);

	close(fd);

	d_log("multimedia.c: get_flac_audio_info() - values: id3_artist: %s, id3_title: %s, id3_album: %s, id3_year: %s, bitrate: %s, samplingrate: %s\n",
			audio->id3_artist, audio->id3_title, audio->id3_album, audio->id3_year, audio->bitrate, audio->samplingrate);
	d_log("multimedia.c: get_flac_audio_info() - values: id3_genre: %s, layer: %s, codec: %s, channelmode: %s, vbr_version_string: %s, vbr_preset: %s\n",
			audio->id3_genre, audio->layer, audio->codec, audio->channelmode, audio->vbr_version_string, audio->vbr_preset);
	d_log("multimedia.c: get_flac_audio_info() - values: is_vbr: %d, vbr_oldnew: %d, vbr_quality: %d, vbr_minimum_bitrate: %d, vbr_noiseshaping: %d\n",
			audio->is_vbr, audio->vbr_oldnew, audio->vbr_quality, audio->vbr_minimum_bitrate, audio->vbr_noiseshaping);
	d_log("multimedia.c: get_flac_audio_info() - values: vbr_stereo_mode: %s, vbr_unwise: %s, vbr_source: %s\n", audio->vbr_stereo_mode, audio->vbr_unwise, audio->vbr_source);
}
#endif

/*
 * First Version: 2011.12.09	Sked
 * Description: substitute any '/' char which is unsafe to use in dirnames
 *		by the '_' char, and return the given genre, this way it can
 *		easily be used in other functions arguments
 */
char *
safe_genre(char *genre)
{
	int i = 0;

	while (genre[i]) {
		if (genre[i] == '/')
			genre[i] = '_';
		++i;
	}

	return genre;
}

const unsigned char *fourcc(FOURCC tag)
{
	static unsigned char buf[5];
	int i;

	buf[0] = tag & 255;
	buf[1] = (tag >> 8) & 255;
	buf[2] = (tag >> 16) & 255;
	buf[3] = (tag >> 24) & 255;
	buf[4] = 0;

	for (i = 0; i < 4; i++)
		if (buf[i] < 32 || buf[i] > 127)
			buf[i] = '?';

	return buf;
}

void avierror(const char *s)
{
	fprintf(stderr, "%s\n", s);
	exit(1);
}

DWORD get32(FILE *f)
{
	DWORD result;
	
	if (fread(&result, sizeof(DWORD), 1, f) != 1) {
		d_log("avinfo: Premature end of file.\n");
		result=0;
	}
	return result;
}

WORD get16(FILE *f)
{
	WORD result;
	
	if (fread(&result, sizeof(WORD), 1, f) != 1) {
		d_log("avinfo: Premature end of file.\n");
		result=0;
	}
	return result;
}

int avinfo(char *filename, struct VIDEO *vinfo)
{
	FILE *f;
	FOURCC tag, list = 0, vids = 0, type = 0;
	DWORD size, hz = 0;
	WORD auds = 0, ch = 0;
	double fps = 0;
	int width = 0, height = 0, i;
	char buf[1024], fourcc_vids[5];
	const char *_vids = "Unknown codec", *_auds = "Unknown codec";

	f = fopen(filename, "rb");

	if (!f) {
		d_log("avinfo: Unable to open file.\n");
		return 1;
	}
	if (!(tag = get32(f)))
		return 1;
	if (!(size = get32(f)))
		return 1;

	if (tag != MKTAG('R','I','F','F')) {
		d_log("avinfo: Not a RIFF file.\n");
		return 2;
	}
	if (!(tag = get32(f)))
		return 1;

	if (tag != MKTAG('A','V','I',' ')) {
		d_log("avinfo: Not an AVI file.\n");
		return 2;
	}
	while (!feof(f)) {
		if (!(tag = get32(f)))
			return 1;
		if (!(size = get32(f)))
			return 1;

		if (!tag) {
			d_log("avinfo: Invalid file format.\n");
			return 2;
		}
		if (tag == MKTAG('L','I','S','T')) {
			if ((list = get32(f)) == MKTAG('m','o','v','i')) {
				fclose(f);
				break;
			}
			continue;
		}
		
		if (tag == MKTAG('a','v','i','h')) {
			AVIMAINHEADER avih;

			if (!fread(&avih, sizeof(avih), 1, f)) {
				d_log("avinfo: Failed to fread()\n");
			}

			width = avih.dwWidth;
			height = avih.dwHeight;

			fseek(f, -sizeof(avih), SEEK_CUR);
		}

		if (tag == MKTAG('s','t','r','h')) {
			AVISTREAMHEADER strh;

			if (!fread(&strh, sizeof(strh), 1, f)) {
				d_log("avinfo: Failed to fread()\n");
			}
			
			if ((type = strh.fccType) == MKTAG('v','i','d','s')) {
				vids = strh.fccHandler;
				fps = (double) strh.dwRate / (double) strh.dwScale;
			}

			fseek(f, -sizeof(strh), SEEK_CUR);
		}

		if (tag == MKTAG('s','t','r','f')) {
			if (type == MKTAG('a','u','d','s')) {
				WAVEFORMATEX wave;

				if (!fread(&wave, sizeof(wave), 1, f)) {
					d_log("avinfo: Failed to fread()\n");
				}

				hz = wave.nSamplesPerSec;
				ch = wave.nChannels;
				auds = wave.wFormatTag;
				
				fseek(f, -sizeof(wave), SEEK_CUR);
			}
			
			if (type == MKTAG('v','i','d','s') && !vids) {
				BITMAPINFOHEADER bm;

				if (!fread(&bm, sizeof(bm), 1, f)) {
					d_log("avinfo: Failed to fread()\n");
				}

				vids = bm.biCompression;
				
				fseek(f, -sizeof(bm), SEEK_CUR);
			}
		}

		fseek(f, size + (size & 1), SEEK_CUR);
	}

	for (i = 0; audio_formats[i].tag; i++)
		if (audio_formats[i].tag == auds)
			_auds = audio_formats[i].descr;

	strcpy(fourcc_vids, (char *)fourcc(vids));

	for (i = 0; video_formats[i].tag; i++)
		if (!strcasecmp(video_formats[i].tag, fourcc_vids))
			_vids = video_formats[i].descr;

	if (hz || ch || auds)
		sprintf(buf,
			"Video: %dx%d (%.2f), %.3f fps, %s [%s] - "
			"Audio: %dHz, %dch, %s [0x%.4x]",
			width, height, (double)width/height, fps, _vids, fourcc(vids),
			hz, ch, _auds, auds);
	else
		sprintf(buf,
			"Video: %dx%d (%.2f), %.3f fps, %s [%s] - "
			"No audio",
			width, height, (double)width/height, fps, _vids, fourcc(vids));
	d_log("avinfo: %s\n", buf);
	vinfo->width = width;
	vinfo->height = height;
	vinfo->fps = fps;
	vinfo->hz = hz;
	vinfo->ch = ch;
	snprintf(vinfo->vids, sizeof(vinfo->vids), "%s", _vids);
	snprintf(vinfo->fourcc, sizeof(vinfo->fourcc), "%s", fourcc(vids));
	snprintf(vinfo->audio, sizeof(vinfo->audio), "%s", _auds);
	snprintf(vinfo->audiotype, sizeof(vinfo->audiotype), "0x%.4x", auds);
	return 0;
}
