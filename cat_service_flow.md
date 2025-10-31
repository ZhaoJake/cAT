# cat_service 接口执行流程详解

## 📋 函数签名

```c
cat_status cat_service(struct cat_object *self)
```

## 🔄 整体执行流程图

```
cat_service()
├── [1] 锁检查与获取
│   ├── 检查 mutex 是否存在
│   └── 存在则调用 lock() 加锁
│
├── [2] Unsolicited事件服务
│   └── unsolicited_events_service(self)
│       └── 处理非请求事件状态机
│
├── [3] 主状态机处理
│   └── switch(self->state) 根据当前状态分发处理
│
├── [4] 状态判断
│   ├── 检查 unsolicited 状态
│   └── 如忙碌则返回 BUSY
│
└── [5] 解锁与返回
    ├── 调用 unlock() 解锁
    └── 返回状态码
```

## 📊 详细状态处理

### 一、初始化阶段 (IDLE)
```c
case CAT_STATE_IDLE:
    └── process_idle_state(self)
        ├── read_cmd_char(self)  // 读取字符
        ├── 检测到 'A' 字符
        └── 切换到 PARSE_PREFIX 状态
```

### 二、命令解析阶段

#### 1. 解析前缀 (PARSE_PREFIX)
```c
case CAT_STATE_PARSE_PREFIX:
    └── parse_prefix(self)
        ├── 读取下一个字符
        ├── 期望 'T' 字符
        └── 切换到 PARSE_COMMAND_CHAR
```

#### 2. 解析命令字符 (PARSE_COMMAND_CHAR)
```c
case CAT_STATE_PARSE_COMMAND_CHAR:
    └── parse_command(self)
        ├── 解析命令名称
        ├── 检测命令类型:
        │   ├── '?' → READ类型
        │   ├── '=' → WRITE类型
        │   └── '\n' → RUN类型
        └── 切换到相应状态
```

#### 3. 更新命令状态 (UPDATE_COMMAND_STATE)
```c
case CAT_STATE_UPDATE_COMMAND_STATE:
    └── update_command(self)
        ├── 遍历所有命令
        ├── 比较命令名称
        └── 更新匹配状态:
            ├── NOT_MATCH (0)
            ├── PARTIAL_MATCH (1)
            └── FULL_MATCH (2)
```

#### 4. 搜索命令 (SEARCH_COMMAND)
```c
case CAT_STATE_SEARCH_COMMAND:
    └── search_command(self)
        ├── 查找最佳匹配命令
        ├── 处理部分匹配
        └── 决定:
            ├── COMMAND_FOUND
            └── COMMAND_NOT_FOUND
```

### 三、命令处理阶段

#### 1. 命令已找到 (COMMAND_FOUND)
```c
case CAT_STATE_COMMAND_FOUND:
    └── command_found(self)
        ├── 根据命令类型分发:
        │   ├── RUN → RUN_LOOP
        │   ├── READ → FORMAT_READ_ARGS
        │   └── WRITE → PARSE_COMMAND_ARGS
        └── 命令未找到
            └── ack_error(self)
```

#### 2. 写入循环 (WRITE_LOOP)
```c
case CAT_STATE_WRITE_LOOP:
    └── process_write_loop(self)
        ├── 调用 cmd->write() 回调
        ├── 处理返回值:
        │   ├── OK → 发送 OK
        │   ├── HOLD → 进入保持状态
        │   └── ERROR → 发送 ERROR
        └── 继续或退出循环
```

#### 3. 读取循环 (READ_LOOP)
```c
case CAT_STATE_READ_LOOP:
    └── process_read_loop(self, CAT_FSM_TYPE_ATCMD)
        ├── 调用 cmd->read() 回调
        ├── 处理返回值:
        │   ├── DATA_OK → 格式化并发送
        │   ├── DATA_NEXT → 继续格式化
        │   └── HOLD → 进入保持状态
        └── 继续或完成
```

#### 4. 测试循环 (TEST_LOOP)
```c
case CAT_STATE_TEST_LOOP:
    └── process_test_loop(self, CAT_FSM_TYPE_ATCMD)
        ├── 调用 cmd->test() 回调
        ├── 自动生成参数格式
        └── 发送测试响应
```

