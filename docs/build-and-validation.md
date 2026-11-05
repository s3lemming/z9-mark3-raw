# 构建、验证与公开边界

本工程冻结集成v39，代码基线是Z9 5.32。新写的94份固件源/头文件由profile登记SHA；完整原模块、源码身份、生成资源哈希、66处原空间补丁窗口及最终产物身份均严格检查。新运行时probe未默认启用。

## 环境与输入

Ubuntu24.04/WSL、Python3.12、GCC AArch6413.3.0、GNU binutils2.42已用于独立重建。普通宿主合成测试需GCC和NumPy。原模块SHA见profile，需自行合法提供。构建不下载固件或私有材料，Python不得使用-O，输出目录必须新建且在仓库外。

```sh
python3 -B tests/run.py --output /private/host-tests
python3 -B tools/build.py --module /private/eg1990_053200.bi --output /private/build --version 5.32
```

build.py只从精确模块抽取菜单布局、事件、原getter和被覆盖指令。原厂340B布局及2156B模板不包含在仓库；recipe保存偏移/长度、项目自有改动和预期哈希，在本地生成。未把原厂资源数组或原指令字直接公开。

可选择两个参数`--version 5.60`，其业务与v39相同；完整版本差异只有4处标识的8字节。两份精确MAIN身份：

| 标识 | MAIN SHA256 | 完整包 SHA256 |
|---|---|---|
|5.32|6c768c7f153338e1b32ecb58ce51c8f4517461b21d8fa3bd944c7d19e22181cc|da818964309b89fab77c13f7b615ac7e39e7fe9f78b2dabbacfd83b404c5c942|
|5.60|48e3de3dbb7f7ce97bcf1690bd8a578b6fce6502d463eba36153afca3943b94d|4d0c2b6bc10c7875df18de4bf015f813b903fc3445cfd84fc18b406703bf13db|

## 完整容器可选步骤

还需原5.32完整容器，以及分开提供的私有格式输入。`--tables`是含T1/T2/T3各256B十六进制字符串的JSON；`--integrity-material`是含K8八字节十六进制字符串的JSON。不附真实值或获取方法，不在日志输出正文。

```sh
python3 -B tools/pack.py pack --stock /private/original.bin --tables /private/tables.json --integrity-material /private/material.json --payload /private/build/payload.bin --version 5.32 --output /private/Z_9_0532.bin
python3 -B tools/pack.py verify --stock /private/original.bin --tables /private/tables.json --integrity-material /private/material.json --candidate /private/Z_9_0532.bin --version 5.32
```

5.60选择对应payload和`--version 5.60`。其他五组件保持原字节、目录模块名保持原值；重算模块CRC、整包CRC及摘要，独立parser逐字节回验并检查冻结整包身份后才写最终BIN。完整性不是厂商私钥签名或设备接受保证。

## 测试层级

CI仅跑原输入无关的策略、设置载荷、虚拟Nrmap、预拍策略和合成封包测试。预拍转换在宿主矩阵里明确为夹具；保留的test_native_precapture_limit.c是需原模块、AArch64地址映射的本地原指令研究，不纳入公开host CI。未把它错误当无固件的宿主程序。

对应历史证据：v39精确MAIN冷菜单/拍后重入及实际C15声音/JPEG通过；用户报告菜单重启和C15/C30快速裁切正常。v37按帧虚拟map落盘与原生合流通过。HE★/DX/预拍等更广矩阵有各自历史分项证据，不提高为本轮全部实机资格。5.60原升级函数/SHA/CRC/提取、705块模拟NOR写入和完整读回通过，但升级UI政策、完整重启与实机接受未覆盖。

## 手册与截图

```sh
python3 -B tools/build_manual.py --font /private/CJK.ttf --output /private/v39-manual.pdf
```

需reportlab、pypdf、Pillow；字体由本地合法提供，不分发字体文件。PDF从manual-content.json生成，菜单PNG有原栅格SHA和精确MAIN来源。生成后必须渲染逐页检查，不用文字提取代替排版验收。

## 边界

公开内容：新写C/汇编配方、短ABI常量、地址/哈希、合成测试、研究结论、少量经请求的菜单截图和PDF。排除：原/修改固件、原厂资源数组、私有格式材料、原反编译正文、实拍照片、个人音频、设备标识、主机路径与内部日志。截图包含厂商UI，仅用于说明，不主张其所有权；研究源/工具的GPL许可不扩大到这些厂商对象。

截图的公开转载授权尚未确认，按发布者明确选择保留并披露。PDF文档嵌入字体不等于允许分发字体文件。公开源码和本地构建接口不构成固件修改、逆向、封包或分发的法律许可，详见[公开内容审计与权属边界](publication-boundary.zh-CN.md)。
