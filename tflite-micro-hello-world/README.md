# TFLite Micro Hello World no ESP32-S3 (Wokwi)

Projeto do desafio de embarcados: reproduzir o Hello World do TensorFlow Lite Micro, rodar no Wokwi e analisar o código e a documentação. O modelo aprende a função seno, é quantizado pra int8 e roda no ESP32-S3 com o ESP-IDF. A análise está no [RELATORIO.md](RELATORIO.md).

![Wokwi rodando o Hello World no VS Code](docs/wokwi-hello-world.png)

Esse print é da extensão do Wokwi no VS Code, com a placa ESP32-S3 e a saída serial do modelo. O segundo print, em `docs/wokwi-hello-world-web.png`, é do simulador web (wokwi.com) e foi o que usei aqui pra conferir. Os valores dos dois são iguais, e a origem de cada um está no relatório (seção 6).

## 1. O que tem aqui

- `treinamento/hello_world_training.ipynb`: notebook que treina a rede, converte pra TFLite float e int8 e avalia o erro contra `sin(x)`.
- `treinamento/hello_world_int8.tflite`: o modelo int8 que sai do notebook (5160 bytes).
- `treinamento/gerar_model_cc.sh`: transforma o `.tflite` em `main/model.cc` com `xxd -i`.
- `main/`: o firmware, que é o exemplo `hello_world` do `esp-tflite-micro`, com só o `model.cc` trocado.
- `wokwi.toml` e `diagram.json`: configuração do Wokwi, só a placa ESP32-S3 com o monitor serial.
- `sdkconfig.defaults`: configuração do exemplo, mais uma linha pro Wokwi (seção 5).

## 2. Treinar o modelo (opcional)

O `hello_world_int8.tflite` já está no repositório, então dá pra pular esta parte. Pra refazer, abra o notebook no Colab ou no Jupyter e execute tudo. Ele grava os modelos em `treinamento/hello_world_models/` (ignorada pelo git). Troquei o caminho `/content/hello_world_models` por `hello_world_models` pra rodar fora do Colab.

O notebook usa TensorFlow 2.x. Rodei com TensorFlow 2.21 e NumPy 2.4. Com NumPy 2 a função `dequantize_output` original estourava o int8, então mudei uma linha pra `(int(output_value) - zero_point) * scale`. O relatório mostra o efeito.

O treino não tem semente, então o erro muda um pouco a cada execução. Na minha execução ficou assim, em 1000 pontos contra `sin(x)`:

| Modelo | MAE | RMSE |
| --- | --- | --- |
| Float | 0,0099 | 0,0139 |
| Int8 | 0,0116 | 0,0165 |

Se treinar de novo, copie o novo `hello_world_int8.tflite` pra `treinamento/` e gere o `model.cc` (seção 3).

## 3. Gerar o model.cc

Precisa do `xxd` (pacote `xxd` no Linux, vem junto com o Vim no macOS).

```sh
sh treinamento/gerar_model_cc.sh
```

O `main/model.cc` que está no repositório já foi gerado a partir do modelo atual.

## 4. Compilar e simular

Abra a pasta `tflite-micro-hello-world` no VS Code (precisa ser ela, não a raiz do repositório), com as extensões ESP-IDF e Wokwi instaladas. Depois:

1. `ESP-IDF: Set Espressif Device Target` e escolha `esp32s3`.
2. `ESP-IDF: Build Project`. Na primeira vez ele baixa o componente `espressif/esp-tflite-micro` do registry, então precisa de internet.
3. `Wokwi: Start Simulator`.

O ESP-IDF precisa ser a versão 5.1 ou mais nova, porque o componente atual exige isso. Compilei com a 5.5.5 e com a 6.1.

A saída esperada no terminal serial é uma linha por ponto, de 0 a 2π:

```
x_value: 0.000000, y_value: 0.015697
x_value: 0.314159, y_value: 0.298252
x_value: 0.628319, y_value: 0.596504
x_value: 0.942478, y_value: 0.800571
x_value: 1.256637, y_value: 0.941848
x_value: 1.570796, y_value: 0.988940
```

Os valores de `y_value` mudam se você treinar de novo, mas devem seguir o seno.

## 5. Por que CONFIG_NN_ANSI_C=y

Com a configuração padrão do exemplo, o S3 usa os kernels do `esp-nn` em assembly. No Wokwi, o exemplo original rodava, mas o `y_value` ficava travado em `-1.118309`, qualquer que fosse o `x`. Esse valor é a menor saída quantizada (-128) desquantizada.

Com `CONFIG_NN_ANSI_C=y` (kernels em C) a saída volta a seguir o seno. Compilando pro ESP32 comum com os kernels padrão também funcionou. Isso sugere que o problema está no assembly do S3 dentro do simulador, mas não testei em placa real, então não sei se é limitação do Wokwi. Se for rodar em hardware, pode apagar essa linha do `sdkconfig.defaults` pra usar os kernels otimizados.

## 6. Origem e licença

O código de `main/` vem do exemplo `hello_world` do [espressif/esp-tflite-micro](https://github.com/espressif/esp-tflite-micro), obtido com `idf.py create-project-from-example "espressif/esp-tflite-micro:hello_world"`, sob licença Apache 2.0 (os cabeçalhos dos arquivos estão mantidos). O que mudou foi o `model.cc` (agora gerado do modelo treinado aqui), o `sdkconfig.defaults` (uma linha a mais), o `wokwi.toml` e o `diagram.json`.
