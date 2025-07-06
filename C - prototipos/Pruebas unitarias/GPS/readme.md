# Prueba Unitaria de GPS

Este archivo describe la prueba unitaria implementada para el módulo GPS del proyecto ATIF.

## Objetivo

Verificar el correcto funcionamiento de las funciones relacionadas con la adquisición y procesamiento de datos GPS.

## Descripción

La prueba unitaria simula la recepción de datos GPS y valida que el módulo:

- Reciba correctamente las cadenas NMEA.
- Procese y extraiga información relevante (latitud, longitud, hora, etc.).
- Detecte y maneje errores en los datos recibidos.

## Funcionamiento

1. **Inicialización:**  
    Se configura el entorno de prueba y se inicializa el módulo GPS.

2. **Simulación de datos:**  
    Se envían cadenas NMEA válidas y no válidas al módulo.

3. **Validación:**  
    Se comprueba que los datos extraídos coincidan con los valores esperados y que los errores sean detectados correctamente.

4. **Resultados:**  
    La prueba reporta el éxito o fallo de cada caso evaluado.

## Ejecución

Para ejecutar la prueba, sigue las instrucciones específicas del entorno de pruebas del proyecto (por ejemplo, usando `make test` o ejecutando el archivo de prueba correspondiente).

## Notas

- Asegúrate de tener todas las dependencias instaladas.
- Consulta el código fuente para detalles sobre los casos de prueba implementados.
- Puedes modificar o ampliar las pruebas según los requisitos del proyecto.