#### 5. 运行循环 (RUN_LOOP)
```c
case CAT_STATE_RUN_LOOP:
    └── process_run_loop(self)
        ├── 调用 cmd->run() 回调
        ├── 处理返回值:
        │   ├── OK → 发送 OK
        │   ├── HOLD → 进入保持状态
        │   └── ERROR → 发送 ERROR
        └── 执行命令逻辑
```

### 四、参数解析阶段

#### 1. 解析命令参数 (PARSE_COMMAND_ARGS)
```c
case CAT_STATE_PARSE_COMMAND_ARGS:
    └── parse_command_args(self)
        ├── 逐字符读取参数
        ├── 检测 '?' → 转换为 TEST类型
        └── 参数结束 → 切换到 PARSE_WRITE_ARGS
```

#### 2. 解析写入参数 (PARSE_WRITE_ARGS)
```c
case CAT_STATE_PARSE_WRITE_ARGS:
    └── parse_write_args(self)
        ├── 根据变量类型解析:
        │   ├── INT_DEC → parse_int_decimal()
        │   ├── UINT_DEC → parse_uint_decimal()
        │   ├── NUM_HEX → parse_num_hexadecimal()
        │   ├── BUF_HEX → parse_buffer_hexadecimal()
        │   └── BUF_STRING → parse_buffer_string()
        ├── 验证数据范围
        ├── 调用 var->write() 回调
        └── 继续或完成
```

### 五、响应格式化阶段

#### 1. 格式化读取参数 (FORMAT_READ_ARGS)
```c
case CAT_STATE_FORMAT_READ_ARGS:
    └── format_read_args(self, CAT_FSM_TYPE_ATCMD)
        ├── 调用 var->read() 回调
        ├── 根据类型格式化:
        │   ├── INT_DEC → format_int_decimal()
        │   ├── UINT_DEC → format_uint_decimal()
        │   ├── NUM_HEX → format_num_hexadecimal()
        │   ├── BUF_HEX → format_buffer_hexadecimal()
        │   └── BUF_STRING → format_buffer_string()
        ├── 添加分隔符
        └── 继续或发送
```

#### 2. 格式化测试参数 (FORMAT_TEST_ARGS)
```c
case CAT_STATE_FORMAT_TEST_ARGS:
    └── format_test_args(self, CAT_FSM_TYPE_ATCMD)
        ├── 生成参数格式信息:
        │   └── "<NAME:TYPE[ACCESS]>"
        ├── 添加分隔符
        └── 发送格式信息
```

### 六、IO刷新阶段

#### 1. IO写入等待 (FLUSH_IO_WRITE_WAIT)
```c
case CAT_STATE_FLUSH_IO_WRITE_WAIT:
    └── process_io_write_wait(self)
        ├── 检查 unsolicited FSM 状态
        └── 切换到 FLUSH_IO_WRITE
```

#### 2. IO写入 (FLUSH_IO_WRITE)
```c
case CAT_STATE_FLUSH_IO_WRITE:
    └── process_io_write(self)
        ├── 逐字符写入到IO:
        │   ├── 新行字符 (CRLF/LF)
        │   ├── 数据缓冲区
        │   └── 结束标志
        ├── 字符发送完成
        └── 切换到 AFTER_FLUSH_* 状态
```

### 七、收尾阶段

#### 1. 刷新后重置 (AFTER_FLUSH_RESET)
```c
case CAT_STATE_AFTER_FLUSH_RESET:
    └── reset_state(self)
        ├── 重置状态为 IDLE
        ├── 清除命令引用
        └── 准备下一个命令
```

#### 2. 刷新后OK (AFTER_FLUSH_OK)
```c
case CAT_STATE_AFTER_FLUSH_OK:
    └── ack_ok(self)
        ├── 生成 "OK" 响应
        └── 开始 IO 写入流程
```

#### 3. 刷新后格式化 (AFTER_FLUSH_FORMAT_READ_ARGS)
```c
case CAT_STATE_AFTER_FLUSH_FORMAT_READ_ARGS:
    └── start_processing_format_read_args()
        └── 继续读取下一个变量
```

