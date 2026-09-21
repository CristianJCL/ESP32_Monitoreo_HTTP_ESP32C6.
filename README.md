# Monitoreo de variables en la nube con ESP32-C6

Proyecto ESP-IDF que se conecta a Wi-Fi y publica dos variables en ThingSpeak
mediante HTTPS POST:

- `field1`: intensidad de señal Wi-Fi en dBm (RSSI).
- `field2`: tiempo de actividad del microcontrolador en segundos.

## Requisitos

- ESP32-C6.
- ESP-IDF 5.x o posterior.
- Canal de ThingSpeak con dos campos habilitados.
- Write API Key del canal.

## Configuración

Desde una terminal de ESP-IDF:

```text
idf.py set-target esp32c6
idf.py menuconfig
```

En `Example Connection Configuration`, capture el SSID y la contraseña Wi-Fi.
En `Configuracion del monitoreo HTTP`, capture la Write API Key y ajuste el
intervalo si se requiere. La clave queda en `sdkconfig`, archivo excluido del
repositorio.

## Compilación y ejecución

```text
idf.py build
idf.py -p COMx flash monitor
```

La consola muestra la IP obtenida y, después de cada envío válido, el RSSI, el
tiempo de actividad y el identificador de la entrada creado por ThingSpeak.

## Pruebas portables

La construcción del payload y la validación de la respuesta pueden verificarse
sin hardware:

```text
cd host_tests
sh run_tests.sh
```

## Seguridad

No incluya credenciales en el repositorio. `sdkconfig`, compilaciones y archivos
locales del entorno están excluidos mediante `.gitignore`.
