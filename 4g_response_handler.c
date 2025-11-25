/*
 * 4G模组响应处理示例
 * 1. 解析MCU发送给4G模组命令的响应
 * 2. 处理4G模组主动上报的URC事件
 */

#include "src/cat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RETENTION_DATA
#define log_i printf
#define log_e printf

/**
 * @brief Check SIM status.
 * @param var The variable pointer.
 * @param write_size The write size.
 * @return 0 on success, otherwise error code.
 */
static int sim_status_check_handle(const struct cat_variable* var, const size_t write_size)
{
    if (strnstr(var->data, "READY", write_size) != NULL)
    {
        log_i("SIM card is ready\r\n");
    }
    else
    {
        log_i("SIM card is not ready\r\n");
    }
    return 0;
}

/**
 * @brief Check IMEI.
 * @param var The variable pointer.
 * @param write_size The write size.
 * @return 0 on success, otherwise error code.
 */
static int imei_check_handle(const struct cat_variable* var, const size_t write_size)
{
    ((char*) var->data)[write_size] = '\0';
    return 0;
}

/**
 * @brief Check ICCID.
 * @param var The variable pointer.
 * @param write_size The write size.
 * @return 0 on success, otherwise error code.
 */
static int iccid_check_handle(const struct cat_variable* var, const size_t write_size)
{
    ((char*) var->data)[write_size] = '\0';
    return 0;
}

/**
 * @brief Check PDP context ready.
 * @param var The variable pointer.
 * @param write_size The write size.
 * @return 0 on success, otherwise error code.
 */
static int pdp_ready_check_handle(const struct cat_variable* var, const size_t write_size)
{
    if (((uint8_t*) var->data)[0] == 1)
    {
        
    }
    else
    {
        
    }
    return 0;
}

/**
 * @brief Check socket status.
 * @param cmd The command pointer.
 * @param data The data pointer.
 * @param data_size The data size.
 * @param args_num The arguments number.
 * @return 0 on success, otherwise error code.
 */
static int socket_status_check_handle(const struct cat_command* cmd, const uint8_t* data, const size_t data_size, const size_t args_num)
{
    uint8_t link_id    = ((uint8_t*) cmd->var[0].data)[0];
    uint8_t link_state = ((uint8_t*) cmd->var[5].data)[0];
    log_i("Link %u state: %u\r\n", link_id, link_state);
    if (link_state == 2)
    {
        
    }
    else
    {
        
    }
    return CAT_RETURN_STATE_OK;
}

/**
 * @brief Check send data result.
 * @param var The variable pointer.
 * @param write_size The write size.
 * @return 0 on success, otherwise error code.
 */
static int send_data_result_check_handle(const struct cat_variable* var, const size_t write_size)
{
    uint32_t total_send_bytes = ((uint32_t*) var->data)[0];
    
    return 0;
}

/**
 * @brief Handle URC.
 * @param cmd The command pointer.
 * @param data The data pointer.
 * @param data_size The data size.
 * @param args_num The arguments number.
 * @return 0 on success, otherwise error code.
 */
static int urc_handle(const struct cat_command* cmd, const uint8_t* data, const size_t data_size, const size_t args_num)
{
    char*  cmd_str = (char*) cmd->var[0].data;
    int8_t param   = ((int8_t*) cmd->var[1].data)[0];
    if (strstr(cmd_str, "recv")) //!< URC: +LIURC: recv
    {
        log_i("URC: Receive data from link %d\r\n", param);
        
    }
    else if (strstr(cmd_str, "closed")) //!< URC: +LIURC: closed
    {
        log_i("URC: Link %d closed\r\n", param);
    }
    else if (strstr(cmd_str, "pdpdeact")) //!< URC: +LIURC: pdpdeact
    {
        log_i("URC: PDP context %d deactivated\r\n", param);
    }
    return CAT_RETURN_STATE_OK;
}

/**
 * @brief Read data handle.
 * @param var The variable pointer.
 * @param write_size The write size.
 * @return 0 on success, otherwise error code.
 */
