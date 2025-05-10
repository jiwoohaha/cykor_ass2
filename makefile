CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = cykor_week_2

all: $(TARGET)

$(TARGET): cykor_week_2.c
	$(CC) $(CFLAGS) -o $(TARGET) cykor_week_2.c

clean:
	rm -f $(TARGET)