# HttpServer


## 快速开始

1. 确保安装好MySQL，并在MySQL中执行以下操作：

``` bash
# 创建yourdb数据库并使用（数据库名可以自行确定）
create database yourDb;
use yourDb;

# 创建user表（保存用户注册信息）
create table user(
    username char(50) NULL,
    passwd char(50) NULL
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
# 完成以下命令后，会在项目同一级目录生成一个build文件夹
# build文件夹下存放着最终生成的可执行文件
./build.sh
cd ../build/release
./HttpServer
```

4. 测试

打开浏览器并输入localhost:port（参考第2步的设置）来向服务器发起请求。

## 压力测试

压力测试采用wenbench完成。

### 测试环境


### 测试结果