static int read_data_handle(const struct cat_variable* var, const size_t write_size)
{
    uint16_t read_data_len = ((uint16_t*) var->data)[0];
    log_i("Read data from link %u length: %u\r\n", 0, read_data_len);
    return 0;
}

/**
 * @brief Network status check handle.
 * @param cmd The command pointer.
 * @param data The data pointer.
 * @param data_size The data size.
 * @param args_num The arguments number.
 * @return 0 on success, otherwise error code.
 */
static int network_status_check_handle(const struct cat_command* cmd, const uint8_t* data, const size_t data_size, const size_t args_num)
{
    char* act_type = ((char*) cmd->var[0].data);
    if (strstr(act_type, "LTE"))
    {
        log_i("Cur cell network is LTE\r\n");
        char* end  = NULL;
        long  rssi = strtol(((char*) cmd->var[10].data), &end, 10); // NOLINT
        if (*end != '\0')
        {
            log_e("RSSI is not a number\r\n");
        }
        else
        {
            log_i("Cur RSSI: %d\r\n", rssi);
        }
        end      = NULL;
        long snr = strtol(((char*) cmd->var[13].data), &end, 10); // NOLINT
        if (*end != '\0')
        {
            log_e("SNR is not a number\r\n");
        }
        else
        {
            log_i("Cur SNR: %d\r\n", snr);
        }
    }
    else if (strstr(act_type, "NR"))
    {
        log_i("Cur cell network is NR\r\n");
        char* end  = NULL;
        long  rssi = strtol(((char*) cmd->var[10].data), &end, 10); // NOLINT
        if (*end != '\0')
        {
            log_e("RSSI is not a number\r\n");
        }
        else
        {
            log_i("Cur RSSI: %d\r\n", rssi);
        }
        end      = NULL;
        long snr = strtol(((char*) cmd->var[12].data), &end, 10); // NOLINT
        if (*end != '\0')
        {
            log_e("SNR is not a number\r\n");
        }
        else
        {
            log_i("Cur SNR: %d\r\n", snr);
        }
    }
    else if (strstr(act_type, "WCDMA"))
    {
        log_i("Cur cell network is WCDMA\r\n");
    }
    else
    {
        log_i("Cur cell network is unknown\r\n");
    }
    return CAT_RETURN_STATE_OK;
}

/// AT Cmd variables definition
#define AT_VAR_BUF_SIZE 64U
static char g_at_var_buf[AT_VAR_BUF_SIZE] = {0};
// SIM card status variable definition
static struct cat_variable g_sim_ready_var[] = {{
    .name      = "SIM_STATUS",
    .type      = CAT_VAR_BUF_STRING,
    .data      = g_at_var_buf,
    .data_size = AT_VAR_BUF_SIZE,
    .access    = CAT_VAR_ACCESS_READ_WRITE,
    .write     = sim_status_check_handle,
}};

// IMEI variable definition
static struct cat_variable g_imei_var[] = {{
    .name      = "IMEI",
    .type      = CAT_VAR_BUF_STRING,
    .data      = g_at_var_buf,
    .data_size = AT_VAR_BUF_SIZE,
    .access    = CAT_VAR_ACCESS_READ_WRITE,
    .write     = imei_check_handle,
}};

// ICCID variable definition
static struct cat_variable g_iccid_var[] = {{
    .name      = "ICCID",
    .type      = CAT_VAR_BUF_STRING,
    .data      = g_at_var_buf,
    .data_size = AT_VAR_BUF_SIZE,
    .access    = CAT_VAR_ACCESS_READ_WRITE,
    .write     = iccid_check_handle,
}};

// PDP context ready variable definition
// clang-format off
static struct cat_variable g_pdp_ready_var[] = {{
    .name      = "Context ID",
    .type      = CAT_VAR_UINT_DEC,
    .data      = &g_at_var_buf[0],
    .data_size = 1,
    .access    = CAT_VAR_ACCESS_READ_WRITE,
    .write     = NULL,
},
{
    .name      = "Context Status",
    .type      = CAT_VAR_UINT_DEC,
    .data      = &g_at_var_buf[1],
    .data_size = 1,
    .access    = CAT_VAR_ACCESS_READ_WRITE,
    .write     = pdp_ready_check_handle,
}};
// clang-format on

