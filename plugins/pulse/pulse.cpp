#include "pulse.hpp"

#include "output_error.hpp"
#include "plugin.hpp"
#include "pcm_format.hpp"
#include "pcm_packet.hpp"

#include "config.hpp"

#include <cstring>

#include <libconfig.h++>

output_base* pulse__new();
void pulse__initialize();

output_plugin_spec pulse_spec = {
    &pulse__initialize,
    &pulse__new
};

DECLARE_OUTPUT_PLUGIN("pulse", pulse_spec)

output_base* pulse__new() {
    return new output_pulse();
}

void pulse__initialize() {
    /* pass */
}

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
  spec.format = PA_SAMPLE_S16LE;

  int channels;
  int rate;

  if (setting.lookupValue("channels", channels)) {
    spec.channels = channels;
  }
  else {
    spec.channels = DEFAULT_GLOBAL_CHANNELS;
  }

  if (setting.lookupValue("rate", rate)) {
    spec.rate = rate;
  }
  else {
    spec.rate = 44100;
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
  if (simple != NULL) {
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

  int error;

  simple = pa_simple_new(this->server, this->name, PA_STREAM_PLAYBACK, NULL, this->stream, &spec, NULL, NULL, &error);

  if (simple == NULL)
  {
    throw output_error("could not establish connection");
  }
}

pcm_format output_pulse::format()
{
  pcm_format format;
  format.encoding = PCM_SIGNED_16;
  format.rate = spec.rate;
  format.channels = spec.channels;
  return format;
}

void output_pulse::write(pcm_packet::ptr pcm)
{
  int error;
  int r;

  if (simple == NULL) {
    throw output_error("connection not established");
  }

  r = pa_simple_write(simple, pcm->data(), pcm->size(), &error);

  if (r < 0) {
    throw output_error("could not write to output");
  }
}
