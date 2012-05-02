#include "output/pulse.hpp"
#include "io.hpp"

#include "config.hpp"
#include <cstring>

output_pulse::output_pulse()
  : simple(NULL), server(NULL), name(NULL), stream(NULL)
{
}

output_pulse::~output_pulse()
{
  close();
}

void output_pulse::setup(libconfig::Setting& setting)
{
  _spec.reset(new pa_sample_spec);

  _spec->format = PA_SAMPLE_S16LE;

  int channels;
  int rate;

  if (setting.lookupValue("channels", channels)) {
    _spec->channels = channels;
  }
  else {
    _spec->channels = DEFAULT_GLOBAL_CHANNELS;
  }

  if (setting.lookupValue("rate", rate)) {
    _spec->rate = rate;
  }
  else {
    _spec->rate = 44100;
  }

  if (!setting.lookupValue("server", this->server))  {
    this->server = NULL;
  }

  if (!setting.lookupValue("name", this->name))  {
    this->name = PROGRAM_NAME;
  }

  if (!setting.lookupValue("stream", this->stream))  {
    if (setting.isGroup()) {
      this->stream = setting.getName();
    }
  }
}

void output_pulse::close()
{
  int error;

  if (simple != NULL) {
    pa_simple_drain(simple, &error);
    pa_simple_free(simple);
    simple = NULL;
  }
}

bool output_pulse::is_open() {
  return simple != NULL;
}

void output_pulse::open()
{
  /* connection already established */
  if (is_open()) {
    return;
  }

  if (!_spec) {
    throw output_error("pulseaudio not setup");
  }

  int error;

  simple = pa_simple_new(this->server, this->name, PA_STREAM_PLAYBACK, NULL, this->stream, _spec.get(), NULL, NULL, &error);

  if (simple == NULL)
  {
    throw output_error("could not establish connection", pa_strerror(error));
  }
}

pcm_info output_pulse::info()
{
  pcm_info info;
  info.endian = PCM_LE;
  info.rate = _spec->rate;
  info.channels = _spec->channels;
  info.bps = 16;
  return info;
}

void output_pulse::write(pcm_packet_ptr pcm)
{
  int error;
  int r;

  if (simple == NULL) {
    throw output_error("connection not established");
  }

  r = pa_simple_write(simple, pcm->data(), pcm->size(), &error);

  if (r < 0) {
    throw output_error("could not write to output", pa_strerror(error));
  }
}
