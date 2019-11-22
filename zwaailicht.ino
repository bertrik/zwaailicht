#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cmdproc.h"
#include "editline.h"

#include <Arduino.h>

#define printf Serial.printf
#define PIN_TURN D1

static char line[120];

static unsigned long turn_start;
static unsigned long turn_time;

static void show_help(const cmd_t * cmds)
{
    for (const cmd_t * cmd = cmds; cmd->cmd != NULL; cmd++) {
        printf("%10s: %s\n", cmd->name, cmd->help);
    }
}

static int do_help(int argc, char *argv[]);


static void run_turn(unsigned long ms)
{
    if (turn_time > 0) {
        digitalWrite(PIN_TURN, 1);

        if ((ms - turn_start) > turn_time) {
            digitalWrite(PIN_TURN, 0);
            turn_time = 0;
        }
    }
}

static int do_turn(int argc, char *argv[])
{
    if (argc < 2) {
        return -1;
    }

    turn_time = atoi(argv[1]);
    turn_start = millis();

    return 0;
}

const cmd_t commands[] = {
    { "help", do_help, "Show help" },
    { "turn", do_turn, "<on/off> Turn on or off" },
    { NULL, NULL, NULL }
};

static int do_help(int argc, char *argv[])
{
    show_help(commands);
    return 0;
}

void setup(void)
{
    Serial.begin(115200);
    printf("\nESP-ZWAAILICHT\n");
    EditInit(line, sizeof(line));

    pinMode(D1, OUTPUT);
}

void loop(void)
{
    // parse command line
    bool haveLine = false;
    if (Serial.available()) {
        char c;
        haveLine = EditLine(Serial.read(), &c);
        Serial.write(c);
    }
    if (haveLine) {
        int result = cmd_process(commands, line);
        switch (result) {
        case CMD_OK:
            printf("OK\n");
            break;
        case CMD_NO_CMD:
            break;
        case CMD_UNKNOWN:
            printf("Unknown command, available commands:\n");
            show_help(commands);
            break;
        default:
            printf("%d\n", result);
            break;
        }
        printf(">");
    }
    // run the zwaailicht process
    run_turn(millis());
}


