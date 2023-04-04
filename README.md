# LeptJSON Parser in C++ Language

[LeptJSON Library](https://github.com/miloyip/json-tutorial) in C++11.

使用 C++ 编写一个简单的 JSON 库：
- 测试驱动开发；
- 以 LeptValue 类管理 JSON 解析数据；
- JSON 数据（字符串、数组、对象）以 union 联合体形式共享存储空间，union 内是指向字符串、数组、对象的指针；
- 合理编写 LeptValue 类的构造函数、析构函数、拷贝构造函数、拷贝赋值运算符，管理内存；
- 通过 C++ vector 容器管理 JSON 的数组和对象，避免内存泄漏。
