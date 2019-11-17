/**
 * @addtogroup DPS310
 *
 */
/*@{*/
/** @file dps320.c
 *
 * @brief Implementation of DPS310 driver
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-11-16
 * @copyright Otso Jousimaa, License MIT.
 *
 */

#include "dps310.h"

/**
 * @brief returns the Product ID of the connected Dps310 sensor
 *
 * @return Product ID of DPS310, i.e. 0x00
 */
uint8_t dps310_get_product_id (void)
{
    return 1;
}

/*@}*/
