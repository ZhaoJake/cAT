/*
 * 4G模组通信示例
 * 使用cAT框架与4G模组进行AT命令交互
 */

#include "src/cat.h"
#include <stdio.h>
#include <string.h>

// ================================
// 1. 定义应用层数据
// ================================

// 网络APN配置
static char apn[64] = "internet";
static char username[32] = "";
static char password[32] = "";

// SIM卡相关
static char iccid[32] = "";
static char imsi[32] = "";

// 网络状态
static uint8_t signal_strength = 0;
static uint8_t network_status = 0;

// IP地址
static char ip_address[16] = "0.0.0.0";

// ================================
// 2. 定义变量数组
// ================================

// APN配置变量
static struct cat_variable apn_vars[] = {
    {
        .name = "APN",
        .type = CAT_VAR_BUF_STRING,
        .data = apn,
        .data_size = sizeof(apn),
        .access = CAT_VAR_ACCESS_READ_WRITE,
    },
    {
        .name = "USER",
        .type = CAT_VAR_BUF_STRING,
        .data = username,
        .data_size = sizeof(username),
        .access = CAT_VAR_ACCESS_READ_WRITE,
    },
    {
        .name = "PASS",
        .type = CAT_VAR_BUF_STRING,
        .data = password,
        .data_size = sizeof(password),
        .access = CAT_VAR_ACCESS_READ_WRITE,
    }
};

// SIM信息变量
static struct cat_variable sim_vars[] = {
    {
        .name = "ICCID",
        .type = CAT_VAR_BUF_STRING,
        .data = iccid,
        .data_size = sizeof(iccid),
        .access = CAT_VAR_ACCESS_READ_ONLY,
    },
    {
        .name = "IMSI",
        .type = CAT_VAR_BUF_STRING,
        .data = imsi,
        .data_size = sizeof(imsi),
        .access = CAT_VAR_ACCESS_READ_ONLY,
    }
};

// 网络状态变量
static struct cat_variable net_vars[] = {
    {
        .name = "RSSI",
        .type = CAT_VAR_UINT_DEC,
        .data = &signal_strength,
        .data_size = sizeof(signal_strength),
        .access = CAT_VAR_ACCESS_READ_ONLY,
    },
    {
        .name = "STATUS",
        .type = CAT_VAR_UINT_DEC,
        .data = &network_status,
        .data_size = sizeof(network_status),
        .access = CAT_VAR_ACCESS_READ_ONLY,
    },
    {
        .name = "IP",
        .type = CAT_VAR_BUF_STRING,
        .data = ip_address,
        .data_size = sizeof(ip_address),
        .access = CAT_VAR_ACCESS_READ_ONLY,
    }
};

// ================================
// 3. 定义回调函数
// ================================

// APN配置写入回调
static cat_return_state apn_write_handler(const struct cat_command *cmd, 
                                         const uint8_t *data, 
                                         const size_t data_size, 
                                         const size_t args_num)
{
    printf("Setting APN: %s, USER: %s, PASS: %s\n", 
           apn, username, password);
    
    // 发送APN配置到4G模组
    // send_to_4g_module("AT+CGDCONT=1,\"IP\",\"%s\",\"%s\",\"%s\"", apn, username, password);
    
    return CAT_RETURN_STATE_OK;
}

// 读取SIM信息回调
static cat_return_state sim_read_handler(const struct cat_command *cmd, 
                                        uint8_t *data, 
                                        size_t *data_size, 
                                        const size_t max_data_size)
{
    // 从4G模组读取ICCID和IMSI
    // read_from_4g_module("AT+CCID"); // 读取ICCID
    // read_from_4g_module("AT+CIMI");  // 读取IMSI
    
    return CAT_RETURN_STATE_OK;
}

// 网络状态回调
static cat_return_state net_read_handler(const struct cat_command *cmd, 
                                        uint8_t *data, 
                                        size_t *data_size, 
                                        const size_t max_data_size)
{
    // 从4G模组读取网络状态
    // read_from_4g_module("AT+CSQ");        // 读取信号强度
    // read_from_4g_module("AT+CGATT?");     // 读取网络附着状态
    // read_from_4g_module("AT+CGPADDR=1");  // 读取IP地址
    
    return CAT_RETURN_STATE_OK;
}

