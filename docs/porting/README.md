# f2/Fn1 menu and recall porting notes

These notes describe **Z9 original 5.32 interfaces and the optional f2-recall source variant**. Development labels 5.60/5.61 do not supply address maps for official firmware of those versions. No Z8 port or hardware acceptance is implied.

1. [菜单研究与发现过程](menu-research.zh-CN.md): table shapes, event contracts and corrected assumptions.
2. [调用内存与ABI](recall-memory-abi.zh-CN.md): boot anchor, dynamic heap, compiler-derived offsets, DSIDs and native interfaces.
3. [移植步骤与验收](recall-porting-guide.zh-CN.md): version/ISA/MMU, native consumers, persistence and ownership.
4. [Machine-readable reference](f2-reference.json): numeric namespaces, source hashes and AArch64 layout.

Reproduce the authored-structure layout without any firmware:

```sh
python3 -B tools/f2_porting_layout.py --output /private/new-layout
```

Requires AArch64 GCC/binutils. The output must be outside this repository. It compiles source and extracts a readonly data section; it does not execute camera firmware. Treat all addresses below as version-bound reference data, never as an address search/replace list for Z8.
