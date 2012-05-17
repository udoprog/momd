#include "mpg123.hpp"

#include "plugin.hpp"
#include "input_error.hpp"
#include "pcm_position.hpp"
#include "pcm_packet.hpp"
#include "audio_type.hpp"

input_base* mpg123__new();
void mpg123__initialize();
bool mpg123__check(audio_type);

int from_mpg123_encoding(int encoding)
{
    switch (encoding) {
    case MPG123_ENC_SIGNED_16:
        return PCM_SIGNED_16;
    case MPG123_ENC_SIGNED_32:
        return PCM_SIGNED_32;
    case MPG123_ENC_FLOAT_32:
        return PCM_FLOAT_32;
    default:
        throw input_error("mpg123: unsupported input encoding");
    }
}

int to_mpg123_encoding(int encoding)
{
    switch (encoding) {
    case PCM_SIGNED_16:
        return MPG123_ENC_SIGNED_16;
    case PCM_SIGNED_32:
        return MPG123_ENC_SIGNED_32;
    case PCM_FLOAT_32:
        return MPG123_ENC_FLOAT_32;
    default:
        throw input_error("mpg123: unsupported input encoding");
    }
}

input_plugin_spec mpg123_spec = {
        &mpg123__initialize,
        &mpg123__check,
        &mpg123__new
};

DECLARE_INPUT_PLUGIN("mpg123", mpg123_spec)

input_base* mpg123__new()
{
        return new input_mpg123();
}

void mpg123__initialize()
{
    int err = mpg123_init();

    if (err != MPG123_OK) {
        throw input_error(mpg123_plain_strerror(err));
    }
}

std::string mp3_ext(".mp3");
std::string mp4_ext(".mp4");

bool mpg123__check(audio_type type)
{
    switch (type) {
    case AUDIO_TYPE_MP3:
        return true;
    case AUDIO_TYPE_M4A:
        return true;
    default:
        return false;
    }
}

template<unc::encoding_t encoding>
unc::ustring decode_string(mpg123_string* string)
{
    if (!string) {
        return unc::ustring();
    }

    std::string byte_string(string->p, string->fill);
    return unc::decode<encoding>(byte_string);
}

template<unc::encoding_t encoding>
unc::ustring decode_byte_string(const char* string)
{
    if (!string) {
        return unc::ustring();
    }

    std::string byte_string(string);
    return unc::decode<encoding>(byte_string);
}

input_mpg123::input_mpg123()
    : handle(NULL), current_format()
{
}

void input_mpg123::read_metadata()
{
    mpg123_scan(handle);
    int meta = mpg123_meta_check(handle);

    if (!(meta & MPG123_ID3)) {
        return;
    }

    mpg123_id3v1 *v1;
    mpg123_id3v2 *v2;

    if (mpg123_id3(handle, &v1, &v2) != MPG123_OK) {
        return;
    }

    if (v2) {
        title = decode_string<unc::utf8>(v2->title);
        artist = decode_string<unc::utf8>(v2->artist);
        album = decode_string<unc::utf8>(v2->album);
        year = decode_string<unc::utf8>(v2->year);
        return;
    }

    if (v1) {
        title = decode_byte_string<unc::latin1>(v1->title);
        artist = decode_byte_string<unc::latin1>(v1->artist);
        album = decode_byte_string<unc::latin1>(v1->album);
        year = decode_byte_string<unc::latin1>(v1->year);
        return;
    }
}

input_mpg123::~input_mpg123()
{
    if (handle != NULL) {
        mpg123_delete(handle);
    }
}

void input_mpg123::open(std::string path, pcm_format format)
{
    int err;
    handle = mpg123_new(NULL, &err);

    if (handle == NULL) {
        throw input_error(mpg123_plain_strerror(err));
    }

    if ((err = mpg123_open(handle, path.c_str())) != MPG123_OK) {
        throw input_error(mpg123_plain_strerror(err));
    }

    if ((err = mpg123_getformat(handle, NULL, NULL, NULL)) != MPG123_OK) {
        throw input_error(mpg123_plain_strerror(err));
    }

    read_metadata();

    /* force 16 bit format */
    int encoding = to_mpg123_encoding(format.encoding);

    mpg123_format_none(handle);

    if ((err = mpg123_format(handle, format.rate, format.channels, encoding)) != MPG123_OK)
    {
        throw input_error(mpg123_plain_strerror(err));
    }

    current_format = format;
}

void input_mpg123::close()
{
}

void input_mpg123::seek(double pos)
{
    int err;
    off_t off = mpg123_timeframe(handle, pos);

    if (off < 0) {
        throw input_error(mpg123_plain_strerror(off));
    }

    if ((err = mpg123_seek(handle, off, SEEK_SET)) != MPG123_OK) {
        throw input_error(mpg123_plain_strerror(err));
    }
}

pcm_position input_mpg123::tell()
{
    int err;
    double seconds;

    if ((err = mpg123_position(handle, 0, 0, NULL, NULL, &seconds, NULL)) != MPG123_OK) {
        throw input_error(mpg123_plain_strerror(err));
    }

    pcm_position meta;
    meta.current = seconds;
    meta.length = seconds + 100;

    return meta;
}

double input_mpg123::length()
{
    return 0;
}

void input_mpg123::update_current_format()
{
    long rate;
    int channels;
    int encoding;

    mpg123_getformat(handle, &rate, &channels, &encoding);

    pcm_format format;
    format.rate = rate;
    format.encoding = from_mpg123_encoding(encoding);
    format.channels = channels;

    current_format = format;
}

pcm_packet::ptr input_mpg123::readsome()
{
    unsigned char buffer[1024 * 8];
    int err;
    size_t done;

    pcm_packet::ptr pcm;

    err = mpg123_read(handle, buffer, sizeof(buffer), &done);

    switch (err) {
    case MPG123_NEW_FORMAT:
        update_current_format();
        break;
    case MPG123_DONE:
        /* stream is done */
        break;
    case MPG123_OK:
        /* proceed as normal */
        break;
    default:
        throw input_error(mpg123_plain_strerror(err));
    }

    const char* data = reinterpret_cast<const char*>(buffer);

    pcm.reset(new pcm_packet(current_format, data, done));
    /*pcm->info.channels = _channels;
    pcm->info.rate = _rate;
    pcm->info.endian = PCM_LE;*/

    return pcm;
}
