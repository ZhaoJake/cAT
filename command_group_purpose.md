# cat_command_group 创建命令组的目的

## 📋 命令组的核心定义

```c
struct cat_command_group {
    const char *name;                      // 组名（用于标识）
    struct cat_command const *cmd;        // 命令数组指针
    size_t cmd_num;                        // 命令数量
    bool disable;                          // 批量禁用标志
};
```

## 🎯 创建命令组的主要目的

### 1. 📦 **批量管理和组织命令**

**目的**: 将相关的命令组织在一起，便于管理和维护

```c
// 示例：将打印相关的命令组织在一起
static struct cat_command print_cmds[] = {
    {.name = "+PRINT", ...},
    {.name = "+CLEAR", ...},
    {.name = "+PAGE", ...}
};

static struct cat_command_group print_group = {
    .name = "print_commands",      // 组名标识
    .cmd = print_cmds,
    .cmd_num = 3
};
```

**优势**:
- 逻辑上相关的命令集中管理
- 代码结构更清晰
- 便于维护和扩展

### 2. 🚫 **批量禁用功能模块**

**目的**: 通过设置`disable`标志，可以一次性禁用整个命令组

```c
// 在 is_command_disable() 函数中 (src/cat.c:748-775)
static bool is_command_disable(struct cat_object *self, size_t index)
{
    ...
    // 检查命令组是否被禁用
    if (cmd_group->disable != false)
        return true;  // 整组禁用
    
    // 检查单个命令是否被禁用
    if (cmd_group->cmd[index - j].disable != false)
        return true;  // 单个禁用
    
    return false;
}
```

**应用场景**:
```c
// 根据编译选项或配置禁用某些功能模块
#ifdef DISABLE_WIFI
static struct cat_command_group wifi_group = {
    .name = "wifi_commands",
    .cmd = wifi_cmds,
    .cmd_num = 5,
    .disable = true  // 整个WIFI命令组被禁用
};
#endif

// 运行时动态禁用
void disable_debug_commands(void) {
    debug_group.disable = true;
}
```

### 3. 🔍 **模块化开发和支持**

**目的**: 不同的开发团队或模块可以维护自己的命令组

```c
// 模块化组织
// 模块1：系统命令
static struct cat_command system_cmds[] = {
    {.name = "#RESET", ...},
    {.name = "#VERSION", ...}
};
static struct cat_command_group system_group = {
    .name = "system",
    .cmd = system_cmds,
    .cmd_num = 2
};

// 模块2：WiFi命令
static struct cat_command wifi_cmds[] = {
    {.name = "+WIFI", ...},
    {.name = "+WIFI_CONN", ...}
};
static struct cat_command_group wifi_group = {
    .name = "wifi",
    .cmd = wifi_cmds,
    .cmd_num = 2
};

// 模块3：传感器命令
static struct cat_command sensor_cmds[] = {
    {.name = "+TEMP", ...},
    {.name = "+HUMIDITY", ...}
};
static struct cat_command_group sensor_group = {
    .name = "sensor",
    .cmd = sensor_cmds,
    .cmd_num = 2
};

// 注册所有模块
static struct cat_command_group *groups[] = {
    &system_group,
    &wifi_group,
    &sensor_group
};
```

### 4. 🔢 **命令数量统计和初始化**

**目的**: 在初始化时统计所有命令的总数

```c
// 在 cat_init() 函数中 (src/cat.c:454-498)
void cat_init(struct cat_object *self, ...)
{
    ...
    self->commands_num = 0;
    for (i = 0; i < desc->cmd_group_num; i++) {
        cmd_group = desc->cmd_group[i];
        
        // 累计每个组的命令数
        self->commands_num += cmd_group->cmd_num;
        
        // 遍历组内的每个命令进行验证
        for (j = 0; j < cmd_group->cmd_num; j++) {
            assert(cmd_group->cmd[j].name != NULL);
            ...
        }
    }
    
    // 缓冲区大小验证：需要足够大来存储匹配状态
    assert(desc->buf_size * 4U >= self->commands_num);
}
```

### 5. 🔎 **按名称搜索命令组**

**目的**: 提供按名称查找命令组的功能

