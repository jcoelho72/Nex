@echo off

REM ----------------------------------------------------------------
REM Compila o código com otimização e geração de WebAssembly
REM A flag -O3 ativa otimizações, -O0 para incluir informação de debug
REM A opção -s WASM=1 gera o .wasm; o EXPORTED_FUNCTIONS indica quais funções exportar
REM EXTRA_EXPORTED_RUNTIME_METHODS inclui métodos úteis, como cwrap e ccall.
REM Certifique-se de usar as aspas corretas para o seu shell.
REM ----------------------------------------------------------------

emcc -O3 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 ^
-s EXPORTED_FUNCTIONS="['_Inicializar','_ExecutarLance','_Parametros','_SetParametro','_GetParametro','_Executar','_malloc','_free']" ^
-s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap', 'UTF8ToString', 'getValue']" ^
-o nex.js nex_em.cpp


pause
