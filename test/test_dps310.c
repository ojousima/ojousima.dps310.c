#include "unity.h"

#include "dps310.h"
#include "dps310_reg.h"
#include <stdbool.h>
#include <string.h>

static uint8_t dps310_registers[DPS310_SIM_TABLE_SIZE];
static uint32_t time_ms;
static uint32_t drdy_ms;
static int32_t bus_code;
static uint8_t efuse_writes;

static void check_reserved_registers(void)
{
    for(uint8_t ii = DPS310_RESERVED_START; ii <= DPS310_RESERVED_END; ii++)
    {
        TEST_ASSERT(0U == dps310_registers[ii]);
    }
}

static void init_dps_regs(void)
{
    memset(dps310_registers, 0, sizeof(dps310_registers));
    dps310_registers[DPS310_MEAS_CFG_REG] = DPS310_MEAS_CFG_BOOT_VAL;
    dps310_registers[DPS310_PROD_ID_REG]  = DPS310_PROD_ID_BOOT_VAL;
    dps310_registers[0x10] = 0xF9U; // C0 = -100
    dps310_registers[0x11] = 0xC0U;
    dps310_registers[0x12] = 0x01U; // C1 = 1
    dps310_registers[0x13] = 0xFBU; // C00 = -20 000
    dps310_registers[0x14] = 0x1EU;
    dps310_registers[0x15] = 0x00U;
    dps310_registers[0x16] = 0x03U; // C10 = 1000 
    dps310_registers[0x17] = 0xE8U;
    dps310_registers[0x18] = 0x07U; // C01 = 2000
    dps310_registers[0x19] = 0xD0U; 
    dps310_registers[0x1A] = 0xFFU; // C11 = -250 
    dps310_registers[0x1B] = 0x06U;
    dps310_registers[0x1C] = 0xFEU; // C20 = -350 
    dps310_registers[0x1D] = 0xA2U;
    dps310_registers[0x1E] = 0x00U; // C21 = 5 
    dps310_registers[0x1F] = 0x05U;         
    dps310_registers[0x20] = 0x00U; // C30 = 10
    dps310_registers[0x21] = 0x0AU;
}

static void assert_on_write_error(const uint8_t reg, const uint8_t value)
{
  bool valid_efuse = false;
  switch(reg)
  {
      case DPS310_PRES_CFG_REG:
          TEST_ASSERT(0U == (value & (~DPS310_PRES_CFG_WMASK)));
          break;
      case DPS310_TEMP_CFG_REG:
          TEST_ASSERT(0U == (value & (~DPS310_TEMP_CFG_WMASK)));
          break;
      case DPS310_MEAS_CFG_REG:
          TEST_ASSERT(0U == (value & (~DPS310_MEAS_CFG_WMASK)));
          break;
      case DPS310_CFG_REG:
          TEST_ASSERT(0U == (value & (~DPS310_CFG_WMASK)));
          break;
      case DPS310_RST_CFG_REG:
          TEST_ASSERT(0U == (value & (~DPS310_RST_CFG_WMASK)));
          break;
      case DPS310_EFUSE_0_REG:
          TEST_ASSERT((DPS310_EFUSE_0_VAL == value) || (0 == value));
          efuse_writes++;
          break;
      case DPS310_EFUSE_1_REG:
          TEST_ASSERT((DPS310_EFUSE_1_VAL == value) || (0 == value));
          efuse_writes++;
      case DPS310_EFUSE_2_REG:
          TEST_ASSERT((DPS310_EFUSE_2_VAL == value));
          efuse_writes++;
      default:
          TEST_ASSERT(0U);
          break;
  }
}

static void simulate_write_action(const uint8_t reg_addr, const uint8_t data)
{
  switch(reg_addr)
  {
    case DPS310_RST_CFG_REG:
        if(DPS310_SOFT_RST_VAL == (data & DPS310_SOFT_RST_MASK))
        {
          init_dps_regs();
          drdy_ms += DPS310_COEF_DELAY_MS;
        }
        break;

    default:
        break;
  }
}

static void assert_on_read_error(void)
{
    TEST_ASSERT(time_ms >= drdy_ms);
}

