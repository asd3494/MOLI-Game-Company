FROM ubuntu:22.04

# 安装必要的依赖
RUN apt-get update && \
    apt-get install -y \
    g++ \
    make \
    git \
    curl \
    && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /app

# 复制所有文件
COPY . .

# 检查文件是否完整
RUN echo "=== Checking files ===" && \
    ls -la *.cpp *.h *.txt *.html *.js 2>/dev/null || echo "Some files may be missing"

# 编译服务器程序
RUN g++ -std=c++11 -o server server.cpp -lpthread

# 编译同步工具（如果需要）
RUN g++ -std=c++11 -o save save.cpp

# 创建必要的初始文件
RUN if [ ! -f userlist.txt ]; then \
        echo "# MOLI Game Company User Database" > userlist.txt && \
        echo "# Format: username:password_hash:timestamp" >> userlist.txt; \
    fi

RUN if [ ! -f allow.txt ]; then \
        echo "# Allowed UID List" > allow.txt && \
        echo "1234009" >> allow.txt && \
        echo "1384733" >> allow.txt && \
        echo "1802886" >> allow.txt && \
        echo "1581608" >> allow.txt; \
    fi

# 健康检查
HEALTHCHECK --interval=30s --timeout=3s --start-period=10s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# 暴露端口
EXPOSE 8080

# 启动服务器
CMD ["./server"]
