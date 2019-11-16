/**
 * @defgroup DPS310 DPS310 driver
 * @brief Embedded C driver for DPS310 pressure and temperature sensor.
 *
 */
/*@{*/
/**
 * @file dps310.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-11-16
 * @copyright Otso Jousimaa, License MIT
 *
 *
 */
#ifndef DPS310_H
#define DPS310_H

#include <stdint.h>

#define DPS310_PRODUCT_ID_VAL 0u //!< Product ID read from DPS310 register-

  /**
   * @brief returns the Product ID of the connected Dps310 sensor
   *
   * @return Product ID of DPS310, i.e. 0x00
   */
  uint8_t dps310_get_product_id(void);

#endif // DPS310_H
/*}*/