#ifndef __INPUT_MPG123_HPP__
#define __INPUT_MPG123_HPP__

#include "input_base.hpp"

#include <string>
#include <unc/unc.hpp>
#include <mpg123.h>

class pcm_meta;
class pcm_format;
class pcm_packet;
class input_base;

class input_mpg123 : public input_base {
public:
    input_mpg123();
    ~input_mpg123();
   
    virtual void open(std::string path, pcm_format);
    virtual void close();
    virtual void seek(double pos);
    virtual pcm_position tell();
    virtual double length();
   
    virtual std::shared_ptr<pcm_packet> readsome();
private:
    void update_current_format();
    void read_metadata();
    mpg123_handle* handle;
    pcm_format current_format;
   
    unc::ustring title;
    unc::ustring artist;
    unc::ustring album;
    unc::ustring year;
};

void initialize_mpg123();

#endif /* __INPUT_MPG123_HPP__ */