### 八、特殊状态

#### 1. 错误状态 (ERROR)
```c
case CAT_STATE_ERROR:
    └── error_state(self)
        ├── 读取剩余字符
        ├── 等待 '\n'
        └── 发送 "ERROR" 响应
```

#### 2. 保持状态 (HOLD)
```c
case CAT_STATE_HOLD:
    └── process_hold_state(self)
        ├── 检查 hold_exit_status
        ├── 如退出则发送 OK/ERROR
        └── 等待用户退出HOLD状态
```

#### 3. 打印命令列表 (PRINT_CMD)
```c
case CAT_STATE_PRINT_CMD:
    └── print_cmd_list(self)
        ├── 遍历所有命令
        ├── 根据命令类型打印:
        │   ├── RUN → "AT+CMD"
        │   ├── READ → "AT+CMD?"
        │   ├── WRITE → "AT+CMD="
        │   └── TEST → "AT+CMD=?"
        └── 发送到IO
```

## 🔗 Unsolicited 事件服务流程

```c
unsolicited_events_service()
├── 检查缓冲区
├── 弹出事件
└── 根据事件类型:
    ├── READ → FORMAT_READ_ARGS
    └── TEST → FORMAT_TEST_ARGS
```

## 📈 状态转换示例

### 完整的RUN命令流程:
```
IDLE → PARSE_PREFIX → PARSE_COMMAND_CHAR 
  → UPDATE_COMMAND_STATE → SEARCH_COMMAND 
  → COMMAND_FOUND → RUN_LOOP 
  → FLUSH_IO_WRITE_WAIT → FLUSH_IO_WRITE 
  → AFTER_FLUSH_OK → FLUSH_IO_WRITE 
  → AFTER_FLUSH_RESET → IDLE
```

### 完整的READ命令流程:
```
IDLE → PARSE_PREFIX → PARSE_COMMAND_CHAR 
  → UPDATE_COMMAND_STATE → SEARCH_COMMAND 
  → COMMAND_FOUND → FORMAT_READ_ARGS 
  → READ_LOOP → FLUSH_IO_WRITE_WAIT 
  → FLUSH_IO_WRITE → AFTER_FLUSH_OK 
  → FLUSH_IO_WRITE → AFTER_FLUSH_RESET 
  → IDLE
```

### 完整的WRITE命令流程:
```
IDLE → PARSE_PREFIX → PARSE_COMMAND_CHAR 
  → UPDATE_COMMAND_STATE → SEARCH_COMMAND 
  → COMMAND_FOUND → PARSE_COMMAND_ARGS 
  → PARSE_WRITE_ARGS → WRITE_LOOP 
  → FLUSH_IO_WRITE_WAIT → FLUSH_IO_WRITE 
  → AFTER_FLUSH_OK → FLUSH_IO_WRITE 
  → AFTER_FLUSH_RESET → IDLE
```

## ⚙️ 关键机制

### 1. 异步非阻塞
- 每次调用只处理一个状态
- 需要多次调用才能完成一个命令
- 返回 `CAT_STATUS_BUSY` 表示需要继续调用

### 2. 状态保持 (HOLD)
- 用于长时间操作
- 退出时返回 OK/ERROR
- 通过 `cat_hold_exit()` 退出

### 3. 并发处理
- 主FSM和Unsolicited FSM并行运行
- Unsolicited事件可以中断主流程
- 通过状态检查协调两者

### 4. IO操作
- 逐字符读取和写入
- 支持CRLF和LF两种格式
- 异步处理，不会阻塞

## 🎯 返回值说明

| 返回值 | 含义 |
|--------|------|
| `CAT_STATUS_OK` | 空闲，无工作 |
| `CAT_STATUS_BUSY` | 忙碌，需要继续调用 |
| `CAT_STATUS_HOLD` | 处于保持状态 |
| `CAT_STATUS_ERROR_*` | 各种错误状态 |

## 💡 使用建议

1. **周期性调用**: 在应用主循环中周期性调用
2. **检查返回值**: 根据返回值决定是否继续调用
3. **处理HOLD状态**: 对于长时间操作使用HOLD机制
4. **IO准备**: 确保IO接口可用且非阻塞

