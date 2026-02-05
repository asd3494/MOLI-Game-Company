FROM ubuntu:22.04

# 更新并安装必要的依赖
RUN apt-get update && \
    apt-get install -y \
    g++ \
    make \
    git \
    curl \
    && rm -rf /var/lib/apt/lists/*

# 创建工作目录
WORKDIR /app

# 复制所有文件
COPY . .

# 编译服务器程序
RUN g++ -std=c++11 -o server server.cpp -lpthread

# 创建必要的文件
RUN touch userlist.txt allow.txt

# 健康检查
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# 暴露端口
EXPOSE 8080

# 启动命令
CMD ["./server"]
