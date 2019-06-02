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
#define MAXDELAYSEC (2)

typedef enum {
  IN_L            = 0,
  IN_R            = 1,
  OUT_L           = 2,
  OUT_R           = 3,
  DELAY           = 4,
  FEEDBACK        = 5,
  FEEDBACK_BOUNCE = 6,
  FEEDBACK_REV    = 7,
  REP1            = 8,
  REP2            = 9
} PortIndex;

typedef struct {
  // Port buffers
  const float*  in_l;
  const float*  in_r;
  float*        out_l;
  float*        out_r;

  int           buff_size;
  float*        buffer_l;
  float*        buffer_r;
  int           rate;
  int           w_ptr_l;
  int           w_ptr_r;
  int           r_ptr_l;
  int           r_ptr_r;
  int           b_ptr_l;
  int           b_ptr_r;
  int           rep1_ptr_l;
  int           rep1_ptr_r;
  int           rep2_ptr_l;
  int           rep2_ptr_r;
  float*        delay;
  float*        feedback;
  float*        feedback_bounce;
  float*        feedback_rev;
  float*        rep1;
  float*        rep2;
} Dly;

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
  (void)descriptor;
  (void)bundle_path;
  (void)features;

  Dly* dly = (Dly*)calloc(1, sizeof(Dly));
  if (!dly) {
    return NULL;
  }
  dly->buff_size = (int) rate * MAXDELAYSEC;
  dly->buffer_l = (float*)calloc(dly->buff_size, sizeof(float));
  if (!dly->buffer_l) {
    return NULL;
  }

  dly->buffer_r = (float*)calloc(dly->buff_size, sizeof(float));
  if (!dly->buffer_r) {
    return NULL;
  }

  dly->w_ptr_l = 0;
  dly->w_ptr_r = 0;
  dly->r_ptr_l = 0;
  dly->r_ptr_r = 0;
  dly->b_ptr_l = 0;
  dly->b_ptr_r = 0;
  dly->rep1_ptr_l = 0;
  dly->rep1_ptr_r = 0;
  dly->rep2_ptr_l = 0;
  dly->rep2_ptr_r = 0;
  dly->rate = (int)rate;

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
    self->delay = (float*)data;
    break;
  case IN_L:
    self->in_l = (const float*)data;
    break;
  case IN_R:
    self->in_r = (const float*)data;
    break;
  case OUT_L:
    self->out_l = (float*)data;
    break;
  case OUT_R:
    self->out_r = (float*)data;
    break;
  case FEEDBACK:
    self->feedback = (float*)data;
    break;
  case FEEDBACK_BOUNCE:
    self->feedback_bounce = (float*)data;
    break;
  case FEEDBACK_REV:
    self->feedback_rev = (float*)data;
    break;
  case REP1:
    self->rep1 = (float*)data;
    break;
  case REP2:
    self->rep2 = (float*)data;
    break;
  }
}

static void
activate (LV2_Handle instance) {
  (void)instance;
}

#define INCREMENT_PTRS \
		self->r_ptr = (self->r_ptr + 1) % self->buff_size; \
    self->w_ptr = (self->w_ptr + 1) % self->buff_size;
#define INCREMENT_WPTRS \
    self->w_ptr_l = (self->w_ptr_l + 1) % self->buff_size; \
    self->w_ptr_r = (self->w_ptr_r + 1) % self->buff_size;

static void
run(LV2_Handle instance, uint32_t n_samples)
{
  Dly* self = (Dly*)instance;

  const int          rate   = self->rate;
  float              delay  = *self->delay;
  const float* const in_l   = self->in_l;
  const float* const in_r   = self->in_r;
  float* const       out_l  = self->out_l;
  float* const       out_r  = self->out_r;
  float              fdbk   = *self->feedback;
  float              fdbk_b = *self->feedback_bounce;
  float              fdbk_r = *self->feedback_rev;
  float              rep1   = *self->rep1;
  float              rep2   = *self->rep2;
  
  int delaySample = (int)(delay*rate);

  for (uint32_t pos = 0; pos < n_samples; pos++) {
    self->r_ptr_l = self->w_ptr_l - delaySample;
    self->r_ptr_r = self->w_ptr_r - delaySample;
    self->rep1_ptr_l = (int) self->r_ptr_l + (delaySample*rep1);
    self->rep1_ptr_r = (int) self->r_ptr_r + (delaySample*rep1);
    self->rep2_ptr_l = (int) self->r_ptr_l + (delaySample*rep2);
    self->rep2_ptr_l = (int) self->r_ptr_l + (delaySample*rep2);

    self->b_ptr_l = self->buff_size - (self->w_ptr_l + delaySample);
    self->b_ptr_r = self->buff_size - (self->w_ptr_r + delaySample);
    if (self->r_ptr_l < 0){
      self->r_ptr_l += self->buff_size;
    }
    if (self->r_ptr_r < 0){
      self->r_ptr_r += self->buff_size;
    }

    if (self->b_ptr_l < 0) {
      self->b_ptr_l += self->buff_size;
    }
    if (self->b_ptr_r < 0) {
      self->b_ptr_r += self->buff_size;
    }

    if (self->rep1_ptr_l < 0) {
      self->rep1_ptr_l += self->buff_size;
    }
    if (self->rep1_ptr_r < 0) {
      self->rep1_ptr_r += self->buff_size;
    }

    if (self->rep2_ptr_l < 0) {
      self->rep2_ptr_l += self->buff_size;
    }
    if (self->rep2_ptr_r < 0) {
      self->rep2_ptr_r += self->buff_size;
    }

    /* Original signal*/
    self->buffer_l[self->w_ptr_l] = in_l[pos]; 
    self->buffer_r[self->w_ptr_r] = in_r[pos]; 
    /* Bounced signal*/
    self->buffer_l[self->w_ptr_l] += (self->buffer_l[self->r_ptr_l] * fdbk_b);
    self->buffer_r[self->w_ptr_r] += (self->buffer_r[self->r_ptr_r] * fdbk_b);
    /* Reverse signal*/
    self->buffer_l[self->w_ptr_l] += (self->buffer_l[self->b_ptr_l] * fdbk_r);
    self->buffer_r[self->w_ptr_r] += (self->buffer_r[self->b_ptr_r] * fdbk_r);

    /* Output signal - Original  */
    out_l[pos] = in_l[pos];
    out_r[pos] = in_r[pos];
    /* Output signal - Delayed signal  */
    out_l[pos] += (self->buffer_l[self->r_ptr_l] * fdbk);
    out_r[pos] += (self->buffer_r[self->r_ptr_r] * fdbk);

    if (rep1 > 0){
      out_l[pos] += (self->buffer_l[self->rep1_ptr_l] * fdbk);
      out_r[pos] += (self->buffer_r[self->rep1_ptr_r] * fdbk);
    }
    if (rep2 > 0) {
      out_l[pos] += (self->buffer_l[self->rep2_ptr_l] * fdbk);
      out_r[pos] += (self->buffer_r[self->rep2_ptr_r] * fdbk);
    }

    INCREMENT_WPTRS;
  }
}


static void
cleanup(LV2_Handle instance)
{
  Dly* self = (Dly*)instance;
  free(self->buffer_l);
  free(self->buffer_r);
  free(self);
}

static const void*
extension_data(const char* uri)
{
  (void)uri;
  return NULL;
}

static const LV2_Descriptor descriptor = {
  URI,
  instantiate,
  connect_port,
  NULL,
  run,
  NULL,
  cleanup,
  extension_data
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:  return &descriptor;
  default: return NULL;
  }
}
