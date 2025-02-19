/**
 * @file app.c
 * @brief Application
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "slog.h"
#include "cmsis_os.h"

void startup_task(void* argument) {
    slog_transmit(0, " __  __         _       _             _   ___  ___ ___ _______   __");
    slog_transmit(0, "|  \\/  |__ _ __| |___  | |__ _  _    /_\\ / _ \\/ __|_ _|_   _\\ \\ / /");
    slog_transmit(0, "| |\\/| / _` / _` / -_) | '_ \\ || |  / _ \\ (_) \\__ \\| |  | |  \\ V / ");
    slog_transmit(0, "|_|  |_\\__,_\\__,_\\___| |_.__/\\_, | /_/ \\_\\___/|___/___| |_|   |_|  ");
    slog_transmit(0, "                             |__/                                  ");

    osThreadExit();
}
