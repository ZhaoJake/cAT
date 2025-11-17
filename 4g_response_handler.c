/*
 * 4G模组响应处理示例
 * 1. 解析MCU发送给4G模组命令的响应
 * 2. 处理4G模组主动上报的URC事件
 */

#include "src/cat.h"
#include <stdio.h>
#include <string.h>

// ================================
// 1. 定义数据状态
// ================================

// 网络状态
typedef struct {
    uint8_t attach_status;      // 附着状态 (0/1)
    uint8_t signal_strength;    // 信号强度 (0-31)
    char ip_address[16];        // IP地址
} network_status_t;

static network_status_t net_status = {0};

// SIM卡信息
typedef struct {
    char iccid[32];             // SIM卡ICCID
    char imsi[32];              // IMSI
    uint8_t registration_status; // 注册状态
} sim_info_t;

static sim_info_t sim_info = {0};

// SMS相关
typedef struct {
    char number[32];            // 发送方号码
    char content[256];          // 消息内容
} sms_info_t;

static sms_info_t received_sms = {0};

static int net_status_handle(const struct cat_variable* var, const size_t write_size)
{
    uint8_t paramter = ((uint8_t*)var->data)[0];
    printf("\n[URC] Received paramter %u\n", paramter);
    return 0;
}

// ================================
// 2. 定义变量用于响应解析
// ================================

// 网络状态变量（用于接收4G模组响应）
static struct cat_variable net_status_vars[] = {
    {
        .name = "STATUS",
        .type = CAT_VAR_UINT_DEC,
        .data = &net_status.attach_status,
        .data_size = sizeof(net_status.attach_status),
        .access = CAT_VAR_ACCESS_READ_WRITE,
    },
    {
        .name = "RSSI",
        .type = CAT_VAR_UINT_DEC,
        .data = &net_status.signal_strength,
        .data_size = sizeof(net_status.signal_strength),
        .access = CAT_VAR_ACCESS_READ_WRITE,
    },
    {
        .name = "IP",
        .type = CAT_VAR_BUF_STRING,
        .data = net_status.ip_address,
        .data_size = sizeof(net_status.ip_address),
        .access = CAT_VAR_ACCESS_READ_WRITE,
    }
};

// SIM信息变量
static struct cat_variable sim_info_vars[] = {
    {
        .name = "ICCID",
        .type = CAT_VAR_BUF_STRING,
        .data = sim_info.iccid,
        .data_size = sizeof(sim_info.iccid),
        .access = CAT_VAR_ACCESS_READ_WRITE,
    },
    {
        .name = "IMSI",
        .type = CAT_VAR_BUF_STRING,
        .data = sim_info.imsi,
        .data_size = sizeof(sim_info.imsi),
        .access = CAT_VAR_ACCESS_READ_WRITE,
    }
};

// SMS变量（用于URC上报）
static struct cat_variable sms_urc_vars[] = {
    {
        .name = "NUMBER",
        .type = CAT_VAR_BUF_STRING,
        .data = received_sms.number,
        .data_size = sizeof(received_sms.number),
        .access = CAT_VAR_ACCESS_READ_WRITE,
    },
    {
        .name = "Parameters",
        .type = CAT_VAR_UINT_DEC,
        .data = received_sms.content,
        .data_size = 2,
        .access = CAT_VAR_ACCESS_READ_WRITE,
    }
};

static struct cat_variable urc_vars[] =
{
    {
        .name = "Cmd",
        .type = CAT_VAR_BUF_STRING,
        .data = received_sms.number,
        .data_size = sizeof(received_sms.number),
        .access = CAT_VAR_ACCESS_READ_WRITE,
    },
    {
        .name = "Parameters",
        .type = CAT_VAR_UINT_DEC,
        .data = received_sms.content,
        .data_size = 1,
        .access = CAT_VAR_ACCESS_READ_WRITE,
        .write = net_status_handle,
    }
};


// ================================
// 3. 命令响应处理回调
// ================================

