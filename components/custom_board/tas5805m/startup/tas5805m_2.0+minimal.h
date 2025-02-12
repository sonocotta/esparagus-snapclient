#pragma once

#include "../include/tas5805m_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_META_SWITCH (255)
#define CFG_META_DELAY (254)
#define CFG_META_BURST (253)
#define CFG_END_1 (0Xaa)
#define CFG_END_2 (0Xcc)
#define CFG_END_3 (0Xee)

typedef struct {
  uint8_t offset;
  uint8_t value;
} tas5805m_cfg_reg_t;

static const tas5805m_cfg_reg_t tas5805m_registers[] = {
// RESET
    { 0x00, 0x00 },
    { 0x7f, 0x00 },
    { 0x03, 0x02 },
    { 0x01, 0x11 },
    { 0x03, 0x02 },
    { CFG_META_DELAY, 10 },
    { 0x03, 0x00 },
    { 0x46, 0x01 },
    { 0x03, 0x02 },
    { 0x61, 0x0b },
    { 0x60, 0x01 },
    { 0x7d, 0x11 },
    { 0x7e, 0xff },
    { 0x00, 0x01 },
    { 0x51, 0x05 },
// Register Tuning
    { 0x00, 0x00 },
    { 0x7f, 0x00 },
    { 0x02, 0x00 },
    { 0x30, 0x00 },
    { 0x4c, 0x30 },
    { 0x53, 0x00 },
    { 0x54, 0x00 },
    { 0x03, 0x03 },
    { 0x78, 0x80 },
};

#ifdef __cplusplus
}
#endif