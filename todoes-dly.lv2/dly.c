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

#define URI "https://github.com/todoesverso/lv2s/todoes-dly"

#define MAXDELAY (192001)

typedef enum {
  DELAY = 0,
  IN    = 1,
  OUT   = 2
} PortIndex;

typedef struct {
  // Port buffers
  const float* delay;
  const float* in;
  float*       out;

  float buffer[MAXDELAY];
  int   w_ptr;
  int   r_ptr;

} Dly;

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

  Dly* dly = (Dly*)calloc(1, sizeof(Dly));
  if (!dly) {
    return NULL;
  }
  dly->r_ptr = 0;
  dly->r_ptr = 0;

  return (LV2_Handle)dly;
}

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
  Dly* self = (Dly*)instance;

  switch ((PortIndex)port) {
  case DELAY:
    self->delay = (const float*)data;
    break;
  case IN:
    self->in = (const float*)data;
    break;
    break;
  case OUT:
    self->out = (float*)data;
    break;
  }
}


static void
activate (LV2_Handle instance) {
  Dly* self = (Dly*)instance;
  for (int i = 0; i<MAXDELAY; i++){
    self->buffer[i] = 0.f;
  }

}

#ifndef MAX
#define MAX(A,B) ( (A) > (B) ? (A) : (B) )
#endif
#ifndef MIN
#define MIN(A,B) ( (A) < (B) ? (A) : (B) )
#endif

#define INCREMENT_PTRS \
		self->r_ptr = (self->r_ptr + 1) % MAXDELAY; \
    self->w_ptr = (self->w_ptr + 1) % MAXDELAY;
#define INCREMENT_WPTRS \
    self->w_ptr = (self->w_ptr + 1) % MAXDELAY;
#define INCREMENT_RPTRS \
    self->w_ptr = (self->r_ptr + 1) % MAXDELAY;

static void
run_mono(LV2_Handle instance, uint32_t n_samples)
{
  Dly* self = (Dly*)instance;

  const float        delay  = MAX(0, MIN((MAXDELAY - 1), *(self->delay)));
  const float* const in     = self->in;
  float* const       out    = self->out;
  int                feedback  = 0.5f;

  
  int delaySample = (int)(delay);

  for (uint32_t pos = 0; pos < n_samples; pos++) {
    self->buffer[self->w_ptr] = in[pos];

    self->r_ptr = self->w_ptr - delaySample;
    if (self->r_ptr < 0){
      self->r_ptr += MAXDELAY;
    }

    out[pos] = in[pos] + (self->buffer[self->r_ptr]*0.9);

    INCREMENT_WPTRS;
    DBG("%f %f %d %d\n", out[pos], in[pos], self->w_ptr, self->r_ptr);
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
  URI "#mono",
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
  default: return NULL;
  }
}
