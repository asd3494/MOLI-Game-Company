FROM ubuntu:22.04

# 设置工作目录（非常关键）
WORKDIR /app

# 安装编译环境
RUN apt-get update && \
    apt-get install -y g++ && \
    rm -rf /var/lib/apt/lists/*

# 拷贝源代码和运行时所需文件
COPY server.cpp .
COPY httplib.h .
COPY index.html .
COPY login_page.html .
COPY signup_page.html .
COPY allow.txt .
COPY userlist.txt .

# 编译 C++ 程序
RUN g++ server.cpp -O2 -std=c++17 -o server

# 暴露端口（给 Koyeb 用）
EXPOSE 8080

# 启动服务
CMD ["./server"]
