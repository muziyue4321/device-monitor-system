CC = gcc
CFLAGS = -Wall -Wextra -I. -lsqlite3 -lpthread
TARGETS = client server query

all: $(TARGETS)

# 客户端编译
client: client.c device_client.c network.c utils.c
	$(CC) $^ -o $@ $(CFLAGS)

# 服务端编译
server: server.c database_server.c utils.c
	$(CC) $^ -o $@ $(CFLAGS)

# 查询端编译
query: query.c utils.c
	$(CC) $^ -o $@ $(CFLAGS)

# 清理编译产物
clean:
	rm -f $(TARGETS) device.db