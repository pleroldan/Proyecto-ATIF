// lcdplus.h
#ifndef LCDPLUS_H
#define LCDPLUS_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

void initLCD();
void actualizarLCD();
LiquidCrystal_I2C& getLCD();

#endif