// SIM卡读取回调（更新ICCID）
static int iccid_read_handler(const struct cat_variable *var)
{
    // 从4G模组读取最新ICCID
    // strcpy(iccid, read_from_4g_module("AT+CCID"));
    printf("Reading ICCID from SIM card\n");
    return 0;
}

// 信号强度读取回调
static int rssi_read_handler(const struct cat_variable *var)
{
    // 从4G模组读取最新信号强度
    // signal_strength = atoi(read_from_4g_module("AT+CSQ"));
    printf("Reading signal strength\n");
    return 0;
}

// ================================
// 4. 定义AT命令
// ================================

static struct cat_command cmds[] = {
    {
        .name = "+CGDCONT",
        .description = "Set APN configuration",
        .write = apn_write_handler,
        .var = apn_vars,
        .var_num = 3,
        .need_all_vars = true
    },
    {
        .name = "+CCID",
        .description = "Read SIM card ICCID",
        .read = sim_read_handler,
        .var = &sim_vars[0],  // 只读ICCID
        .var_num = 1
    },
    {
        .name = "+CIMI",
        .description = "Read SIM card IMSI",
        .read = sim_read_handler,
        .var = &sim_vars[1],  // 只读IMSI
        .var_num = 1
    },
    {
        .name = "+NET",
        .description = "Get network status",
        .read = net_read_handler,
        .var = net_vars,
        .var_num = 3
    }
};

// ================================
// 5. 定义命令组和描述符
// ================================

static struct cat_command_group cmd_group = {
    .name = "4g_commands",
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
    .buf_size = sizeof(working_buf)
};

// ================================
// 6. IO接口实现（连接4G模组）
// ================================

// 示例：从4G模组读取字符
static int read_from_4g_module(char *ch)
{
    // 实际实现：从串口/SPI/UART读取
    // return serial_read(ch);
    
    // 模拟实现
    *ch = getchar();
    return 1;
}

// 示例：向4G模组写入字符
static int write_to_4g_module(char ch)
{
    // 实际实现：写入串口/SPI/UART
    // return serial_write(ch);
    
    // 模拟实现
    putchar(ch);
    return 1;
}

// IO接口定义
static struct cat_io_interface io = {
    .read = read_from_4g_module,
    .write = write_to_4g_module
};

// ================================
// 7. 主程序
// ================================

int main(void)
{
    struct cat_object at_obj;
    
    // 初始化cAT框架
    cat_init(&at_obj, &desc, &io, NULL);
    
    printf("4G Module AT Command Interface\n");
    printf("Type AT commands or use '+' prefix for extended commands\n");
    printf("Example: AT+CGDCONT=\"internet\",\"\",\"\"\n");
    printf("         AT+CCID?\n");
    printf("         AT+CIMI?\n");
    printf("         AT+NET?\n");
    printf("\n");
    
    // 主循环：处理AT命令
    while (1) {
        cat_status status = cat_service(&at_obj);
        
        if (status == CAT_STATUS_OK) {
            // 空闲状态，可以做其他事情
            // 例如：检查4G模组状态
        }
        
        // 处理其他任务
        // process_4g_module_status();
    }
    
    return 0;
}

// ================================
// 8. 使用示例
// ================================

/*
 * 使用说明：
 * 
 * 1. 配置APN:
 *    AT+CGDCONT="internet","",""
 *    或
 *    AT+CGDCONT=1,"IP","internet","",""
 * 
 * 2. 读取SIM卡信息:
 *    AT+CCID?
 *    AT+CIMI?
 * 
 * 3. 查询网络状态:
 *    AT+NET?
 * 
 * 4. 自动响应:
 *    +CGDCONT=1,"IP","internet","",""
 *    OK
 *    
 *    +CCID=<ICCID:STRING[RO]>
 *    89860012345678901234
 *    OK
 * 
 * 发送命令到4G模组:
 * 1. 应用层调用: cat_service() 处理AT命令
 * 2. 框架解析: 识别命令类型和参数
 * 3. 调用回调: 执行相应的处理函数
 * 4. 回调函数: 发送实际的AT命令到4G模组
 * 5. 接收响应: 通过IO接口返回给框架
 * 6. 框架处理: 格式化并发送OK/ERROR响应
 */


