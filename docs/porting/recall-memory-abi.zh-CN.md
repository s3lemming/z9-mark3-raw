# 调用相关内存、字段和ABI

所有绝对地址仅用于原Z9 5.32及以其为基础的当前变体。机器可读数值和对应源码SHA见[f2-reference.json](f2-reference.json)。结构偏移由AArch64 GCC13.3.0直接编译提取，尚不是Z8或其他编译器ABI承诺。

## 地址空间和启动内存

| 空间 | 当前规则 / 用途 |
|---|---|
| MAIN payload ROM | VA=模块文件偏移+0x40160000；不含模块末尾CRC2 |
| 已证明初始化RAM | A02808000..A02ACF000，当前配方仅在此范围用 `payload_offset=RAM_VA-0x9FDD0B000` |
| 运行RAM/BSS/堆 | 不套ROM或初始化RAM偏移公式；须通过原生产者或运行时指针确认 |
| 新增代码 | 链接入口44F6C000，具体新增函数地址由本次menu.elf/nm与build-report确定 |
| 帧状态页 | A04E3D000，预留0xC00 |
| live settings | A04E3DC00..A04E3DFE0，集成音频时上限992字节 |
| 音频运行时锚点 | A04E3DFE0，32字节，位于第一页末端 |
| 菜单状态页 | A04E3E000，第二个启动拥有页 |

当前启动配方只增加两个页。原启动代码401611CC还计算额外guard页并按16KiB对齐；盲目增加第三页可能移动原表区。不能把新增BSS空间、MMU页表和可执行映射当作同一件事。代码/资源的RO、RX边界按构建配方与原MMU消费者核对，不能在另一个机型照抄。

### 32字节锚点（相对A04E3DFE0）

| 偏移 | 字段 / 宽度 |
|---:|---|
| 0/4/8/12 | magic/state/attempts/fault，各u32 |
| 16 | allocation，u64动态堆基址 |
| 24/28 | bytes/reserved，各u32 |

初始化CAS把state0改为1，调用原40D30458分配10240字节，清零后发布pointer，再以release发布state2；失败置state3。magic为0x4D323333。读取者用acquire检查state2；只看magic或非零指针不够。运行时每次启动只分配一次，启动结束前不回收；这不授予异步PCM/DMA回收围栏。

### 动态堆分区（不是固定BSS地址）

| 基于allocation的范围 | 用途 |
|---|---|
| +0..8192 | 集成音频子区，adapter上限1536；不在这里重定义picker/PCM ABI |
| +8192..8320 | mark1保留区128字节 |
| +8320..10240 | f2保留区1920字节 |

令F=allocation+8320，实际`struct runtime`大小944，未用尾空间仍属于保留区，不是自动可复用RAM。旧独立mark2固定BSS、旧8320字节堆布局不能混入本10240字节集成合同。分配失败时逐接口检查回退，不推定所有扩展都会自动失效。

## F2运行结构

| 相对F偏移 | 字段 |
|---:|---|
| 0/4/8 | magic=0x46325231、init_state、fault |
| 12/16/20 | vr_allowed、guard_column、guard_mask |
| 24/28/32/36 | enters、exits、gets、menu_commits |
| 40/44/48/52 | last_error、saves、notifies、guard_generation |
| 56 | 856字节leaf结构 |
| 912..940（步长4） | save_gate_calls/value、save_requests、save_batch_calls/count、save_first_id、save_precheck_fault、save_return |

leaf相对自身：state+0、holder+96、callbacks+176、rows+216、event_pointers+352、events+376。三行共132字节，指针对齐产生4字节padding，不能紧贴行末按348开始读u64。state+0x28指callback、+0x50指holder、+0x48为原生已构造对象；holder+8指模板头、+16指rows、+24指事件指针数组、+32为u16行数3。init_state按0/1/2/3区分未初始化/忙/就绪/失败；拥有关系、指针链与行数一起校验，不能只按地址或magic认领菜单。

## DSID、条件键和私有编码

| 用途 | 普通列0 | 保持列1 |
|---|---|---|
| 12项原勾选掩码（u32） | 0x060014D7 | 0x06007929 |
| 原释放目标 | 0x06001792 | 0x0600793D |
| 原CH/CL速度 | 0x06001729/172A | 0x0600793B/793C |
| 原保存23字段首ID | 0x060014D8 | 0x0600792A |

基础VR为0x07000675（Off0/Normal1/Sport2），开关视图0x03000676。原getter解析DSID，不把ID数值当RAM地址。条件键0x4000012C为活动、0x40000143为活动列，属于另一编号空间：原地址函数40F27EC4的类别1基址A03D77B60，加低字节×4；两者分别A03D77C10/A03D77C6C。菜单正在编辑哪一列通过4050D134(0)返回掩码ID；历史菜单列选择器RAM A03320D94不能替代调用worker的活动列。

私有掩码：低0..11保留原checkbox（bit10 AF-ON、bit11释放），bit12勾选VR，13..14是VR目标，15..16是私有释放0原生/1C15/2C30，bit17必须0，`TAG=0xB0540000, TAG_MASK=0xFFFC0000`。上半部全零或原全一形式按legacy且不启用私有选项；未知标记、VR3、释放3及保留位错误拒绝，不猜测迁移。encode保留源低12位，不从目标结构接管它们。

## 原生接口与返回语义

| 原5.32 ROM VA | 当前调用合同 |
|---|---|
| 41019238 | getter(id,u32*out)，状态bit0成功；先原读再覆盖 |
| 403E74AC | 菜单标量提交(id,value)，提交后读回核对 |
| 403E74D8 | 菜单批量提交(ids,values,n)；与保存服务不是同一返回约定 |
| 41018524 | options(id,struct*,byte_capacity)，容量单位是字节；检查flags/count和值列表 |
| 40ED2758 / 40ED2848 | 原调用进入 / 退出；由trampoline保留被替换原指令 |
| 4050C1BC | 原保存许可，原样返回，不绕过 |
| 405338AC | 原“保存当前设定”事件action |
| 410E43A0 | 保存批处理(ids,values,n)，当前包装以返回0为成功 |
| 40EC85C4 | 通知(n,ids,flags)，不是持久化完成证明 |

getter汇编包装保留X0..X18/X30，176字节栈保持16字节对齐；返回原getter状态，C代码只修改已证明完整DSID的输出。低16位筛选只是快速路径。不要承诺所有挂钩都保留NZCV：每个站点必须按原后继消费者分别核对。原prologue本地提取16字节并重放；移植前必须检查PC相对指令/分支重定位，不能固定照搬16字节。

VR覆盖使用同代次guard：进入/退出先撤回许可、执行原动作，再按原options和活动列重新确认；getter双读偶数guard_generation，并核对mask/column，避免迟到通知跨代次覆盖。退出不把临时VR写回基础设置。

历史存储定位：普通mask在storageID0F第54记录、offset860；保持mask属于storageID53，不能放进0F尾部。紧凑镜像两者都是4B，offset1152/2248。它们是不同文件/编号空间的静态定位，不是通用物理NOR绝对地址，也不构成实机掉电保存或冷重载验收。