// socket status variable definition
#define SERVICE_TYPE_LEN (4U)
#define IP_ADDRESS_LEN (16U)
#define PORT_LEN (6U)
static struct cat_variable g_socket_status_var[] = {{
                                                        .name      = "Connect ID",
                                                        .type      = CAT_VAR_UINT_DEC,
                                                        .data      = &g_at_var_buf[0],
                                                        .data_size = 1,
                                                        .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                        .write     = NULL,
                                                    },
                                                    {
                                                        .name      = "Service Type",
                                                        .type      = CAT_VAR_BUF_STRING,
                                                        .data      = &g_at_var_buf[1],
                                                        .data_size = SERVICE_TYPE_LEN,
                                                        .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                        .write     = NULL,
                                                    },
                                                    {
                                                        .name      = "IP Address",
                                                        .type      = CAT_VAR_BUF_STRING,
                                                        .data      = &g_at_var_buf[SERVICE_TYPE_LEN + 1],
                                                        .data_size = IP_ADDRESS_LEN,
                                                        .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                        .write     = NULL,
                                                    },
                                                    {
                                                        .name      = "Remote Port",
                                                        .type      = CAT_VAR_UINT_DEC,
                                                        .data      = &g_at_var_buf[SERVICE_TYPE_LEN + 1 + IP_ADDRESS_LEN],
                                                        .data_size = 2,
                                                        .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                        .write     = NULL,
                                                    },
                                                    {
                                                        .name      = "Local Port",
                                                        .type      = CAT_VAR_UINT_DEC,
                                                        .data      = &g_at_var_buf[SERVICE_TYPE_LEN + 1 + IP_ADDRESS_LEN + 2],
                                                        .data_size = 2,
                                                        .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                        .write     = NULL,
                                                    },
                                                    {
                                                        .name      = "Socket State",
                                                        .type      = CAT_VAR_UINT_DEC,
                                                        .data      = &g_at_var_buf[SERVICE_TYPE_LEN + 1 + IP_ADDRESS_LEN + 2 + 2],
                                                        .data_size = 1,
                                                        .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                        .write     = NULL,
                                                    },
                                                    {
                                                        .name      = "Context ID",
                                                        .type      = CAT_VAR_UINT_DEC,
                                                        .data      = &g_at_var_buf[SERVICE_TYPE_LEN + 1 + IP_ADDRESS_LEN + 2 + 2 + 1],
                                                        .data_size = 1,
                                                        .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                        .write     = NULL,
                                                    },
                                                    {
                                                        .name      = "Server ID",
                                                        .type      = CAT_VAR_INT_DEC,
                                                        .data      = &g_at_var_buf[SERVICE_TYPE_LEN + 1 + IP_ADDRESS_LEN + 2 + 2 + 1 + 1],
                                                        .data_size = 1,
                                                        .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                        .write     = NULL,
                                                    },
                                                    {
                                                        .name      = "Access Mode",
                                                        .type      = CAT_VAR_UINT_DEC,
                                                        .data      = &g_at_var_buf[SERVICE_TYPE_LEN + 1 + IP_ADDRESS_LEN + 2 + 2 + 1 + 1 + 1],
                                                        .data_size = 1,
                                                        .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                        .write     = NULL,
                                                    },
                                                    {
                                                        .name      = "AT Port",
                                                        .type      = CAT_VAR_BUF_STRING,
                                                        .data      = &g_at_var_buf[SERVICE_TYPE_LEN + 1 + IP_ADDRESS_LEN + 2 + 2 + 1 + 1 + 1 + 1],
                                                        .data_size = PORT_LEN,
                                                        .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                        .write     = NULL,
                                                    }};
/// Send data result variable definition
static struct cat_variable g_send_data_result_var[] = {{
    .name      = "Total Send Bytes",
    .type      = CAT_VAR_UINT_DEC,
    .data      = &g_at_var_buf[0],
    .data_size = 4,
    .access    = CAT_VAR_ACCESS_READ_WRITE,
    .write     = send_data_result_check_handle,
}};