// 处理网络附着响应
static cat_return_state cgatt_read_handler(const struct cat_command *cmd, 
                                            uint8_t *data, 
                                            size_t *data_size, 
                                            const size_t max_data_size)
{
    printf("\n[Response] Network attach status: %d\n", net_status.attach_status);
    printf("[Response] Signal strength: %d\n", net_status.signal_strength);
    
    // 根据状态触发后续操作
    if (net_status.attach_status == 1) {
        printf("[Status] Network attached successfully\n");
        // 触发获取IP地址
        // send_to_4g("AT+CGPADDR=1");
    } else {
        printf("[Status] Network not attached\n");
    }
    
    return CAT_RETURN_STATE_OK;
}

// 处理IP地址响应
static cat_return_state cgpaddr_read_handler(const struct cat_command *cmd, 
                                             uint8_t *data, 
                                             size_t *data_size, 
                                             const size_t max_data_size)
{
    printf("\n[Response] IP Address: %s\n", net_status.ip_address);
    
    if (strcmp(net_status.ip_address, "0.0.0.0") != 0) {
        printf("[Status] Got IP address successfully\n");
        // 可以开始TCP连接等操作
    }
    
    return CAT_RETURN_STATE_OK;
}

// 处理信号强度响应
static cat_return_state csq_read_handler(const struct cat_command *cmd, 
                                         uint8_t *data, 
                                         size_t *data_size, 
                                         const size_t max_data_size)
{
    printf("\n[Response] Signal strength: %d\n", net_status.signal_strength);
    
    if (net_status.signal_strength > 20) {
        printf("[Status] Good signal quality\n");
    } else if (net_status.signal_strength > 10) {
        printf("[Status] Acceptable signal quality\n");
    } else {
        printf("[Status] Poor signal quality\n");
    }
    
    return CAT_RETURN_STATE_OK;
}

// ================================
// 4. URC事件处理回调
// ================================

// 处理SMS接收URC
static cat_return_state sms_received_read_handler(const struct cat_command *cmd, 
                                                  uint8_t *data, 
                                                  size_t *data_size, 
                                                  const size_t max_data_size)
{
    printf("\n[URC] SMS received from: %s\n", received_sms.number);
    printf("[URC] Content: %s\n", received_sms.content);
    
    // 处理SMS内容
    // process_sms(&received_sms);
    
    return CAT_RETURN_STATE_OK;
}

// 处理网络附着URC
static cat_return_state cgatt_urc_handler(const struct cat_command *cmd, 
                                          uint8_t *data, 
                                          size_t *data_size, 
                                          const size_t max_data_size)
{
    printf("\n[URC] Network attach state changed: %d\n", net_status.attach_status);
    
    if (net_status.attach_status == 1) {
        printf("[URC] Network attached\n");
        // 网络附着后立即获取IP
        // send_to_4g("AT+CGPADDR=1");
    } else {
        printf("[URC] Network detached\n");
        // 清理网络相关状态
        strcpy(net_status.ip_address, "0.0.0.0");
    }
    
    return CAT_RETURN_STATE_OK;
}

// ================================
// 5. 定义AT命令（用于解析响应和URC）
// ================================

static struct cat_command cmds[] = {
    // 响应类命令（MCU查询4G模组状态）
    {
        .name = "+CGATT",
        .description = "Network attach status",
        .read = cgatt_read_handler,
        .var = &net_status_vars[0],  // 只解析STATUS
        .var_num = 1,
    },
    {
        .name = "+CGPADDR",
        .description = "Get IP address",
        .read = cgpaddr_read_handler,
        .var = &net_status_vars[2],  // 只解析IP
        .var_num = 1,
    },
    {
        .name = "+CSQ",
        .description = "Signal quality",
        .read = csq_read_handler,
        .var = &net_status_vars[1],  // 只解析RSSI
        .var_num = 1,
    },
    
