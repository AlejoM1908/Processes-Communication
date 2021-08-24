# Taller de comunicación entre Procesos

Este repositorio hace parte de la solución al taller #1 para el primer parcial del curso de Sistemas Operativos de la Universidad Nacional de Colombia en el primer semestre de 2021

## Funcionamiento del programa

El proposito principal del programa es poder medir el método de comunicación más eficiente entre procesos, generando un promedio de transferencia de diferentes tamaños de paquetes en una simulación del problema del productor y consumidor, donde el productor construye el paquete de datos a transferir, hace el envio al consumidor y mide el tiempo que transcurre.

Los métodos de comunicación a medir son:
- Uso de Archivos
- Memoria Compartida
- Tuberias
- Paso de Mensajes

Los tamaños de paquetes a medir seran de:
- 1 Kb
- 10 Kb
- 100 Kb
- 1 Mb
- 10 Mb
- 100 Mb

## Restricciones de la practica

La solución debe ser capaz de:

- Hallar el tiempo medio de transferencia de cada metodo en base a un tamaño de paquete de datos determinados
- Usar un Bash Script que automatice la totalidad de los experimentos y despliegue los resultados
- Usar el problema de productor consumidor

## Objetivos de la practica

 - Afianzar el uso de estructuras y memoria dinamica en C, además de estructuras de datos como linked list y hash tables
 - Demostrar conocimeintos en el manejo de procesos vistos en clase
 - Generar una solución modular, rapida, eficiente y confiable que haga uso de buenas practicas de programación
 - Aprender la utilidad de los Bash Script para automatizar y aumentar la eficiencia en ejecución

 ## Explicación de la solución desarrollada

Se construyeron cuatro programas por separado que implementaban cada uno una solución de comunicación entre procesos, cada programa generaba dos procesos emparentados que creaban sucesivamente cada paquete de datos requerido para la prueba y lo enviaba usando su método especifico. Este proceso de envío se realiza un total de cinco veces y se promedian los resultados obtenidos para mayor precisión.

Finalmente un Bash Script se encarga de unir y automatizar todo en un programa único que despliega los tiempos promedio que le toma a cada programa hacer los envíos correspondientes de sus paquetes para realizar el análisis del método más eficiente de comunicación entre procesos.

## Participantes del desarrollo

Gabriel Santana Paredes (gsantana@unal.edu.co)

Diego Esteban Morales (dimorales@unal.edu.co)

Daniel Alejandro Melo (dmelo@unal.edu.co)
