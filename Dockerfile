FROM ubuntu:22.04

RUN apt-get update && apt-get install -y g++ ca-certificates

WORKDIR /app
COPY . .

RUN g++ server.cpp -O2 -std=c++17 -pthread -o server

EXPOSE 8080

CMD ["./server"]