    // URC类命令（4G模组主动上报）
    {
        .name = "+CMTI",
        .description = "New SMS indicator",
        .read = sms_received_read_handler,
        .var = sms_urc_vars,
        .var_num = 2,
    },
    {
        .name = "CGATT",
        .description = "Network attach URC",
        .read = cgatt_urc_handler,
        .var = &net_status_vars[0],
        .var_num = 1,
    },
    {
        .name = "LIURC",
        .description = "Generic URC command",
        .read = NULL,
        .var = urc_vars,
        .var_num = 2,
    }
};

// ================================
// 6. 命令组和描述符
// ================================

static struct cat_command_group cmd_group = {
    .name = "4g_responses",
    .cmd = cmds,
    .cmd_num = sizeof(cmds) / sizeof(cmds[0]),
};

static struct cat_command_group *groups[] = {
    &cmd_group
};

static char working_buf[256];

static struct cat_descriptor desc = {
    .cmd_group = groups,
    .cmd_group_num = 1,
    .buf = (uint8_t*)working_buf,
    .buf_size = sizeof(working_buf),
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
    
    // 初始化cAT框架
    cat_init(&at_obj, &desc, &io, NULL);
    
    printf("=== 4G Module Response Handler ===\n\n");
    
    // 场景1: 模拟MCU发送查询命令后的响应
    printf("--- 场景1: MCU查询网络状态 ---\n");
    printf("MCU发送: AT+CGATT?\n");

    simulate_4g_response("+LIURC: \"closed\",0\r\n");
    while (cat_service(&at_obj) != CAT_STATUS_OK) {
        ; // 处理完成
    }
    
    // 模拟4G模组响应: +CGATT: 1
    simulate_4g_response("+CGATT: 1\r\nOK\r\n");
    
    // 处理响应
    while (cat_service(&at_obj) != CAT_STATUS_OK) {
        ; // 处理完成
    }
    
    printf("\n--- 场景2: MCU查询信号强度 ---\n");
    printf("MCU发送: AT+CSQ?\n");
    
    // 模拟4G模组响应: +CSQ: 25,99
    simulate_4g_response("+CSQ: 25,99\r\nOK\r\n");
    
    while (cat_service(&at_obj) != CAT_STATUS_OK) {
        ; // 处理完成
    }
    
    // 场景2: 处理URC上报
    printf("\n--- 场景3: 4G模组主动上报SMS ---\n");
    printf("URC上报: +CMTI: \"SM\",5\n");
    
    // 模拟URC上报: +CMTI: "SM",5
    simulate_4g_response("+CMTI: \"SM\",5\r\n");
    
    while (cat_service(&at_obj) != CAT_STATUS_OK) {
        ; // 处理完成
    }
    
    printf("\n--- 场景4: 4G模组主动上报网络附着 ---\n");
    printf("URC上报: +CGATT: 1\n");
    
    // 模拟URC上报: +CGATT: 1
    simulate_4g_response("+CGATT: 1\r\n");
    
    while (cat_service(&at_obj) != CAT_STATUS_OK) {
        ; // 处理完成
    }
    
    return 0;
}

// ================================
// 9. 使用说明
// ================================

/*
 * 使用流程：
 * 
 * 1. 响应处理流程（MCU主动查询）:
 *    MCU发送命令: AT+CGATT?
 *        ↓CAT_RETURN_STATE_PRINT_CMD_LIST_OK
 *    4G模组响应: +CGATT: 1
 *        ↓
 *    cAT框架解析: 识别为+Cgatt响应，解析参数STATUS=1
 *        ↓
 *    调用回调: cgatt_read_handler()
 *        ↓
 *    应用程序处理: 根据状态做后续操作
 * 
 * 2. URC处理流程（4G模组主动上报）:
 *    4G模组上报: +CGATT: 1
 *        ↓
 *    cAT框架解析: 识别为+Cgatt URC，解析参数
 *        ↓
 *    调用回调: cgatt_urc_handler()
 *        ↓
 *    应用程序处理: 网络状态变化通知
 * 
 * 关键点：
 * - cAT框架自动识别命令和解析参数
 * - 通过变量映射自动将数据存储到应用层变量
 * - 通过回调函数处理具体的业务逻辑
 */

