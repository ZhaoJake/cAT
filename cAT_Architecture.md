# cAT AT命令解析框架架构图

## 整体架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        cAT Framework                            │
├─────────────────────────────────────────────────────────────────┤
│  Application Layer                                             │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐ │
│  │   Command       │  │   Variable      │  │   Callback      │ │
│  │   Handlers      │  │   Handlers      │  │   Functions     │ │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│  Core Engine Layer                                              │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │                cat_service()                               │ │
│  │  ┌─────────────────┐              ┌─────────────────┐     │ │
│  │  │   Main FSM      │              │ Unsolicited FSM │     │ │
│  │  │                 │              │                 │     │ │
│  │  │ IDLE → PARSE →  │              │ IDLE → FORMAT → │     │ │
│  │  │ SEARCH → FOUND  │              │ READ/TEST LOOP  │     │ │
│  │  │ READ/WRITE/TEST │              │ FLUSH → RESET   │     │ │
│  │  │ RUN LOOP        │              │                 │     │ │
│  │  └─────────────────┘              └─────────────────┘     │ │
│  └─────────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│  Interface Layer                                                │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐ │
│  │   IO Interface  │  │  Mutex Interface│  │   Buffer Mgmt   │ │
│  │   read/write    │  │   lock/unlock   │  │   AT/Unsolicited│ │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

## 数据流图

```
Input Stream → IO Interface → Main FSM → Command Processing
     ↓              ↓            ↓              ↓
   "AT+CMD?"   →  read()   →  PARSE_CHAR  →  SEARCH_CMD
     ↓              ↓            ↓              ↓
   "AT+CMD=123" → read()   →  PARSE_ARGS  →  WRITE_LOOP
     ↓              ↓            ↓              ↓
   "AT+CMD"     → read()   →  RUN_LOOP    →  Callback
     ↓              ↓            ↓              ↓
   Unsolicited → Buffer   →  Unsolicited →  Format & Send
   Events       → Queue   →  FSM         →  Response
```

## 状态机详细流程

### Main FSM States
```
IDLE
  ↓ (receive 'A')
PARSE_PREFIX
  ↓ (receive 'T')
PARSE_COMMAND_CHAR
  ↓ (receive command chars)
UPDATE_COMMAND_STATE
  ↓ (process all commands)
SEARCH_COMMAND
  ↓ (find best match)
COMMAND_FOUND
  ↓ (based on command type)
┌─────────────────────────────────────────┐
│ READ_LOOP    WRITE_LOOP   TEST_LOOP     │
│ (AT+CMD?)    (AT+CMD=val) (AT+CMD=?)    │
│     ↓            ↓            ↓         │
│ FORMAT_READ  PARSE_WRITE  FORMAT_TEST   │
│     ↓            ↓            ↓         │
│ FLUSH_IO     WRITE_LOOP   FLUSH_IO      │
└─────────────────────────────────────────┘
  ↓
AFTER_FLUSH_OK/RESET
  ↓
IDLE
```

### Unsolicited FSM States
```
IDLE
  ↓ (check buffer)
FORMAT_READ_ARGS / FORMAT_TEST_ARGS
  ↓ (format variables)
READ_LOOP / TEST_LOOP
  ↓ (call callbacks)
FLUSH_IO_WRITE
  ↓ (send response)
AFTER_FLUSH_OK/RESET
  ↓
IDLE
```

## 核心组件关系

```
cat_object
├── cat_descriptor
│   ├── cat_command_group[]
│   │   └── cat_command[]
│   │       ├── cat_variable[]
│   │       └── callback functions
│   └── buffers (AT + Unsolicited)
├── cat_io_interface
│   ├── read()
│   └── write()
├── cat_mutex_interface (optional)
│   ├── lock()
│   └── unlock()
└── cat_unsolicited_fsm
    ├── unsolicited_cmd_buffer[]
    └── state machine
```

## 命令类型处理

| Command Type | Format        | Handler Called | Purpose                    |
|--------------|---------------|----------------|----------------------------|
| RUN          | AT+CMD        | run()          | Execute command            |
| READ         | AT+CMD?       | read()         | Read current values        |
| WRITE        | AT+CMD=value  | write()        | Set new values             |
| TEST         | AT+CMD=?      | test()         | Show parameter format      |

## 变量类型支持

| Variable Type | Format Example | Parsing Method           |
|---------------|----------------|--------------------------|
| INT_DEC       | -123, +456     | parse_int_decimal()      |
| UINT_DEC      | 123, 456       | parse_uint_decimal()     |
| NUM_HEX       | 0x1A, 0xFF     | parse_num_hexadecimal()  |
| BUF_HEX       | 1A2B3C4D       | parse_buffer_hexadecimal()|
| BUF_STRING    | "hello world"  | parse_buffer_string()    |

## 关键特性

1. **异步处理**: 非阻塞状态机设计
2. **并发支持**: 主FSM + Unsolicited FSM并行
3. **线程安全**: 可选互斥锁保护
4. **状态保持**: HOLD机制支持长时间操作
5. **缓冲区管理**: 自动缓冲区分配和管理
6. **错误处理**: 完整的错误状态和恢复机制
7. **扩展性**: 支持自定义命令和变量类型
