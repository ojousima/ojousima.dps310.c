/**
 * @defgroup DPS310 DPS310 driver
 * @brief Embedded C driver for DPS310 pressure and temperature sensor.
 *
 */
/** @{ */
/**
 * @file dps310.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-11-23
 * @copyright Ruuvi Innovations, License MIT
 *
 */
#ifndef DPS310_H
#define DPS310_H

#include <stdint.h>

#define DPS310_PRODUCT_ID_VAL (0U) //!< Product ID read from DPS310 register

/**
 * @brief Sleep at least this many milliseconds.
 *
 * Function is allowed to sleep for longer, but not shorter time.
 */
typedef void(*dps310_sleep)(const uint32_t ms);

/**
 * @brief Read data from bus.
 *
 * If the read function returns non-zero value, it will get returned to caller in application. 
 *
 * @param[in] comm_ctx A pointer to context for platform. E.g. I2C address or SPI SS pin.
 * @param[in] reg_addr Address of register to read. 
 * @param[out] data Pointer to which data will be written. Must be data_len bytes long. 
 * @param[in] data_len Length of data.
 * @return 0 on success, non-zero error code on error.
 */
typedef uint32_t(*dps310_read)(const void* const comm_ctx, const uint8_t reg_addr, uint8_t * const data, const uint8_t data_len);

/**
 * @brief Write data from bus.
 *
 * If the write function returns non-zero value, it will get returned to caller in application. 
 *
 * @param[in] comm_ctx A pointer to context for platform. E.g. I2C address or SPI SS pin.
 * @param[in] reg_addr Address of register to write. 
 * @param[in] data Pointer to which data will be written. Must be data_len bytes long. 
 * @param[in] data_len Length of data.
 * @return 0 on success, non-zero error code on error.
 */
typedef uint32_t(*dps310_write)(const void* const comm_ctx, const uint8_t reg_addr, const uint8_t * const data, const uint8_t data_len);

typedef enum {
    DPS310_MR_INVALID = 0,
    DPS310_MR_1,
    DPS310_MR_2,
    DPS310_MR_4,
    DPS310_MR_8,
    DPS310_MR_16,
    DPS310_MR_32,
    DPS310_MR_64,
    DPS310_MR_128
}dps310_mr_t;

typedef enum {
    DPS310_OS_INVALID = 0,
    DPS310_OS_1,
    DPS310_OS_2,
    DPS310_OS_4,
    DPS310_OS_8,
    DPS310_OS_16,
    DPS310_OS_32,
    DPS310_OS_64,
    DPS310_OS_128
}dps310_os_t;

typedef struct 
{
  //flags
  uint8_t init_fail;

  uint8_t product_id;
  uint8_t revision_id;

  //settings
  dps310_mr_t temp_mr;
  dps310_os_t temp_osr;
  dps310_mr_t pres_mr;
  dps310_os_t pres_osr;

  // compensation coefficients
  int32_t c00;
  int32_t c10;
  int32_t c01;
  int32_t c11;
  int32_t c20;
  int32_t c21;
  int32_t c30;

  // last measured scaled temperature (necessary for pressure compensation)
  float last_temp_scal;
  
  // Communication interface.
  void* const comm_ctx;
  const dps310_sleep sleep;
  const dps310_read  read;
  const dps310_write write;
}dps310_ctx_t;

/**
 * @brief Returns the Product ID of the connected DPS310 sensor.
 *
 * @return Product ID of DPS310, i.e. 0x00.
 */
int32_t dps310_get_product_id (dps310_ctx_t* const ctx);

/**
 * @brief Returns the Revision ID of the connected DPS310 sensor.
 *
 * @return Revision ID of DPS310.
 */
int32_t dps310_get_revision_id(dps310_ctx_t* const ctx);

/**
 * @brief Set the DPS310 to standby mode.
 *
 * @return status code, 0 on success.
 */
int32_t dps310_standby(dps310_ctx_t* const ctx);

  /**
   * @brief Perform one temperature measurement.
   *
   * Uses the oversampling rate of given context and blocks until measurement is ready.
   *
   * @param[out] result        Peference to a float where the result will be written.
   * @return status code, 0 on success.
   */
