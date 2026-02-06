FROM ubuntu:20.04

# 设置非交互式安装以避免提示
ENV DEBIAN_FRONTEND=noninteractive

# 更新并安装必要工具
RUN apt-get update && \
    apt-get install -y \
    g++ \
    make \
    curl \
    && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /app

# 复制所有文件
COPY . .

# 创建必要的文件（如果不存在）
RUN if [ ! -f userlist.txt ]; then \
        echo "# MOLI Game Company User Database" > userlist.txt; \
    fi && \
    if [ ! -f allow.txt ]; then \
        echo "1234009" > allow.txt; \
        echo "1384733" >> allow.txt; \
        echo "1802886" >> allow.txt; \
        echo "1581608" >> allow.txt; \
    fi

# 编译服务器（使用简单命令）
RUN g++ -o server server.cpp -lpthread -std=c++11 2>&1 | tee build.log && \
    if [ ! -f server ]; then echo "Compilation failed!" && cat build.log && exit 1; fi

# 显示成功信息
RUN echo "Build successful! Files in /app:" && ls -la

# 暴露端口
EXPOSE 8080

# 启动命令
CMD ["./server"]
