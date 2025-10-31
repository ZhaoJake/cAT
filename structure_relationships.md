# cAT框架结构体关系详解

## 📊 结构体层次关系图

```
┌─────────────────────────────────────────────────────────────┐
│                    cat_descriptor                           │
│  ┌───────────────────────────────────────────────────────┐ │
│  │  cmd_group (数组指针)                                  │ │
│  │  cmd_group_num (数量)                                  │ │
│  │  buf (工作缓冲区)                                      │ │
│  │  buf_size                                              │ │
│  │  unsolicited_buf                                       │ │
│  └───────────────────────────────────────────────────────┘ │
└─────────────────────────┬───────────────────────────────────┘
                           │ 包含
                           ↓
┌─────────────────────────────────────────────────────────────┐
│                  cat_command_group                         │
│  ┌───────────────────────────────────────────────────────┐ │
│  │  name (组名)                                           │ │
│  │  cmd (命令数组指针)                                    │ │
│  │  cmd_num (命令数量)                                    │ │
│  │  disable (禁用标志)                                    │ │
│  └───────────────────────────────────────────────────────┘ │
└─────────────────────────┬───────────────────────────────────┘
                           │ 包含多个
                           ↓
┌─────────────────────────────────────────────────────────────┐
│                    cat_command                              │
│  ┌───────────────────────────────────────────────────────┐ │
│  │  name (命令名: "+PRINT")                              │ │
│  │  description (描述)                                   │ │
│  │  write/read/run/test (回调函数)                       │ │
│  │  var (变量数组指针) ────────────────┐                │ │
│  │  var_num (变量数量)                 │                │ │
│  │  need_all_vars                      │                │ │
│  │  only_test/disable                 │                │ │
│  └─────────────────────────────────────│─────────────────┘ │
└────────────────────────────────────────┼─────────────────────┘
                                         │ 包含多个
                                         ↓
                           ┌─────────────────────────────┐
                           │    cat_variable              │
                           │  ┌─────────────────────────┐ │
                           │  │ name ("X", "Y", "MSG")   │ │
                           │  │ type (INT/UINT/STRING)   │ │
                           │  │ data (数据指针)          │ │
                           │  │ data_size                │ │
                           │  │ access (READ/WRITE)      │ │
                           │  │ write/read (回调)        │ │
                           │  └─────────────────────────┘ │
                           └─────────────────────────────┘
```

## 🔗 详细关系分析

### 1. cat_variable (变量)

**作用**: 定义单个命令参数的数据结构

```c
struct cat_variable {
    const char *name;              // 变量名称
    cat_var_type type;             // 数据类型
    void *data;                    // 数据指针
    size_t data_size;              // 数据大小
    cat_var_access access;         // 访问权限
    cat_var_write_handler write;   // 写入回调
    cat_var_read_handler read;     // 读取回调
};
```

**示例**:
```c
static uint8_t x = 0;
static struct cat_variable x_var = {
    .name = "X",
    .type = CAT_VAR_UINT_DEC,
    .data = &x,                    // 指向实际数据
    .data_size = sizeof(x),
    .access = CAT_VAR_ACCESS_READ_WRITE,
};
```

### 2. cat_command (命令)

**作用**: 定义一个AT命令及其关联的变量

```c
struct cat_command {
    const char *name;                      // 命令名
    const char *description;              // 描述
    cat_cmd_write_handler write;          // WRITE回调
    cat_cmd_read_handler read;            // READ回调
    cat_cmd_run_handler run;              // RUN回调
    cat_cmd_test_handler test;            // TEST回调
    struct cat_variable const *var;       // 变量数组
    size_t var_num;                       // 变量数量
    bool need_all_vars;                   // 是否需要所有变量
    bool only_test;                       // 仅测试
    bool disable;                         // 禁用
    bool implicit_write;                  // 隐式写入
};
```

**关键关系**:
- 一个命令包含0到多个变量
- 通过`var`指针引用变量数组
- 通过`var_num`指定变量数量

**示例**:
```c
static struct cat_variable print_vars[] = {
    { .name = "X", ... },
    { .name = "Y", ... },
    { .name = "MESSAGE", ... }
};

static struct cat_command print_cmd = {
    .name = "+PRINT",
    .description = "Print at (X,Y)",
    .var = print_vars,            // 关联变量数组
    .var_num = 3,                 // 3个变量
    .need_all_vars = true
};
```

