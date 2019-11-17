#include "unity.h"

#include "dps310.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_dps310_get_product_id(void)
{
  TEST_IGNORE_MESSAGE("Setting up project, ignore results")
  uint8_t id = dps310_get_product_id();
  TEST_ASSERT(DPS310_PRODUCT_ID_VAL == id);
}