```c
// cat_search_command_group_by_name() (src/cat.c:2409-2424)
struct cat_command_group const* cat_search_command_group_by_name(struct cat_object *self, const char *name)
{
    for (i = 0; i < self->desc->cmd_group_num; i++) {
        cmd_group = self->desc->cmd_group[i];
        if ((cmd_group->name != NULL) && (strcmp(cmd_group->name, name) == 0))
            return cmd_group;
    }
    return NULL;
}
```

**使用示例**:
```c
// 运行时查找并操作命令组
const struct cat_command_group *group = cat_search_command_group_by_name(&obj, "wifi");
if (group != NULL) {
    printf("Found WiFi command group with %zu commands\n", group->cmd_num);
    // 可以动态禁用
    ((struct cat_command_group*)group)->disable = true;
}
```

## 📊 命令组在代码中的使用

### 1. **初始化验证**
```c
// 遍历所有命令组，进行初始化验证
for (i = 0; i < desc->cmd_group_num; i++) {
    cmd_group = desc->cmd_group[i];
    
    // 验证组的完整性
    assert(cmd_group->cmd != NULL);
    assert(cmd_group->cmd_num > 0);
    
    // 统计命令总数
    self->commands_num += cmd_group->cmd_num;
}
```

### 2. **命令查找**
```c
// get_command_by_index() 通过索引找到对应的命令
// 需要遍历命令组来确定位置
static struct cat_command const* get_command_by_index(...)
{
    ...
    j = 0;
    for (i = 0; i < self->desc->cmd_group_num; i++) {
        cmd_group = self->desc->cmd_group[i];
        
        if (index >= j + cmd_group->cmd_num) {
            j += cmd_group->cmd_num;
            continue;
        }
        
        return &cmd_group->cmd[index - j];
    }
}
```

### 3. **禁用检查**
```c
// 检查命令是否被禁用（考虑组级别的禁用）
static bool is_command_disable(struct cat_object *self, size_t index)
{
    ...
    for (i = 0; i < self->desc->cmd_group_num; i++) {
        cmd_group = self->desc->cmd_group[i];
        
        // 如果索引超出了当前组的范围，继续查找
        if (index >= j + cmd_group->cmd_num) {
            j += cmd_group->cmd_num;
            continue;
        }
        
        // 检查命令组是否被禁用
        if (cmd_group->disable != false)
            return true;
        
        // 检查单个命令是否被禁用
        if (cmd_group->cmd[index - j].disable != false)
            return true;
        
        break;
    }
    
    return false;
}
```

## 💡 实际应用场景

### 场景1: 编译时功能选择
```c
// 基础命令组
static struct cat_command_group basic_group = {
    .name = "basic",
    .cmd = basic_cmds,
    .cmd_num = 3
};

// 可选的WiFi功能
#ifdef ENABLE_WIFI
static struct cat_command_group wifi_group = {
    .name = "wifi",
    .cmd = wifi_cmds,
    .cmd_num = 5,
    .disable = false
};
#endif

// 可选的蓝牙功能
#ifdef ENABLE_BLUETOOTH
static struct cat_command_group bt_group = {
    .name = "bluetooth",
    .cmd = bt_cmds,
    .cmd_num = 4,
    .disable = false
};
#endif
```

### 场景2: 运行时动态控制
```c
void enable_debug_mode(bool enable) {
    debug_group.disable = !enable;
}

void disable_privileged_commands(void) {
    privileged_group.disable = true;
}
```

### 场景3: 模块化开发
```c
// sensor_module.c
void sensor_init(void) {
    sensor_group.disable = false;
}

void sensor_deinit(void) {
    sensor_group.disable = true;
}

// main.c
int main(void) {
    // 初始化后注册命令组
    static struct cat_command_group *all_groups[] = {
        &system_group,
        &sensor_group,
        &wifi_group
    };
    
    static struct cat_descriptor desc = {
        .cmd_group = all_groups,
        .cmd_group_num = 3
    };
    
    cat_init(&obj, &desc, &io, NULL);
}
```

## 🎯 总结

命令组的主要目的：

1. ✅ **组织管理**: 将相关命令分组，代码更清晰
2. ✅ **批量控制**: 通过`disable`标志批量启用/禁用功能模块
3. ✅ **模块化**: 支持多模块开发和维护
4. ✅ **灵活配置**: 编译时和运行时都可控制命令的可用性
5. ✅ **易于扩展**: 新增功能只需添加新的命令组

命令组为cAT框架提供了**模块化、可配置、易维护**的命令组织方式，使得大型项目能够更好地管理大量AT命令。