### 3. cat_command_group (命令组)

**作用**: 将多个命令组织在一起

```c
struct cat_command_group {
    const char *name;                      // 组名
    struct cat_command const *cmd;        // 命令数组
    size_t cmd_num;                        // 命令数量
    bool disable;                          // 禁用整组
};
```

**关键关系**:
- 一个命令组包含多个命令
- 通过`cmd`指针引用命令数组
- 可以批量禁用整组命令

**示例**:
```c
static struct cat_command cmds[] = {
    { .name = "+PRINT", ... },
    { .name = "#HELP", ... },
    { .name = "#QUIT", ... }
};

static struct cat_command_group cmd_group = {
    .name = "print_commands",
    .cmd = cmds,                  // 关联命令数组
    .cmd_num = 3                   // 3个命令
};
```

### 4. cat_descriptor (解析器描述符)

**作用**: 顶层配置，包含所有命令组和缓冲区

```c
struct cat_descriptor {
    struct cat_command_group* const *cmd_group;  // 命令组数组
    size_t cmd_group_num;                        // 组数量
    uint8_t *buf;                                // 工作缓冲区
    size_t buf_size;                             // 缓冲区大小
    uint8_t *unsolicited_buf;                    // Unsolicited缓冲区
    size_t unsolicited_buf_size;                 // Unsolicited大小
};
```

**关键关系**:
- 包含多个命令组
- 提供工作缓冲区
- 是框架的根配置

**示例**:
```c
static struct cat_command_group *groups[] = {
    &cmd_group
};

static struct cat_descriptor desc = {
    .cmd_group = groups,          // 命令组数组
    .cmd_group_num = 1,            // 1个组
    .buf = buf,                    // 工作缓冲区
    .buf_size = sizeof(buf)
};
```

## 📈 完整数据流示例

```c
// 1. 定义应用数据
static uint8_t x = 0, y = 0;
static char msg[16] = "";

// 2. 定义变量
static struct cat_variable print_vars[] = {
    {.name = "X", .type = CAT_VAR_UINT_DEC, .data = &x, ...},
    {.name = "Y", .type = CAT_VAR_UINT_DEC, .data = &y, ...},
    {.name = "MSG", .type = CAT_VAR_BUF_STRING, .data = msg, ...}
};

// 3. 定义命令
static struct cat_command cmds[] = {
    {.name = "+PRINT", .var = print_vars, .var_num = 3, ...},
    {.name = "#HELP", ...}
};

// 4. 定义命令组
static struct cat_command_group cmd_group = {
    .cmd = cmds,
    .cmd_num = 2
};

// 5. 定义描述符
static char buf[128];
static struct cat_command_group *groups[] = {&cmd_group};
static struct cat_descriptor desc = {
    .cmd_group = groups,
    .cmd_group_num = 1,
    .buf = buf,
    .buf_size = sizeof(buf)
};
```

## 🎯 关系总结

| 结构体 | 包含 | 层次 | 作用 |
|--------|------|------|------|
| cat_descriptor | cat_command_group[] | 最高 | 全局配置 |
| cat_command_group | cat_command[] | 高 | 命令分组 |
| cat_command | cat_variable[] | 中 | 单条命令 |
| cat_variable | - | 最低 | 单个参数 |

## 💡 关键要点

1. **分层组织**: descriptor → command_group → command → variable
2. **数组引用**: 每层都使用数组指针引用下层数组
3. **灵活配置**: 可以禁用命令、命令组
4. **数据绑定**: variable通过data指针绑定应用数据
5. **回调关联**: command和variable都可定义回调函数

## 🔄 初始化流程

```
1. 定义应用数据变量 (x, y, msg)
   ↓
2. 创建 cat_variable 数组 (print_vars)
   ↓
3. 创建 cat_command 数组 (cmds)
   ↓
4. 创建 cat_command_group (cmd_group)
   ↓
5. 创建 cat_descriptor (desc)
   ↓
6. 调用 cat_init(&obj, &desc, &io, &mutex)
```

