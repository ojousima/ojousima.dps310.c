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
#include <stddef.h>

// Valid up to 31 bits
static int32_t twos_complement (const uint32_t value, const uint8_t bits)
{
    const int32_t max_signed_value_exclusive = (int32_t) (1U << (bits - 1U));
    int32_t complement = (int32_t) value;

    if (complement >= max_signed_value_exclusive)
    {
        complement -= 2 * max_signed_value_exclusive;
    }

    return complement;
}

static dps310_status_t read_coefs (dps310_ctx_t * const ctx)
{
    uint8_t coefs[DPS310_COEF_REG_LEN] = {0};
    dps310_status_t reg_read_ret = ctx->read (ctx->comm_ctx, DPS310_COEF_START_REG,
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

static dps310_status_t soft_reset (dps310_ctx_t * const ctx)
{
    dps310_status_t status = DPS310_SUCCESS;
    uint8_t cmd = DPS310_SOFT_RST_VAL;
    status |= ctx->write (ctx->comm_ctx, DPS310_RST_CFG_REG, &cmd, 1U);

    if (DPS310_SUCCESS == status)
    {
        ctx->device_status = DPS310_READY;
    }
    else
    {
        status |= DPS310_BUS_ERROR;
        ctx->device_status = DPS310_BUS_ERROR;
    }

    return status;
}

static dps310_status_t read_revision (dps310_ctx_t * const ctx)
{
    dps310_status_t status = DPS310_SUCCESS;
    uint8_t revision[1U] = {0xFFU};

    if (DPS310_READY == ctx->device_status)
    {
        status |= ctx->read (ctx->comm_ctx, DPS310_PROD_ID_REG, revision, 1U);

        if (DPS310_SUCCESS == status)
        {
            ctx->product_id = revision[0U]  & DPS310_PROD_ID_MASK;
            ctx->revision_id = (revision[0U] & DPS310_PROD_ID_REV_MASK)
                               >> DPS310_PROD_ID_REV_SHIFT;

            if ( (DPS310_PRODUCT_ID_VAL != ctx->product_id)
                    || (DPS310_REVISION_ID_VAL != ctx->revision_id))
            {
                status |= DPS310_UNKNOWN_REV;
                ctx->device_status = DPS310_INVALID_STATE;
            }
        }
        else
        {
            status |= DPS310_BUS_ERROR;
            ctx->device_status = DPS310_BUS_ERROR;
        }
    }

    return status;
}

// Undocumented workaround for undocumented feature of DPS310, lifted from
// DPS310 Arduino driver.
// https://github.com/Infineon/DPS310-Pressure-Sensor/blob/888200c7efd8edb19ce69a2144e28ba31cdad449/src/DpsClass.cpp#L448
static const dps310_status_t efuse_write (dps310_ctx_t * const ctx)
{
    dps310_status_t ret_code = DPS310_SUCCESS;

    if (DPS310_READY == ctx->device_status)
    {
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

        if (DPS310_SUCCESS != ret_code)
        {
            ret_code |= DPS310_BUS_ERROR;
            ctx->device_status = DPS310_BUS_ERROR;
        }
    }

    return ret_code;
}

dps310_status_t dps310_init (dps310_ctx_t * const ctx)
{
    dps310_status_t err_code = DPS310_SUCCESS;

    if ( (NULL == ctx)
            || (NULL == ctx->sleep)
            || (NULL == ctx->write)
            || (NULL == ctx->read))
    {
        err_code = DPS310_ERROR_NULL;
    }
    else
    {
        ctx->sleep (DPS310_POR_DELAY_MS);
        err_code |= soft_reset (ctx);
        ctx->sleep (DPS310_COEF_DELAY_MS);
        err_code |= read_revision (ctx);
        err_code |= efuse_write (ctx);
        err_code |= read_coefs (ctx);
        err_code |= dps310_config_temp (ctx, DPS310_DEFAULT_MR, DPS310_DEFAULT_OS);
        err_code |= dps310_config_pres (ctx, DPS310_DEFAULT_MR, DPS310_DEFAULT_OS);
    }

    return err_code;
}

static dps310_status_t set_mr_reg (uint8_t * const reg, const dps310_mr_t mr)
{
    dps310_status_t err_code = DPS310_SUCCESS;

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
        break;
    }

    return err_code;
}

static dps310_status_t set_os_reg (uint8_t * const reg, const dps310_os_t os)
{
    dps310_status_t err_code = DPS310_SUCCESS;

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
        break;
    }

    return err_code;
}

dps310_status_t dps310_config_temp (dps310_ctx_t * const ctx, const dps310_mr_t temp_mr,
                                    const dps310_os_t temp_osr)
{
    dps310_status_t err_code = DPS310_SUCCESS;
    uint8_t cmd = 0U;

    if (DPS310_READY == ctx->device_status)
    {
        err_code |= set_mr_reg (&cmd, temp_mr);
        err_code |= set_os_reg (&cmd, temp_osr);

        // TODO: SET coef_src
        if (DPS310_SUCCESS == err_code)
        {
            err_code |= ctx->write (ctx->comm_ctx, DPS310_TEMP_CFG_REG, &cmd, 1U);

            if (DPS310_SUCCESS == err_code)
            {
                ctx->temp_mr = temp_mr;
                ctx->temp_osr = temp_osr;
            }
            else
            {
                err_code |= DPS310_BUS_ERROR;
                ctx->device_status = DPS310_INVALID_STATE;
            }
        }
    }

    return err_code;
}

dps310_status_t dps310_config_pres (dps310_ctx_t * const ctx, const dps310_mr_t pres_mr,
                                    const dps310_os_t pres_osr)
{
    dps310_status_t err_code = DPS310_SUCCESS;
    uint8_t cmd = 0U;

    if (DPS310_READY == ctx->device_status)
    {
        err_code |= set_mr_reg (&cmd, pres_mr);
        err_code |= set_os_reg (&cmd, pres_osr);

        if (DPS310_SUCCESS == err_code)
        {
            err_code |= ctx->write (ctx->comm_ctx, DPS310_PRES_CFG_REG, &cmd, 1U);

            if (DPS310_SUCCESS == err_code)
            {
                ctx->pres_mr = pres_mr;
                ctx->pres_osr = pres_osr;
            }
            else
            {
                err_code |= DPS310_BUS_ERROR;
                ctx->device_status = DPS310_BUS_ERROR;
            }
        }
    }

    return err_code;
}

/** @} */
