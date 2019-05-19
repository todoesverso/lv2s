/*
  Copyright 2019 Todoes Verso <todoesverso@gmail.com>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <math.h>
#include <stdlib.h>

#include "dbg.h"

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define AMP_URI "https://github.com/todoesverso/lv2s/todoes-amp"

typedef enum {
  AMP_GAIN   = 0,
  AMP_INPUT  = 1,
  AMP_OUTPUT = 2
} PortIndex;

typedef struct {
  // Port buffers
  const float* gain;
  const float* input;
  float*       output;
} Amp;

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
  (void)descriptor;
  (void)rate;
  (void)bundle_path;
  (void)features;

  Amp* amp = (Amp*)calloc(1, sizeof(Amp));
  if (!amp) {
    return NULL;
  }

  return (LV2_Handle)amp;
}

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
  Amp* amp = (Amp*)instance;

  switch ((PortIndex)port) {
  case AMP_GAIN:
    amp->gain = (const float*)data;
    break;
  case AMP_INPUT:
    amp->input = (const float*)data;
    break;
  case AMP_OUTPUT:
    amp->output = (float*)data;
    break;
  }
}


/** Define a macro for converting a gain in dB to a coefficient. */
#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g) * 0.05f) : 0.0f)

static void
run_mono(LV2_Handle instance, uint32_t n_samples)
{
  const Amp* amp = (const Amp*)instance;

  const float        gain   = *(amp->gain);
  const float* const input  = amp->input;
  float* const       output = amp->output;

  const float coef = DB_CO(gain);

  for (uint32_t pos = 0; pos < n_samples; pos++) {
    output[pos] = input[pos] * coef;
    DBG("%f %f %f\n", output[pos], input[pos], coef);
  }
}

static void
cleanup(LV2_Handle instance)
{
  free(instance);
}

static const void*
extension_data(const char* uri)
{
  (void)uri;
  return NULL;
}

static const LV2_Descriptor descriptor_mono = {
  AMP_URI,
  instantiate,
  connect_port,
  NULL,
  run_mono,
  NULL,
  cleanup,
  extension_data
};

static const LV2_Descriptor descriptor_stereo = {
  AMP_URI,
  instantiate,
  connect_port,
  NULL,
  run_mono,
  NULL,
  cleanup,
  extension_data
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:  return &descriptor_mono;
  case 1:  return &descriptor_stereo;
  default: return NULL;
  }
}
