# HttpServer


## 快速开始

1. 确保安装好MySQL并启动MySQL服务，并在MySQL中执行以下操作：

``` bash
# 创建yourdb数据库并使用（数据库名可以自行确定）
create database yourDb;
use yourDb;

# 创建user表（保存用户注册信息）
create table user(
    username char(50) NULL,  # 用户名
    passwd char(50) NULL     # 密码
)engine=InnoDB;
```

2. 修改main.cpp中的数据库初始化信息

```
// 原始
HttpServer server(13333, 8, "root", "123456", "user", 8);

// 以下参数，修改成你期望的设置
HttpServer server(yourPort, threadNum, yourDbUserName, yourDbPasswd, yourDbName, sqlConnNum);
```

3. 编译构建并运行
```bash
./build.sh
./HttpServer
```

4. 测试

打开浏览器并输入localhost:port（参考第2步的设置）来向服务器发起请求。

## 压力测试

压力测试采用wenbench完成。

### 测试环境

- ubuntu
- 8核，8G内存


### 测试结果






