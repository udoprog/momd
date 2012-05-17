#include "ao.hpp"

#include "output_error.hpp"
#include "plugin.hpp"
#include "pcm_format.hpp"
#include "pcm_packet.hpp"

#include "config.hpp"
#include <cstring>
#include <cerrno>

const char* ao_enodriver_string = "No driver corresponds to driver_id";
const char* ao_enotlive_string = "This driver is not a live input device";
const char* ao_ebadoption_string = "A valid option key has an invalid value";
const char* ao_eopendevice_string = "Cannot open the device";
const char* ao_efail_string = "General failure";

const char* ao_strerror(int err)
{
    switch (err) {
    case AO_ENODRIVER:
        return ao_enodriver_string;
    case AO_ENOTLIVE:
        return ao_enotlive_string;
    case AO_EBADOPTION:
        return ao_ebadoption_string;
    case AO_EOPENDEVICE:
        return ao_eopendevice_string;
    case AO_EFAIL:
    default:
        return ao_efail_string;
    }
}

output_base* ao__new();
void ao__initialize();

output_plugin_spec ao_spec = {
    &ao__initialize,
    &ao__new
};

DECLARE_OUTPUT_PLUGIN("ao", ao_spec)

output_base* ao__new() {
    return new output_ao();
}

void ao__initialize() {
    ao_initialize();
}

output_ao::output_ao()
  : device(NULL)
{
}

output_ao::~output_ao()
{
  close();
}

void output_ao::setup(libconfig::Setting& setting)
{
    int channels;
    int rate;
    int bps;
    std::string driver_string;

    if (setting.lookupValue("channels", channels)) {
        sample_format.channels = channels;
    }
    else {
        sample_format.channels = DEFAULT_GLOBAL_CHANNELS;
    }
    
    if (setting.lookupValue("rate", rate)) {
        sample_format.rate = rate;
    }
    else {
        sample_format.rate = 44100;
    }
    
    if (setting.lookupValue("bps", bps)) {
        sample_format.bits = bps;
    }
    else {
        sample_format.bits = 16;
    }
    
    sample_format.byte_format = AO_FMT_LITTLE;
    
    if (setting.lookupValue("driver", driver_string)) {
        driver_id = ao_driver_id(driver_string.c_str());
    }
    else {
        driver_id = ao_default_driver_id();
    }

    if (driver_id == -1) {
        throw output_error("Invalid driver");
    }
    
    if (setting.exists("options")) {
        libconfig::Setting& o = setting["options"];

        if (!o.isGroup()) {
            return;
        }

        int length = o.getLength();

        ao_option* current = NULL;
        options = NULL;

        for (int i = 0; i < length; i++) {
            libconfig::Setting& s = o[i];

            if (current == NULL) {
                options = current = new ao_option;
            }
            else {
                current->next = new ao_option;
                current = current->next;
            }

            std::string key = s.getName();
            std::string value = s;

            current->key = new char[key.size() + 1];
            current->value = new char[value.size() + 1];
            current->next = NULL;

            ::strncpy(current->key, key.c_str(), key.size() + 1);
            ::strncpy(current->value, value.c_str(), value.size() + 1);
        }
    }
}

void output_ao::close()
{
  if (device != NULL) {
    ao_close(device);
    free(device);
    device = NULL;
  }
}

bool output_ao::is_open() {
    return device != NULL;
}

void output_ao::open()
{
    /* connection already established */
    if (is_open()) {
        return;
    }

    device = ao_open_live(driver_id, &sample_format, NULL);

    if (device == NULL) {
        throw output_error(ao_strerror(errno));
    }
}

pcm_format output_ao::format()
{
    pcm_format format;
    format.encoding = PCM_SIGNED_16;
    format.rate = sample_format.rate;
    format.channels = sample_format.channels;
    return format;
}

void output_ao::write(pcm_packet::ptr pcm)
{
    ao_play(device, const_cast<char*>(pcm->data()), pcm->size());
}
