CC=emcc
CXX=emcc

OUTPUT_FOLDER=./dist
OUTPUT=$(OUTPUT_FOLDER)/gearboy.html
DISTDIR=dist
GEARBOY_SRC=../../src
GEARBOY_AUDIO_SRC=../audio-shared
GEARBOY_WASM_SRC= .

OBJS=$(GEARBOY_WASM_SRC)/main.o $(GEARBOY_SRC)/MBC2MemoryRule.o $(GEARBOY_SRC)/Audio.o $(GEARBOY_SRC)/MBC1MemoryRule.o $(GEARBOY_SRC)/IORegistersMemoryRule.o $(GEARBOY_SRC)/audio/Gb_Apu.o $(GEARBOY_SRC)/MultiMBC1MemoryRule.o $(GEARBOY_SRC)/GearboyCore.o $(GEARBOY_SRC)/audio/Multi_Buffer.o $(GEARBOY_SRC)/audio/Effects_Buffer.o $(GEARBOY_SRC)/MBC5MemoryRule.o $(GEARBOY_SRC)/audio/Gb_Apu_State.o $(GEARBOY_SRC)/audio/Blip_Buffer.o $(GEARBOY_SRC)/MemoryRule.o $(GEARBOY_SRC)/Input.o $(GEARBOY_SRC)/Processor.o $(GEARBOY_SRC)/Video.o $(GEARBOY_SRC)/Memory.o $(GEARBOY_SRC)/Cartridge.o $(GEARBOY_SRC)/MBC3MemoryRule.o $(GEARBOY_SRC)/RomOnlyMemoryRule.o $(GEARBOY_SRC)/CommonMemoryRule.o $(GEARBOY_AUDIO_SRC)/Sound_Queue.o $(GEARBOY_SRC)/audio/Gb_Oscs.o $(GEARBOY_SRC)/opcodes.o $(GEARBOY_SRC)/opcodes_cb.o
SRCS=$(GEARBOY_WASM_SRC)/main.cpp $(GEARBOY_SRC)/MBC2MemoryRule.cpp $(GEARBOY_SRC)/Audio.cpp $(GEARBOY_SRC)/MBC1MemoryRule.cpp $(GEARBOY_SRC)/IORegistersMemoryRule.cpp $(GEARBOY_SRC)/audio/Gb_Apu.cpp $(GEARBOY_SRC)/MultiMBC1MemoryRule.cpp $(GEARBOY_SRC)/GearboyCore.cpp $(GEARBOY_SRC)/audio/Multi_Buffer.cpp $(GEARBOY_SRC)/audio/Effects_Buffer.cpp $(GEARBOY_SRC)/MBC5MemoryRule.cpp $(GEARBOY_SRC)/audio/Gb_Apu_State.cpp $(GEARBOY_SRC)/audio/Blip_Buffer.cpp $(GEARBOY_SRC)/MemoryRule.cpp $(GEARBOY_SRC)/Input.cpp $(GEARBOY_SRC)/Processor.cpp $(GEARBOY_SRC)/Video.cpp $(GEARBOY_SRC)/Memory.cpp $(GEARBOY_SRC)/Cartridge.cpp $(GEARBOY_SRC)/MBC3MemoryRule.cpp $(GEARBOY_SRC)/RomOnlyMemoryRule.cpp $(GEARBOY_SRC)/CommonMemoryRule.cpp Wasm_Sound_Queue.cpp $(GEARBOY_SRC)/audio/Gb_Oscs.cpp $(GEARBOY_SRC)/opcodes.cpp $(GEARBOY_SRC)/opcodes_cb.cpp
BIN=gearboy

CFLAGS+=-Wall

WASM_WEBSITE_LD_FLAGS+= -s MODULARIZE=1 -s EXPORT_ES6=1 
WASM_LDFLAGS+=-s USE_SDL=2 -s FORCE_FILESYSTEM=1 -s WASM=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]'

.SECONDARY: $(OBJS)

test:
	$(CC) -O3 $(SRCS) $(WASM_LDFLAGS) $(CFLAGS) -o gearboy.html


all: $(BIN) $(LIB)

%.o: %.cpp
	@rm -f $@
	$(CXX) $(WASM_LDFLAGS) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJS)
	$(CC) $(WASM_LDFLAGS) -o index.js $(OBJS) --source-map-base http://localhost:8000/

emscripten: $(OBJS)
	$(CC) $(WASM_LDFLAGS) -o test.html $@ $(OBJS)

clean:
	for i in $(OBJS); do (if test -e "$$i"; then ( rm $$i ); fi ); done
	@rm -f $(BIN) $(LIB)
	rm index.wasm
	rm index.js