/// URC Cmd variables definition
#define URC_CMD_LEN (20U)
static struct cat_variable g_urc_var[] = {{
                                              .name      = "CMD",
                                              .type      = CAT_VAR_BUF_STRING,
                                              .data      = g_at_var_buf,
                                              .data_size = URC_CMD_LEN,
                                              .access    = CAT_VAR_ACCESS_READ_WRITE,
                                              .write     = NULL,
                                          },
                                          {
                                              .name      = "Parameter",
                                              .type      = CAT_VAR_INT_DEC,
                                              .data      = &g_at_var_buf[URC_CMD_LEN],
                                              .data_size = 1,
                                              .access    = CAT_VAR_ACCESS_READ_WRITE,
                                              .write     = NULL,
                                          }};

/// read data variable definition
static struct cat_variable g_read_data_var[] = {{
    .name      = "Read Data",
    .type      = CAT_VAR_UINT_DEC,
    .data      = g_at_var_buf,
    .data_size = 2,
    .access    = CAT_VAR_ACCESS_READ_WRITE,
    .write     = read_data_handle,
}};

/// Network status variable definition
static struct cat_variable g_network_status_var[] = {{
                                                         .name      = "act_type",
                                                         .type      = CAT_VAR_BUF_STRING,
                                                         .data      = g_at_var_buf,
                                                         .data_size = 10,
                                                         .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                         .write     = NULL,
                                                     },
                                                     {
                                                         .name      = "PCID",
                                                         .type      = CAT_VAR_BUF_STRING,
                                                         .data      = &g_at_var_buf[10],
                                                         .data_size = 10,
                                                         .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                         .write     = NULL,
                                                     },
                                                     {
                                                         .name      = "state",
                                                         .type      = CAT_VAR_BUF_STRING,
                                                         .data      = &g_at_var_buf[20],
                                                         .data_size = 10,
                                                         .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                         .write     = NULL,
                                                     },
                                                     {
                                                         .name      = "MCC",
                                                         .type      = CAT_VAR_BUF_STRING,
                                                         .data      = &g_at_var_buf[30],
                                                         .data_size = 5,
                                                         .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                         .write     = NULL,
                                                     },
                                                     {
                                                         .name      = "MNC",
                                                         .type      = CAT_VAR_BUF_STRING,
                                                         .data      = &g_at_var_buf[35],
                                                         .data_size = 5,
                                                         .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                         .write     = NULL,
                                                     },
                                                     {
                                                         .name      = "TAC",
                                                         .type      = CAT_VAR_BUF_STRING,
                                                         .data      = &g_at_var_buf[40],
                                                         .data_size = 5,
                                                         .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                         .write     = NULL,
                                                     },
                                                     {
                                                         .name      = "arfcn",
                                                         .type      = CAT_VAR_BUF_STRING,
                                                         .data      = &g_at_var_buf[45],
                                                         .data_size = 10,
                                                         .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                         .write     = NULL,
                                                     },
                                                     {
                                                         .name      = "band",
                                                         .type      = CAT_VAR_BUF_STRING,
                                                         .data      = &g_at_var_buf[55],
                                                         .data_size = 5,
                                                         .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                         .write     = NULL,
                                                     },
                                                     {
                                                         .name      = "UL_bandwidth",
                                                         .type      = CAT_VAR_BUF_STRING,
                                                         .data      = &g_at_var_buf[60],
                                                         .data_size = 5,
                                                         .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                         .write     = NULL,
                                                     },
                                                     {
                                                         .name      = "DL_bandwidth",
                                                         .type      = CAT_VAR_BUF_STRING,
                                                         .data      = &g_at_var_buf[65],
                                                         .data_size = 5,
                                                         .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                         .write     = NULL,
                                                     },
                                                     {
                                                         .name      = "RSRP",
                                                         .type      = CAT_VAR_BUF_STRING,
                                                         .data      = &g_at_var_buf[70],
                                                         .data_size = 5,
                                                         .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                         .write     = NULL,
                                                     },
                                                     {
                                                         .name      = "RSRQ",
                                                         .type      = CAT_VAR_BUF_STRING,
                                                         .data      = &g_at_var_buf[75],
                                                         .data_size = 5,
                                                         .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                         .write     = NULL,
                                                     },
                                                     {
                                                         .name      = "RSSI",
                                                         .type      = CAT_VAR_BUF_STRING,
                                                         .data      = &g_at_var_buf[80],
                                                         .data_size = 5,
                                                         .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                         .write     = NULL,
                                                     },
                                                     {
                                                         .name      = "SINR",
                                                         .type      = CAT_VAR_BUF_STRING,
                                                         .data      = &g_at_var_buf[85],
                                                         .data_size = 5,
                                                         .access    = CAT_VAR_ACCESS_READ_WRITE,
                                                         .write     = NULL,
                                                     }};

