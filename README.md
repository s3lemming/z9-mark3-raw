## f2/Fn1 与预拍摄菜单更新

新增[可选f2菜单变体](docs/f2-recall-precapture.zh-CN.md)：普通/保持调用分别增加防抖与C15/C30，支持严格重建5.61开发标签。原v39入口和手册保持历史版本；实机、额定帧率及其余未闭环范围见说明。

# Z9 mark3 RAW - integrated v39

针对 Nikon Z9 原始5.32的实验性源码与研究文档。C15/C30增加HE、HE★ RAW/RAW+JPEG、预拍/DX及分组菜单，包含集成mark2冷启动音频菜单修复。5.60是同一v39业务实现的版本标识变体，不是官方5.60实现。

- [PDF 使用手册](docs/v39-user-manual.zh-CN.pdf) / [可检索原文](docs/v39-user-manual.zh-CN.md)
- [功能与改动点](docs/changes.zh-CN.md)
- [NEF研究、功能影响与未完成项](docs/nef-research.zh-CN.md)
- [参考证据登记](docs/evidence-register.json) / [构建与验证](docs/build-and-validation.md)
- [原始菜单截图及来源](docs/assets/screens/provenance.json)
- [公开内容审计、第三方权属与授权未确认项](docs/publication-boundary.zh-CN.md)

**第三方内容说明：** 本仓库保留4张尼康菜单截图及带截图PDF；其公开转载授权尚未确认。截图权利仍属相应权利人，项目GPL不覆盖这些厂商UI。保留截图是发布者在明确获知该不确定性后的选择。源码发布也不等于对固件修改、逆向或封包工具的法律许可。

v39用户反馈：音频菜单重启正常，C15/C30 RAW快速裁切执行正常。虚拟Nrmap0的数值画质、另一12位辅助图、剩余七段ShotInfo、半按rXX精确显示及长期/全矩阵验收没有因此自动完成。RAW处理与回放放大不是相同验收，见研究文档。

## 本地构建

需要Python3、GCC AArch64 13.3.0、binutils2.42；使用自己提供的精确原始模块。不同工具链产生不同字节会被拒绝。输出必须在仓库外，包含派生固件，应保持私有。

```sh
python3 -B tests/run.py --output /private/new-host-tests
python3 -B tools/build.py --module /private/eg1990_053200.bi --output /private/new-build --version 5.32
```

也可选择`--version 5.60`，仍严格核对冻结产物身份。原菜单数据、原指令及模板仅从用户本地精确模块抽取，仓库不含原厂资源正文。完整容器封包另需本地原包与私有格式材料，详见构建文档；不下载这些材料。

仓库不含原固件、安装BIN、NEF实拍、私有格式材料、原厂反编译正文或个人音频。公开包为新写源码、配方、合成测试与文档，另有上述明确披露权属与授权不确定性的UI截图。教育用途或署名声明不自动构成转载授权。

## 参考

源配方方法参考 [NikonFirmwarePatch](https://github.com/XinmiaoZhao/NikonFirmwarePatch/tree/1a60fc00ff082b6ccc346db0df626662308b3974)；QEMU/HE研究参考 [RdWing/z8-qemu](https://github.com/RdWing/z8-qemu/tree/9881d71053dacadbb81d2b352533ccf6a52cd604)。这里只列历史研究固定提交，未声称是上游最新版本或把其Z8地址直接用于Z9。

音频研究与独立发布见 [mark2 v14](https://github.com/s3lemming/z9-firmware-patch/tree/241b8031aec14761610128036fd4e8d88583a419)。本仓库使用集成运行时堆/锚点布局，不能混用独立mark2固定BSS或产物身份。未复制上游RAW补丁实现、CEVA微码或厂商私有材料。新写代码GPL-3.0-or-later，见LICENSE/NOTICE。
