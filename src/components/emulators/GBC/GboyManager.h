#ifndef GBOY_MANAGER_H
#define GBOY_MANAGER_H

#include <cstdint>


void gnuboy_start();
void gnuboy_resume();
void gnuboy_suspend();
void gnuboy_save();
bool gnuboy_execute_game(const char *name, uint8_t console, bool load);

#endif // GBOY_MANAGER_H