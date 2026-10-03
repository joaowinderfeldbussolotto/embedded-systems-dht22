#!/bin/sh
# Gera main/model.cc a partir do modelo int8 treinado no notebook.
set -e
cd "$(dirname "$0")"
{
  echo '// Gerado por treinamento/gerar_model_cc.sh a partir de treinamento/hello_world_int8.tflite'
  echo '// (xxd -i, como no exemplo original do esp-tflite-micro).'
  echo
  echo '#include "model.h"'
  echo
  echo '// Keep model aligned to 8 bytes to guarantee aligned 64-bit accesses.'
  echo 'alignas(8) const unsigned char g_model[] = {'
  xxd -i < hello_world_int8.tflite
  echo '};'
  echo "const int g_model_len = $(wc -c < hello_world_int8.tflite);"
} > ../main/model.cc
