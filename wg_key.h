#ifndef WG_KEY_H
#define WG_KEY_H

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t wg_key[32];

typedef char wg_key_b64_string[45];

bool wg_key_is_zero (const wg_key key);

void wg_key_to_base64 (wg_key_b64_string base64, const wg_key key);

int wg_key_from_base64 (wg_key key, const wg_key_b64_string base64);

void wg_generate_public_key (wg_key public_key, const wg_key private_key);

void wg_generate_private_key (wg_key private_key);

void wg_generate_preshared_key (wg_key preshared_key);

#endif // WG_KEY_H
