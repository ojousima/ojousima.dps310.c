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
#include "dps310_reg.h"

static int32_t twos_complement (const uint32_t value, const uint8_t bits)
{
    const int32_t max_signed_value_exclusive = (int32_t) (1U << (bits - 1U));
    int32_t complement = value;

    if (value >= max_signed_value_exclusive)
    {
        complement -= 2 * max_signed_value_exclusive;
    }

    return complement;
}

static int32_t read_coefs (dps310_ctx_t * const ctx)
{
    uint8_t coefs[DPS310_COEF_REG_LEN] = {0};
    int32_t reg_read_ret = ctx->read (ctx->comm_ctx, DPS310_COEF_START_REG,
                                      coefs, DPS310_COEF_REG_LEN);
    uint32_t reg_val = ( ( (uint32_t) coefs[DPS310_COEF_C0_OFFSET] * 256U)
                         + ( (uint32_t) coefs[DPS310_COEF_C0_OFFSET + 1U]))
                       >> DPS310_COEF_C0_SHIFT;
    ctx->c0 = twos_complement (reg_val, DPS310_COEF_C0_LEN_BITS);
    reg_val = ( ( (uint32_t) coefs[DPS310_COEF_C1_OFFSET] * 256U)
                + ( (uint32_t) coefs[DPS310_COEF_C1_OFFSET + 1U]))
              & ( (1U << DPS310_COEF_C1_LEN_BITS) - 1U);
    ctx->c1 = twos_complement (reg_val, DPS310_COEF_C1_LEN_BITS);
    reg_val = ( ( (uint32_t) coefs[DPS310_COEF_C00_OFFSET] * 256U * 256U)
                + ( (uint32_t) coefs[DPS310_COEF_C00_OFFSET + 1U] * 256U)
                + ( (uint32_t) coefs[DPS310_COEF_C00_OFFSET + 2U]))
              >> DPS310_COEF_C00_SHIFT;
    ctx->c00 = twos_complement (reg_val, DPS310_COEF_C00_LEN_BITS);
    reg_val = ( ( (uint32_t) coefs[DPS310_COEF_C10_OFFSET] * 256U * 256U)
                + ( (uint32_t) coefs[DPS310_COEF_C10_OFFSET + 1U] * 256U)
                + ( (uint32_t) coefs[DPS310_COEF_C10_OFFSET + 2U]))
              & ( (1U << DPS310_COEF_C10_LEN_BITS) - 1U);
    ctx->c10 = twos_complement (reg_val, DPS310_COEF_C10_LEN_BITS);
    reg_val = ( ( (uint32_t) coefs[DPS310_COEF_C01_OFFSET] * 256U)
                + ( (uint32_t) coefs[DPS310_COEF_C01_OFFSET + 1U]));
    ctx->c01 = twos_complement (reg_val, DPS310_COEF_C01_LEN_BITS);
    reg_val = ( ( (uint32_t) coefs[DPS310_COEF_C11_OFFSET] * 256U)
                + ( (uint32_t) coefs[DPS310_COEF_C11_OFFSET + 1U]));
    ctx->c11 = twos_complement (reg_val, DPS310_COEF_C01_LEN_BITS);
    reg_val = ( ( (uint32_t) coefs[DPS310_COEF_C20_OFFSET] * 256U)
                + ( (uint32_t) coefs[DPS310_COEF_C20_OFFSET + 1U]));
    ctx->c20 = twos_complement (reg_val, DPS310_COEF_C20_LEN_BITS);
    reg_val = ( ( (uint32_t) coefs[DPS310_COEF_C21_OFFSET] * 256U)
                + ( (uint32_t) coefs[DPS310_COEF_C21_OFFSET + 1U]));
    ctx->c21 = twos_complement (reg_val, DPS310_COEF_C21_LEN_BITS);
    reg_val = ( ( (uint32_t) coefs[DPS310_COEF_C30_OFFSET] * 256U)
                + ( (uint32_t) coefs[DPS310_COEF_C30_OFFSET + 1U]));
    ctx->c30 = twos_complement (reg_val, DPS310_COEF_C30_LEN_BITS);
    return reg_read_ret;
}

static int32_t soft_reset (dps310_ctx_t * const ctx)
{
    uint8_t cmd = DPS310_SOFT_RST_VAL;
    return ctx->write (ctx->comm_ctx, DPS310_RST_CFG_REG, &cmd, 1U);
}

static int32_t read_revision (dps310_ctx_t * const ctx)
{
    uint8_t revision[1U] = {0xFFU};
    int32_t err_code = ctx->read (ctx->comm_ctx, DPS310_PROD_ID_REG, revision, 1U);

    if (DPS310_SUCCESS == err_code)
    {
        ctx->product_id = revision[0U]  & DPS310_PROD_ID_MASK;
        ctx->revision_id = (revision[0U] & DPS310_PROD_ID_REV_MASK)
                           >> DPS310_PROD_ID_REV_SHIFT;

        if ( (DPS310_PRODUCT_ID_VAL != ctx->product_id)
                || (DPS310_REVISION_ID_VAL != ctx->revision_id))
        {
            err_code = DPS310_UNKNOWN_REV;
        }
    }

    return err_code;
}