static int32_t mock_write(const void* const comm_ctx, 
                          const uint8_t reg_addr, 
                          const uint8_t * const data, 
                          const uint8_t data_len)
{
    for(uint8_t ii = 0; ii < data_len; ii++)
    {
        assert_on_write_error(reg_addr + ii, data[ii]);
        dps310_registers[reg_addr + ii] = data[ii];
        simulate_write_action(reg_addr + ii, data[ii]);
    }
    return bus_code;
}

static int32_t mock_read( const void* const comm_ctx, 
                          const uint8_t reg_addr, 
                          uint8_t * const data, 
                          const uint8_t data_len)
{
    for(uint8_t ii = 0; ii < data_len; ii++)
    {
        assert_on_read_error();
        data[ii] = dps310_registers[reg_addr + ii];
    }
    return bus_code;
}

static void mock_sleep(uint32_t ms)
{
    time_ms += ms;
}

static void reset_dps_ctx(dps310_ctx_t* const p_dps)
{
    //flags
  p_dps->init_fail = 0;

  p_dps->product_id = 0;
  p_dps->revision_id = 0;

  //settings
  p_dps->temp_mr = 0;
  p_dps->temp_osr = 0;
  p_dps->pres_mr = 0;
  p_dps->pres_osr = 0;

  // compensation coefficients
  p_dps->c00 = 0;
  p_dps->c10 = 0;
  p_dps->c01 = 0;
  p_dps->c11 = 0;
  p_dps->c20 = 0;
  p_dps->c21 = 0;
  p_dps->c30 = 0;

  // last measured scaled temperature (necessary for pressure compensation)
  p_dps->last_temp_scal = 0;
}

static dps310_ctx_t dps = 
{
  .sleep = &mock_sleep,
  .read  = &mock_read,
  .write = &mock_write
};

static void init_dps_coefs(void)
{

}

void setUp(void)
{
    init_dps_regs();
    init_dps_coefs();
    time_ms  = 0;
    drdy_ms  = 0;
    bus_code = 0;
    efuse_writes = 0;
    reset_dps_ctx(&dps);
}

void tearDown(void)
{
    check_reserved_registers();
}

void test_dps310_init_ok(void)
{
  drdy_ms += DPS310_POR_DELAY_MS;
  int32_t err_code = dps310_init(&dps);
  TEST_ASSERT(0x00U == dps.product_id);
  TEST_ASSERT(0x01U == dps.revision_id);
  // TODO: Check coefficients
  TEST_ASSERT(DPS310_MR_1 == dps.temp_mr);
  TEST_ASSERT(DPS310_OS_1 == dps.temp_osr);
  TEST_ASSERT(DPS310_MR_1 == dps.pres_mr);
  TEST_ASSERT(DPS310_OS_1 == dps.pres_osr);

  TEST_ASSERT(-100 == dps.c0);
  TEST_ASSERT(1 == dps.c1);
  TEST_ASSERT(-20000 == dps.c00);
  TEST_ASSERT(1000 == dps.c10);
  TEST_ASSERT(-350 == dps.c20);
  TEST_ASSERT(10 == dps.c30);
  TEST_ASSERT(2000 == dps.c01);
  TEST_ASSERT(-250 == dps.c11);
  TEST_ASSERT(5 == dps.c21);

  TEST_ASSERT(5U == efuse_writes);
  TEST_ASSERT(DPS310_MODE_STANDBY_VAL 
              == (dps310_registers[DPS310_MEAS_CFG_REG] & DPS310_MODE_MASK));
  TEST_ASSERT(DPS310_SOFT_RST_VAL == dps310_registers[DPS310_RST_CFG_REG]);
  TEST_ASSERT(0U == dps310_registers[DPS310_EFUSE_0_REG]);
  TEST_ASSERT(0U == dps310_registers[DPS310_EFUSE_1_REG]);
  TEST_ASSERT(DPS310_EFUSE_2_VAL == dps310_registers[DPS310_EFUSE_2_REG]);
  TEST_ASSERT(time_ms >= (DPS310_POR_DELAY_MS + DPS310_COEF_DELAY_MS));
  TEST_ASSERT(DPS310_SUCCESS == err_code);
}
