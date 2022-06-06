# Debug 说明

Catalog

1. existTable未正确实现。已重写。
2. updateTable会直接扔掉之前Index的信息。已重写。
3. 优化了所有查询tableName的速度：O(N) -> O(logN)
4. 只有在构造时才会解析AllTableNames文件，只有在析构时才会写AllTableNames文件
4. 使用str = str.substr(...) 做赋值是危险的。具体原因待深究。已修改。(erase)



Record

1. 将Record中所有Catalog和IndexManager换成了静态的。必须这样，否则不符合目前Catalog的逻辑
2. 一些让代码更tidy的Trick。



Exec_engine

1. 调换了许多不合理的顺序。应当总是Catalog先动，如删除文件时，因为Catalog能知道表的存在性等信息



Buffer

1. 加入了一个存储文件名和page_id的map，加速getPage的查询。
2. flushPage接口修改。增加安全性。（去掉了两个参数，保证内存和磁盘中文件的对应关系）
2. BUG：当文件已被删除，还是会flush到文件中。通过修改map和initialize page，把那个页清空。