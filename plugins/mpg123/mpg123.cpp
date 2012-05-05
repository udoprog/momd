#include "mpg123.hpp"

#include "plugin.hpp"
#include "input_error.hpp"
#include "pcm_meta.hpp"
#include "pcm_packet.hpp"

input_base* mpg123__new();
void mpg123__initialize();
bool mpg123__check(const char* path, const char* ext);

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

bool mpg123__check(const char* path, const char* ext)
{
    std::string string_path(path);
    std::string string_ext(ext);

    if (mp3_ext.compare(string_ext) == 0) {
        return true;
    }

    if (mp4_ext.compare(string_ext) == 0) {
        return true;
    }

    return false;
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
    : handle(NULL), _channels(0), _rate(0), _encoding(0),
      _length(0.0)
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

void input_mpg123::open(std::string path)
{
    int err;
    int encoding;
    handle = mpg123_new(NULL, &err);

    if (handle == NULL) {
        throw input_error(mpg123_plain_strerror(err));
    }

    if ((err = mpg123_open(handle, path.c_str())) != MPG123_OK) {
        throw input_error(mpg123_plain_strerror(err));
    }

    if ((err = mpg123_getformat(handle, &_rate, &_channels, &encoding)) != MPG123_OK) {
        throw input_error(mpg123_plain_strerror(err));
    }

    read_metadata();

    /* force 16 bit format */
    encoding = MPG123_ENC_SIGNED_16;

    mpg123_format_none(handle);
    mpg123_format(handle, _rate, _channels, encoding);

    double seconds_left;

    if ((err = mpg123_position(handle, 0, 0, NULL, NULL, NULL, &seconds_left)) != MPG123_OK) {
        throw input_error(mpg123_plain_strerror(err));
    }

    this->_encoding = encoding;
    this->_length = seconds_left;
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

pcm_meta input_mpg123::tell()
{
    int err;
    double seconds;

    if ((err = mpg123_position(handle, 0, 0, NULL, NULL, &seconds, NULL)) != MPG123_OK) {
        throw input_error(mpg123_plain_strerror(err));
    }

    pcm_meta meta;
    meta.current = seconds;
    meta.length = seconds + 100;

    return meta;
}

double input_mpg123::length()
{
    return _length;
}

pcm_info input_mpg123::info()
{
    pcm_info info;
    info.rate = _rate;
    info.channels = _channels;
    info.endian = PCM_LE;
    info.bps = 16;
    return info;
}

pcm_packet::ptr input_mpg123::readsome()
{
    unsigned char buffer[1024 * 8];
    int status;
    size_t done;

    pcm_packet::ptr pcm;

    status = mpg123_read(handle, buffer, sizeof(buffer), &done);

    if (status != MPG123_OK) {
        return pcm;
    }

    pcm.reset(new pcm_packet(reinterpret_cast<char*>(buffer), done));
    /*pcm->info.channels = _channels;
    pcm->info.rate = _rate;
    pcm->info.endian = PCM_LE;*/

    return pcm;
}
