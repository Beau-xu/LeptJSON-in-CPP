# LeptJSON Parser in C++ Language

[LeptJSON Library](https://github.com/miloyip/json-tutorial) in C++11.

测试驱动开发，使用 C++ 编写一个简单的 JSON 库：
- 实现字符串解析为 JSON 格式，或 JSON 的字符串化（stringify）；
- 以 LeptValue 类管理 JSON 数据；
- JSON 数字、字符串、数组、对象以 union 联合体形式共享存储空间；
- 编写 LeptValue 类的析构函数、拷贝控制函数、成员函数，为 union 合理分配内存，避免内存泄漏；
- 通过 C++ vector 容器管理 JSON 的数组和对象，实现内存自动管理。

优化：
- 将 union 指针成员（指向类对象的指针`string*`、`vector<T>*`）替换为类对象本身。原代码保存在[指针分支](https://github.com/Beau-xu/LeptJSON-in-CPP/tree/class-pointer-in-union)