int32_t dps310_measure_temp_once_sync(dps310_ctx_t* const ctx, float * const result);

  /**
   * @brief Starts a single temperature measurement.
   *
   * @return status code, 0 on success.
   */
  int32_t dps310_measure_temp_once_async(dps310_ctx_t* const ctx);

  /**
   * @brief Perform one pressure measurement.
   *
   * @param[out] result A poitner to a float value where the result will be written.
   * @return status code, 0 on success.
   */
  int32_t dps310_measure_pressure_once_sync(dps310_ctx_t* const ctx, float * const result);

  /**
   * @brief Start a single pressure measurement.
   *
   * @return status code, 0 on success.
   */
  int32_t dps310_measure_pressure_once_async(dps310_ctx_t* const ctx);

  /**
   * @brief Get the result async measurement, in C or Pa. 
   *
   * @param[out] result   A pointer to a float value where the result will be written.
   * @return status code, 0 on success.
   */
  int32_t dps310_get_single_result(float const * result);
  
  /**
   * @brief Starts a continuous temperature and pressure measurement.
   *  
   * Mmeasurement rate and oversampling rate for temperature and pressure measurement are read from the context.
   *
   * @param tempMr        measure rate for temperature
   * @param tempOsr       oversampling rate for temperature
   * @param prsMr         measure rate for pressure
   * @param prsOsr        oversampling rate for pressure
   * @return status code, 0 on success.
   */
  int32_t dps310_measure_continuous_async(dps310_ctx_t* const ctx);

  /**
   * @brief Get the interrupt status flag of the FIFO.
   *
   * @retval 1 if the FIFO is full and caused an interrupt.
   * @retval 0 if the FIFO is not full or FIFO interrupt is disabled.
   * @retval -1 on error.
   */
  int32_t dps310_get_int_fifo_full(dps310_ctx_t* const ctx);

  /**
   * @brief Get the interrupt status flag that indicates a finished temperature measurement.
   *
   * @retval 1 if the temperature measurement is ready and and caused an interrupt.
   * @retval 0 if the temperature measurement is not ready or temperature interrupt is disabled.
   * @retval -1 on error.
   */
  int32_t dps310_get_int_temp_ready(dps310_ctx_t* const ctx);

  /**
   * @brief Get the interrupt status flag that indicates a finished pressure measurement.
   *
   * @retval 1 if the temperature measurement is ready and and caused an interrupt.
   * @retval 0 if the temperature measurement is not ready or temperature interrupt is disabled.
   * @retval -1 on error.
   */
  int32_t dps310_get_int_pres_ready(dps310_ctx_t* const ctx);

  
  /**
   * @brief Get results stored to FIFO in continuous mode, oldest measurement first.
   *  
   * @param[out] temp A pointer to buffer where temperature results will be placed to, in C. 
   * @param[out] pres A pointer to buffer where pressure results will be placed to, in Pa.
   * @param[in,out] count Input: Number of elements in buffer. Output: number of read elements, 0 if no elements were read.
   * @return status code, 0 on success.
   */
  int32_t dps310_get_cont_results(dps310_ctx_t* const ctx, float * const temp, float * const pres, uint8_t * const count);

/**
 * @brief Initialize DPS310 instance. 
 *
 * Before calling this function you must setup the function pointers and the communication context. 
 * On initialization, sensor reads product ID, revision, calibration coefficients and writes the efuse workaround to IC.
 * After initialization, sensor is placed in standby, with oversampling and measurement rate configured to 1. 
 *
 * @param[in,out] ctx Input: Context to initialize. Output: Values read from physical sensor are entered into context. 
 * @return Status code, 0 on success. 
 * 
 */
  int32_t dps310_init(dps310_ctx_t* const ctx);

/**
 * @brief Uninitialize DPS310 instance. 
 *
 * After uninitialization, sensor is placed in standby, with oversampling configured to 1. 
 *
 * @param[in,out] ctx Input: context to uninitialize. Output: State of context after uninit.
 * @return Status code, 0 on success. 
 * 
 */
  int32_t dps310_uninit(dps310_ctx_t* const ctx);

/**
 * @brief Configure temperature measurement.
 *
 * @note (Pressure + temperature) * (oversampling * rate) must be lower than 620, 
 * e.g. temperature OS 1 mr 4 and pressure OS 128 and MR 4 
 * => temperature 4 OSMR + pressure 512 OSMR => 516 => ok. 
 * Full rationale is that sensor must be able to finish measurements in one second, 
 * and 128 measurements (per value) take 206.8 ms. 
 *
 * @param[in,out] ctx Input: Context of sensor. Output: Context after configuration.
 * @param[in] temp_mr Measurement rate.
 * @param[in] temp_osr Oversampling per measurement.
 */
  int32_t dps310_config_temp(dps310_ctx_t* const ctx, const dps310_mr_t temp_mr, const dps310_os_t temp_osr);

/**
 * @brief Configure pressure measurement.
 *
 * @note (Pressure + temperature) * (oversampling * rate) must be lower than 620, 
 * e.g. temperature OS 1 mr 4 and pressure OS 128 and MR 4 
 * => temperature 4 OSMR + pressure 512 OSMR => 516 => ok. 
 * Full rationale is that sensor must be able to finish measurements in one second, 
 * and 128 measurements (per value) take 206.8 ms. 
 *
 * @param[in,out] ctx Input: Context of sensor. Output: Context after configuration.
 * @param[in] pres_mr Measurement rate.
 * @param[in] pres_osr Oversampling per measurement.
 *
 */
  int32_t dps310_config_pres(dps310_ctx_t* const ctx, const dps310_mr_t pres_mr, const dps310_os_t pres_osr);

  /** 
   * @brief Configure DPS310 interrupts.
   *
   *
   * @return Status code, 0 on success.
   */
  int32_t dps310_set_int_sources(dps310_ctx_t* const ctx, const uint8_t intr_source, const uint8_t polarity);


/** Workaround hardware issue in some devices. Call in init of every device. */
static int32_t dps310_workaround_efuse(dps310_ctx_t* const ctx);
/** Read coefficients of device */
static int32_t dps310_read_coeffs(dps310_ctx_t* const ctx);
/** Clear out FIFO readings */
static int32_t dps310_flush_fifo(dps310_ctx_t* const ctx);
/** Calculate temperature from raw ADC value. */
static float dps310_calc_temp(dps310_ctx_t* const ctx, const int32_t raw);
/** Calculate pressure from raw ADC value. */
static float dps310_calc_pres(dps310_ctx_t* const ctx, const int32_t raw);

#endif // DPS310_H
/** @} */