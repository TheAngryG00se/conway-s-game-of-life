CC = gcc
CFLAGS = -O2 -Wall -Wno-unknown-pragmas $(shell sdl2-config --cflags) $(if $(RANDOM),-DRANDOM)
LDLIBS = $(shell sdl2-config --libs)

BUILD_DIR = build

all: $(BUILD_DIR)/life_omp $(BUILD_DIR)/life_seq

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/life_omp: life.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -fopenmp $< -o $@ $(LDLIBS)

$(BUILD_DIR)/life_seq: life.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean