/*******************************************************************************
 *
 * Copyright (c) 2011, 2012, 2013, 2014, 2015 Olaf Bergmann (TZI) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v. 1.0 which accompanies this distribution.
 *
 * The Eclipse Public License is available at http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at 
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Olaf Bergmann  - initial API and implementation
 *    Hauke Mehrtens - memory optimization, ECC integration
 *
 *******************************************************************************/

/** 
 * @file prng.h
 * @brief Pseudo Random Numbers
 */

#ifndef _DTLS_PRNG_H_
#define _DTLS_PRNG_H_

#include "stm32f7xx_hal.h"
#include "tinydtls.h"
#include "conf_lwm2m.h"

/** 
 * @defgroup prng Pseudo Random Numbers
 * @{
 */

#ifdef WITH_FREERTOS
#include <string.h>
extern RNG_HandleTypeDef hrng;

static inline int dtls_prng(unsigned char *buf, size_t len) {
	unsigned int val_32bit;
	while (len--) {
		if(HAL_RNG_GenerateRandomNumber(&hrng, &val_32bit) == HAL_ERROR){
			return -1;
		}
		*buf++ = val_32bit & 0xFF;
	}
	return 1;
}

static inline void
dtls_prng_init(void) {
	if(HAL_RNG_GetState(&hrng) != HAL_RNG_STATE_READY){
		HAL_RNG_Init(&hrng);	//Initialized if not already
	}
}
#elif defined(WITH_CONTIKI) /* WITH_CONTIKI */
#include <string.h>
#include "random.h"

#ifdef HAVE_PRNG
static inline int
dtls_prng(unsigned char *buf, size_t len)
{
	return contiki_prng_impl(buf, len);
}
#else
/**
 * Fills \p buf with \p len random bytes. This is the default
 * implementation for prng().  You might want to change prng() to use
 * a better PRNG on your specific platform.
 */
static inline int
dtls_prng(unsigned char *buf, size_t len) {
  unsigned short v = random_rand();
  while (len > sizeof(v)) {
    memcpy(buf, &v, sizeof(v));
    len -= sizeof(v);
    buf += sizeof(v);
    v = random_rand();
  }

  memcpy(buf, &v, len);
  return 1;
}
#endif /* HAVE_PRNG */

static inline void
dtls_prng_init(unsigned short seed) {
  /* random_init() messes with the radio interface of the CC2538 and
   * therefore must not be called after the radio has been
   * initialized. */
#ifndef CONTIKI_TARGET_CC2538DK
	random_init(seed);
#endif
}
#else
#include <stdlib.h>

/**
 * Fills \p buf with \p len random bytes. This is the default
 * implementation for prng().  You might want to change prng() to use
 * a better PRNG on your specific platform.
 */
static inline int
dtls_prng(unsigned char *buf, size_t len) {
  while (len--)
    *buf++ = rand() & 0xFF;
  return 1;
}

static inline void
dtls_prng_init(unsigned short seed) {
	srand(seed);
}
#endif /* WITH_CONTIKI */

/** @} */

#endif /* _DTLS_PRNG_H_ */
