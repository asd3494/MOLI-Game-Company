FROM ubuntu:22.04

# 安装最小化依赖
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    g++ \
    make \
    curl \
    && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /app

# 首先复制最不可能变化的文件
COPY httplib.h .

# 复制HTML文件
COPY index.html .
COPY login_page.html .
COPY signup_page.html .

# 复制JavaScript文件
COPY triangle-effect.js .

# 复制配置文件
COPY allow.txt .

# 复制用户数据文件（如果存在）
COPY userlist.txt . 2>/dev/null || echo "userlist.txt not found, will create on startup"

# 最后复制C++源文件
COPY server.cpp .
COPY save.cpp .

# 创建空的userlist.txt（如果不存在）
RUN touch userlist.txt

# 简单的编译命令
RUN g++ -std=c++11 -o server server.cpp -lpthread

# 检查编译是否成功
RUN if [ ! -f server ]; then echo "Compilation failed!" && exit 1; fi

# 显示文件信息
RUN echo "=== Build completed ===" && \
    echo "Files in /app:" && \
    ls -la && \
    echo "=== Server binary ===" && \
    file server && \
    echo "=== Userlist preview ===" && \
    head -5 userlist.txt

# 暴露端口
EXPOSE 8080

# 简单的启动命令
CMD ["./server"]
