CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic

PKG_CONFIG = pkg-config
SDL_PACKAGES = sdl3 sdl3-image sdl3-ttf

.PHONY: all clean

all: output/sdl3_window

output/sdl3_window: src/sdl3_window.cpp src/farm.hpp
	mkdir -p output
	$(CXX) $(CXXFLAGS) src/sdl3_window.cpp -o output/sdl3_window $(shell $(PKG_CONFIG) --cflags --libs $(SDL_PACKAGES))

clean:
	rm -f output/sdl3_window