# Makefile para compilar Nex para WebAssembly usando Emscripten (emcc)

EMCC = emcc
CFLAGS = -O3 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=67108864
EXPORTED_FUNCTIONS = -s EXPORTED_FUNCTIONS="['_Inicializar','_ExecutarLance','_Parametros','_SetParametro','_GetParametro','_Executar','_malloc','_free']"
EXPORTED_RUNTIME_METHODS = -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap', 'UTF8ToString', 'getValue']"

TARGET = nex.js
SOURCES = nex_em.cpp

.PHONY: all clean

all: $(TARGET) 

$(TARGET): $(SOURCES)
	$(EMCC) $(CFLAGS) $(EXPORTED_FUNCTIONS) $(EXPORTED_RUNTIME_METHODS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET) nex.wasm
