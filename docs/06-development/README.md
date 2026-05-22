# 开发规范

本目录包含 EMS 项目的开发规范和最佳实践。

## 文档列表

- [CODING_STANDARDS.md](CODING_STANDARDS.md) - 编码规范

## 开发规范概述

EMS 项目遵循严格的编码规范，确保代码质量和可维护性。

### 核心原则

1. **代码风格** - 遵循 Linux 内核编码风格
2. **命名规范** - 清晰的命名约定
3. **文档要求** - 必要的注释和文档
4. **测试要求** - 单元测试和集成测试
5. **安全编码** - 避免常见安全漏洞

### 快速检查

```bash
# 代码风格检查
scripts/checkpatch.pl --file your_file.c

# 静态分析
make cppcheck

# 运行测试
make test
```

## 开发流程

1. **创建分支**
   ```bash
   git checkout -b feature/your-feature
   ```

2. **编写代码**
   - 遵循编码规范
   - 添加必要注释
   - 编写单元测试

3. **本地验证**
   ```bash
   make clean
   make x86_64_full_defconfig
   make -j$(nproc)
   make test
   ```

4. **提交代码**
   ```bash
   git add .
   git commit -m "模块：简短描述"
   ```

5. **创建 PR**
   - 清晰的 PR 描述
   - 关联相关 issue
   - 等待 code review

## 代码审查要点

- [ ] 符合编码规范
- [ ] 有适当的错误处理
- [ ] 有必要的注释
- [ ] 通过所有测试
- [ ] 无编译告警
- [ ] 无内存泄漏

## 相关文档

- [架构设计](../03-architecture/) - 了解系统架构
- [构建系统](../02-build-system/) - 构建和测试
