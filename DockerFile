# 使用官方GCC镜像作为构建环境（自带g++编译器）
FROM gcc:latest as builder

# 设置容器内的工作目录
WORKDIR /app

# 将宿主机的所有源代码复制到容器的/app目录下
COPY . .

# 编译C++程序（请确保此命令与你在本地使用的完全一致）
# 这一行是核心，会生成可执行文件 `server`
RUN g++ -std=c++11 server.cpp -o server -lpthread

# 第二阶段：使用一个更小的运行时镜像
FROM ubuntu:latest

# 安装运行时可能需要的库（httplib依赖的线程库通常是系统自带的，为保险可以更新）
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /app

# 从上一阶段的构建镜像中，只复制编译好的可执行文件
COPY --from=builder /app/server .

# 声明容器运行时监听的端口（Render会注入PORT环境变量）
# 这个数字（如8080）仅作声明，实际端口由Render决定
EXPOSE 8080

# 设置启动命令
CMD ["./server"]
