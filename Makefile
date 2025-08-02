CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
LIBS = -lcurl -lcjson

TARGET = loquatcli
SOURCE = loquatcli.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LIBS)

clean:
	rm -f $(TARGET)

install-deps:
	# For Ubuntu/Debian
	sudo apt-get update && sudo apt-get install -y libcurl4-openssl-dev libcjson-dev
	# For CentOS/RHEL/Fedora
	# sudo yum install libcurl-devel libcjson-devel
	# For macOS
	# brew install curl cjson

run: $(TARGET)
	./$(TARGET)

help:
	@echo "Available targets:"
	@echo "  all        - Build the client (default)"
	@echo "  clean      - Remove built files"
	@echo "  install-deps - Install libcurl development package"
	@echo "  run        - Build and run the client"
	@echo "  help       - Show this help message" 