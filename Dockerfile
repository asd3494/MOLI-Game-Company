FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# 复制所有资源（html / txt / cpp）
COPY . .

# 编译（cpp-httplib 必须 pthread）
RUN g++ server.cpp -O2 -std=c++17 -pthread -o server

EXPOSE 8080

CMD ["./server"]