// Undocumented workaround for undocumented feature of DPS310, lifted from
// DPS310 Arduino driver.
// https://github.com/Infineon/DPS310-Pressure-Sensor/blob/888200c7efd8edb19ce69a2144e28ba31cdad449/src/DpsClass.cpp#L448
static const int32_t efuse_write (dps310_ctx_t * const ctx)
{
    int32_t ret_code = DPS310_SUCCESS;
    uint8_t regs[5U] =
    {
        DPS310_EFUSE_0_REG,
        DPS310_EFUSE_1_REG,
        DPS310_EFUSE_2_REG,
        DPS310_EFUSE_0_REG,
        DPS310_EFUSE_1_REG
    };
    uint8_t cmds[5U] =
    {
        DPS310_EFUSE_0_VAL,
        DPS310_EFUSE_1_VAL,
        DPS310_EFUSE_2_VAL,
        0U,
        0U
    };
    uint8_t cmd_idx = 0U;

    while ( (cmd_idx < sizeof (cmds)) && (DPS310_SUCCESS == ret_code))
    {
        ret_code = ctx->write (ctx->comm_ctx, regs[cmd_idx], &cmds[cmd_idx], 1U);
        cmd_idx++;
    }

    return ret_code;
}

int32_t dps310_init (dps310_ctx_t * const ctx)
{
    int32_t err_code = DPS310_SUCCESS;
    ctx->sleep (DPS310_POR_DELAY_MS);
    err_code = soft_reset (ctx);

    if (DPS310_SUCCESS == err_code)
    {
        ctx->sleep (DPS310_COEF_DELAY_MS);
        err_code = read_revision (ctx);
    }

    if (DPS310_SUCCESS == err_code)
    {
        err_code = efuse_write (ctx);
    }

    if (DPS310_SUCCESS == err_code)
    {
        err_code = read_coefs (ctx);
    }

    if (DPS310_SUCCESS == err_code)
    {
        err_code = dps310_config_temp (ctx, DPS310_DEFAULT_MR, DPS310_DEFAULT_OS);
    }

    if (DPS310_SUCCESS == err_code)
    {
        err_code = dps310_config_pres (ctx, DPS310_DEFAULT_MR, DPS310_DEFAULT_OS);
    }

    return err_code;
}

static int32_t set_mr_reg (uint8_t * const reg, const dps310_mr_t mr)
{
    int32_t err_code = DPS310_SUCCESS;

    switch (mr)
    {
    case DPS310_MR_1:
        *reg = 0U << DPS310_MR_SHIFT;
        break;

    case DPS310_MR_2:
        *reg = 1U << DPS310_MR_SHIFT;
        break;

    case DPS310_MR_4:
        *reg = 2U << DPS310_MR_SHIFT;
        break;

    case DPS310_MR_8:
        *reg = 3U << DPS310_MR_SHIFT;
        break;

    case DPS310_MR_16:
        *reg = 4U << DPS310_MR_SHIFT;
        break;

    case DPS310_MR_32:
        *reg = 5U << DPS310_MR_SHIFT;
        break;

    case DPS310_MR_64:
        *reg = 6U << DPS310_MR_SHIFT;
        break;

    case DPS310_MR_128:
        *reg = 7U << DPS310_MR_SHIFT;
        break;

    default:
        err_code = DPS310_INVALID_PARAM;
    }

    return err_code;
}

static int32_t set_os_reg (uint8_t * const reg, const dps310_os_t os)
{
    int32_t err_code = DPS310_SUCCESS;

    switch (os)
    {
    case DPS310_OS_1:
        *reg = 0U;
        break;

    case DPS310_OS_2:
        *reg = 1U;
        break;

    case DPS310_OS_4:
        *reg = 2U;
        break;

    case DPS310_OS_8:
        *reg = 3U;
        break;

    case DPS310_OS_16:
        *reg = 4U;
        break;

    case DPS310_OS_32:
        *reg = 5U;
        break;

    case DPS310_OS_64:
        *reg = 6U;
        break;

    case DPS310_OS_128:
        *reg = 7U;
        break;

    default:
        err_code = DPS310_INVALID_PARAM;
    }

    return err_code;
}

int32_t dps310_config_temp (dps310_ctx_t * const ctx, const dps310_mr_t temp_mr,
                            const dps310_os_t temp_osr)
{
    int32_t err_code = DPS310_SUCCESS;
    uint8_t cmd = 0U;
    err_code = set_mr_reg (&cmd, temp_mr);

    if (DPS310_SUCCESS == err_code)
    {
        err_code = set_os_reg (&cmd, temp_osr);
    }

    // TODO: SET coef_src
    if (DPS310_SUCCESS == err_code)
    {
        err_code = ctx->write (ctx->comm_ctx, DPS310_TEMP_CFG_REG, &cmd, 1U);
    }

    if (DPS310_SUCCESS == err_code)
    {
        ctx->temp_mr = temp_mr;
        ctx->temp_osr = temp_osr;
    }

    return err_code;
}

int32_t dps310_config_pres (dps310_ctx_t * const ctx, const dps310_mr_t pres_mr,
                            const dps310_os_t pres_osr)
{
    int32_t err_code = DPS310_SUCCESS;
    uint8_t cmd = 0U;
    err_code = set_mr_reg (&cmd, pres_mr);

    if (DPS310_SUCCESS == err_code)
    {
        err_code = set_os_reg (&cmd, pres_osr);
    }

    if (DPS310_SUCCESS == err_code)
    {
        err_code = ctx->write (ctx->comm_ctx, DPS310_PRES_CFG_REG, &cmd, 1U);
    }

    if (DPS310_SUCCESS == err_code)
    {
        ctx->pres_mr = pres_mr;
        ctx->pres_osr = pres_osr;
    }

    return err_code;
}

/*@}*/