/// AT Cmd commands definition
static struct cat_command g_at_cmds[] = {
    {
        .name           = "CPIN",
        .description    = "SIM card check",
        .var            = g_sim_ready_var,
        .var_num        = sizeof(g_sim_ready_var) / sizeof(g_sim_ready_var[0]),
        .need_all_vars  = true,
        .only_test      = false,
        .disable        = false,
        .implicit_write = false,
    },
    {
        .name           = "LGSN",
        .description    = "IMEI check",
        .var            = g_imei_var,
        .var_num        = sizeof(g_imei_var) / sizeof(g_imei_var[0]),
        .need_all_vars  = true,
        .only_test      = false,
        .disable        = false,
        .implicit_write = false,
    },
    {
        .name           = "CCID",
        .description    = "ICCID check",
        .var            = g_iccid_var,
        .var_num        = sizeof(g_iccid_var) / sizeof(g_iccid_var[0]),
        .need_all_vars  = true,
        .only_test      = false,
        .disable        = false,
        .implicit_write = false,
    },
    {
        .name           = "LIACT",
        .description    = "PDP context ready check",
        .var            = g_pdp_ready_var,
        .var_num        = sizeof(g_pdp_ready_var) / sizeof(g_pdp_ready_var[0]),
        .need_all_vars  = true,
        .only_test      = false,
        .disable        = false,
        .implicit_write = false,
    },
    {
        .name           = "LISTATE",
        .description    = "Check socket state",
        .var            = g_socket_status_var,
        .var_num        = sizeof(g_socket_status_var) / sizeof(g_socket_status_var[0]),
        .need_all_vars  = true,
        .only_test      = false,
        .disable        = false,
        .implicit_write = false,
        .write          = socket_status_check_handle,
    },
    {
        .name           = "LISEND",
        .description    = "Send data",
        .var            = g_send_data_result_var,
        .var_num        = sizeof(g_send_data_result_var) / sizeof(g_send_data_result_var[0]),
        .need_all_vars  = true,
        .only_test      = false,
        .disable        = false,
        .implicit_write = false,
    },
    {
        .name           = "LIURC",
        .description    = "Check URC",
        .var            = g_urc_var,
        .var_num        = sizeof(g_urc_var) / sizeof(g_urc_var[0]),
        .need_all_vars  = true,
        .only_test      = false,
        .disable        = false,
        .implicit_write = false,
        .write          = urc_handle,
    },
    {
        .name           = "LIRD",
        .description    = "Read data",
        .var            = g_read_data_var,
        .var_num        = sizeof(g_read_data_var) / sizeof(g_read_data_var[0]),
        .need_all_vars  = true,
        .only_test      = false,
        .disable        = false,
        .implicit_write = false,
    },
    {
        .name           = "LSERVCELINFO",
        .description    = "Get cell network information",
        .var            = g_network_status_var,
        .var_num        = sizeof(g_network_status_var) / sizeof(g_network_status_var[0]),
        .need_all_vars  = false,
        .only_test      = false,
        .disable        = false,
        .implicit_write = false,
        .write          = network_status_check_handle,
    },
};

/// AT Cmd group definition
static struct cat_command_group g_at_cmd_group = {
    .name    = "Lierda 5G Module AT Commands",
    .cmd     = g_at_cmds,
    .cmd_num = sizeof(g_at_cmds) / sizeof(g_at_cmds[0]),
    .disable = false,
};

static struct cat_command_group* g_at_cmd_groups[] = {
    &g_at_cmd_group,
};

/// AT Cmd buffer definition
#define AT_CMD_BUF1_SIZE (1024U)
#define AT_CMD_BUF2_SIZE (64U)
RETENTION_DATA static uint8_t g_at_cmd_buf1[AT_CMD_BUF1_SIZE] = {0};
RETENTION_DATA static uint8_t g_at_cmd_buf2[AT_CMD_BUF2_SIZE] = {0};

/// AT Cmd descriptor definition
RETENTION_DATA static struct cat_descriptor g_at_cmd_desc = {
    .cmd_group            = g_at_cmd_groups,
    .cmd_group_num        = sizeof(g_at_cmd_groups) / sizeof(g_at_cmd_groups[0]),
    .buf                  = g_at_cmd_buf1,
    .buf_size             = AT_CMD_BUF1_SIZE,
    .unsolicited_buf      = g_at_cmd_buf2,
    .unsolicited_buf_size = AT_CMD_BUF2_SIZE,
};

// ================================
// 7. IO接口（从4G模组读取响应）
// ================================

static char io_buffer[1024];
static size_t io_buffer_len = 0;
static size_t io_buffer_pos = 0;

// 模拟4G模组发送响应
static void simulate_4g_response(const char *response)
{
    // 实际应用中，这里是从串口读取4G模组的响应
    strcpy(io_buffer, response);
    io_buffer_len = strlen(response);
    io_buffer_pos = 0;
}

static void simulate_4g_response_raw(uint16_t len, uint8_t *data)
{
    memcpy(io_buffer, data, len);
    io_buffer_len = len;
    io_buffer_pos = 0;
}

// 从4G模组读取字符
static int read_from_4g_module(char *ch)
{
    if (io_buffer_pos >= io_buffer_len) {
        return 0;  // 无数据可读
    }
    
    *ch = io_buffer[io_buffer_pos++];
    return 1;
}

// 向4G模组发送命令
static int write_to_4g_module(char ch)
{
    // 实际应用中发送到串口
    putchar(ch);
    return 1;
}

static struct cat_io_interface io = {
    .read = read_from_4g_module,
    .write = write_to_4g_module
};

// ================================
// 8. 主程序
// ================================

int main(void)
{
    struct cat_object at_obj;
    
    cat_init(&at_obj, &g_at_cmd_desc, &io, NULL);
    
    printf("=== 4G Module Response Handler ===\n\n");

    // char test_data[]={0x41, 0x54, 2B 43 50 49 4E 3F 0D 0D 0A 2B 43 50 49 4E 3A 20 52 45 41 44 59 0D 0A 0D 0A 4F 4B 0D 0A };
    // uint8_t test_data[] = {
    //     0x41, 0x54, 0x2B, 0x43, 0x50, 0x49, 0x4E, 0x3F, 0x0D, 0x0D, 0x0A,
    //     0x2B, 0x43, 0x50, 0x49, 0x4E, 0x3A, 0x20, 0x52, 0x45, 0x41, 0x44,
    //     0x59, 0x0D, 0x0A, 0x0D, 0x0A, 0x4F, 0x4B, 0x0D, 0x0A
    // };
    // char test_data[]= "+LISTATE: 0,\"TCP\",\"112.125.89.8\",33859,0,2,1,-1,0,\"uart\"\r\nOK\r\n";
    char test_data[] = "+LSERVCELINFO: \"LTE\",444,\"CONNECT\",460,00,5088,38950,40,5,5,-76,-6,-49,34\r\n\r\nOK\r\n";
    // simulate_4g_response_raw(sizeof(test_data), test_data);
    simulate_4g_response(test_data);
    while (cat_service(&at_obj) != CAT_STATUS_OK) {
        ; // 处理完成
    }
    return 0;
}