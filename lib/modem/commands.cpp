#include "commands.h"

Command::Command(const char* cmd): command(cmd) {}

bool Command::parse(char* buffer, size_t len) {
    char * pch = strstr(buffer, command);
    if (pch == NULL) {
        return false;
    } else {
        size_t command_len = strlen(command);
        return buffer[command_len] == '\n';
    }
}

ExecutionResult Command::execute(char* buffer) {
    return NOT_EXECUTED;
}

void Command::ok(char* buffer) {
    sprintf(buffer, "OK\n");
}

void Command::error(const char* name, const char* valid, char* buffer) {
    sprintf(buffer, "Invalid %s. Valid values: %s\n", name, valid);
}

void Command::reset() { }

template <typename T>
AssignableCommand<T>::AssignableCommand(const char* cmd): Command(cmd) {}

template <typename T> 
AssignableCommand<T>::~AssignableCommand() {
    reset();
}

template <typename T>
bool AssignableCommand<T>::parse(char* buffer, size_t len) {
    char * pch = strstr(buffer, command);
    if (pch == NULL) {
        return false;
    } else {
        size_t command_len = strlen(command);
        char terminal_char = buffer[command_len];
        switch (terminal_char)
        {
            case '=': {
                char* new_line = strchr(buffer, '\n');
                if (new_line == NULL) {
                    return false;
                } else {
                    is_assign = true;
                    size_t arg_length = new_line - buffer - command_len;
                    char* arg_cstr = new char[arg_length];
                    strncpy(arg_cstr, buffer + command_len + 1, arg_length - 1);
                    arg_cstr[arg_length - 1] = '\0';
                    parseArgument(arg_cstr);
                    delete arg_cstr;
                    return true;
                }
            }
            case '\n':
                return true;
            default: 
                return false;
        }
    }
}

template <> void AssignableCommand<int>::parseArgument(char* arg) {
    argument_size = 1;
    argument = atoi(arg);
}

template <> void AssignableCommand<float>::parseArgument(char* arg) {
    argument_size = 1;
    argument = strtof(arg, NULL);
}

template <> void AssignableCommand<char**>::parseArgument(char* arg) {
    argument_size = 1;
    char* pch = strpbrk(arg, ",");
    while (pch != NULL)
    {
        argument_size++;
        pch = strpbrk(pch+1, ",");
    }
    argument = new char*[argument_size];
    for (int i = 0; i < argument_size; i++) {
        char *tok = strtok((i == 0)? arg : nullptr, ",");
        if (tok == nullptr) {
            break;
        }
        argument[i] = tok;
    }
}

template <typename T>
ExecutionResult AssignableCommand<T>::execute(char* buffer) {
    ExecutionResult result = NOT_EXECUTED;
    if (is_assign) {
        result = executeAssign(buffer);
    } else {
        result = executeQuery(buffer);
    }
    reset();
    return result;
}

template <> void AssignableCommand<float>::returnValue(float value, char* buffer) {
    sprintf(buffer, "%.1f\n", value);
};

template <> void AssignableCommand<int>::returnValue(int value, char* buffer) {
    sprintf(buffer, "%d\n", value);
};

template <typename T>
void AssignableCommand<T>::reset() {
    is_assign = false;
    argument_size = 0;
}

template <> void AssignableCommand<char**>::reset() {
    if (is_assign) {
        delete argument;
    }
    is_assign = false;
    argument_size = 0;
};

// AT
CommandAT::CommandAT(): Command("AT") {}

ExecutionResult CommandAT::execute(char* buffer) {
    ok(buffer);
    return EXECUTED;
}

// ATZ
CommandReset::CommandReset(ConfigPersister* persister): Command("ATZ"), persister(persister) {}

ExecutionResult CommandReset::execute(char* buffer) {
    persister->reset();
    persister->getConfig();
    ok(buffer);
    return EXECUTED_UPDATED;
}

// AT+RST
CommandRestart::CommandRestart(): Command("AT+RST") {}

ExecutionResult CommandRestart::execute(char* buffer) {
    ok(buffer);
    ESP.restart();
    return EXECUTED;
}

// AT+CONF
CommandConfig::CommandConfig(ModemConfig* config): Command("AT+CONF"), config(config) {}

ExecutionResult CommandConfig::execute(char* buffer) {
    sprintf(buffer, "%04X,%.1f,%.1f,%u,%u,%u,%u\n",
        config->address, config->radio.frequency, config->radio.bandwidth, 
        config->radio.sfactor, config->radio.coding_rate, config->radio.power, config->radio.gain);
    return EXECUTED;
}

// AT+ADDR
CommandAddr::CommandAddr(ModemConfig* config): Command("AT+ADDR"), config(config) {}

ExecutionResult CommandAddr::execute(char* buffer) {
    sprintf(buffer, "%04X", config->address);
    return EXECUTED;
}

// AT+FREQ
CommandFreq::CommandFreq(ModemConfig* config): AssignableCommand<float>("AT+FREQ"), config(config) {}

ExecutionResult CommandFreq::executeAssign(char* buffer) {
    if (argument >= 410.0 && argument <= 525.0) {
        config->radio.frequency = argument;
        ok(buffer);
        return EXECUTED_UPDATED;
    } else {
        error("frequency", "410.0-525.0", buffer);
        return EXECUTED;
    }
}

ExecutionResult CommandFreq::executeQuery(char* buffer) {
    AssignableCommand::returnValue(config->radio.frequency, buffer);
    return EXECUTED;
}

// AT+BW
CommandBandwidth::CommandBandwidth(ModemConfig* config): AssignableCommand<float>("AT+BW"), config(config) {}

ExecutionResult CommandBandwidth::executeAssign(char* buffer) {
    if (argument == 500.0 || argument == 250.0 || argument == 125.0 || argument == 62.5) {
        config->radio.bandwidth = argument;
        ok(buffer);
        return EXECUTED_UPDATED;
    } else {
        error("bandwidth", "62.5, 125.0, 250.0, 500.0", buffer);
        return EXECUTED;
    }
}

ExecutionResult CommandBandwidth::executeQuery(char* buffer) {
    AssignableCommand::returnValue(config->radio.bandwidth, buffer);
    return EXECUTED;
}

// AT+SF
CommandSfactor::CommandSfactor(ModemConfig* config): AssignableCommand<int>("AT+SF"), config(config) {}

ExecutionResult CommandSfactor::executeAssign(char* buffer) {
    if (argument >= 6 && argument <= 12) {
        config->radio.sfactor = argument;
        ok(buffer);
        return EXECUTED_UPDATED;
    } else {
        error("spread factor", "7, 8, 9, 10, 11, 12", buffer);
        return EXECUTED;
    }
}

ExecutionResult CommandSfactor::executeQuery(char* buffer) {
    AssignableCommand::returnValue(config->radio.sfactor, buffer);
    return EXECUTED;
}

// AT+RATE
CommandCodeRate::CommandCodeRate(ModemConfig* config): AssignableCommand<int>("AT+RATE"), config(config) {}

ExecutionResult CommandCodeRate::executeAssign(char* buffer) {
    if (argument >= 5 && argument <= 8) {
        config->radio.coding_rate = argument;
        ok(buffer);
        return EXECUTED_UPDATED;
    } else {
        error("code rate", "5, 6, 7, 8", buffer);
        return EXECUTED;
    }
}

ExecutionResult CommandCodeRate::executeQuery(char* buffer) {
    AssignableCommand::returnValue(config->radio.coding_rate, buffer);
    return EXECUTED;
}

// AT+POW
CommandPower::CommandPower(ModemConfig* config): AssignableCommand<int>("AT+POW"), config(config) {}

ExecutionResult CommandPower::executeAssign(char* buffer) {
    if (((argument >= 2) && (argument <= 17)) || (argument == 20)) {
        config->radio.power = argument;
        ok(buffer);
        return EXECUTED_UPDATED;
    } else {
        error("power", "2-17,20", buffer);
        return EXECUTED;
    }
}

ExecutionResult CommandPower::executeQuery(char* buffer) {
    AssignableCommand::returnValue(config->radio.power, buffer);
    return EXECUTED;
}

// AT+SCAN
CommandScan::CommandScan(ModemState* state): Command("AT+SCAN"), state(state) {}

ExecutionResult CommandScan::execute(char* buffer) {
    strcpy(buffer, "");
    std::vector<Node*> nodes = state->nodes.getNodes();
    if (nodes.size() > 0) {
        char b[32];
        unsigned long now = millis();
        for (auto it = nodes.begin(); it != nodes.end(); it++) {
            int32_t age = now - (*it)->time;
            sprintf(b, "%04X,%.1f,%d\n", (*it)->address, (*it)->rssi, age);
            strcat(buffer, b);
        }
    }
    return EXECUTED;
}

// AT+STAT
CommandStat::CommandStat(ModemState* state): Command("AT+STAT"), state(state) {}

ExecutionResult CommandStat::execute(char* buffer) {
    sprintf(buffer, "%d,%d\n", 
        state->network.transmit,
        state->network.receive);
    return EXECUTED;
}

// AT+WIFI
CommandWIFI::CommandWIFI(ModemConfig* config): AssignableCommand<char**>("AT+WIFI"), config(config) {}

ExecutionResult CommandWIFI::executeAssign(char* buffer) {
    if (argument_size == 0 || argument_size > 4) {
        return EXECUTED;
    }
    if (argument_size >= 1) {
        uint8_t WIFI_mode = atoi(argument[0]);
        if (WIFI_mode >= 0 && WIFI_mode <= 2) {
            config->network.WIFI_mode = static_cast<WiFiMode>(WIFI_mode);
        }
    }
    if (argument_size >= 2) {
        uint8_t channel = atoi(argument[1]);
        if (channel >= 1 && channel <= 13) {
            config->network.channel = channel;
        }
    }
    if (argument_size >= 3) {
        memset(config->network.ssid, 0, SSID_SIZE);
        if (strlen(argument[2]) > 0) {
            strcpy(config->network.ssid, argument[2]);     
        }
    }
    if (argument_size == 4) {
        memset(config->network.password, 0, PASSWORD_SIZE);
        if (strlen(argument[3]) >= 8) {
            strcpy(config->network.password, argument[3]);
        }
    }
    ok(buffer);
    return EXECUTED_UPDATED;
}

ExecutionResult CommandWIFI::executeQuery(char* buffer) {
    sprintf(buffer, "%d,%d", 
        config->network.WIFI_mode,
        config->network.channel);
    strcat(buffer, ",");
    if (strlen(config->network.ssid) > 0) {
        strcat(buffer, config->network.ssid);
    }
    strcat(buffer, ",");
    if (strlen(config->network.password) >= 8) {
        strcat(buffer, "*");
    }
    strcat(buffer, "\n");
    return EXECUTED;
}

// AT+IP get local IP-address
CommandIP::CommandIP(ModemState* state): Command("AT+IP"), state(state) {}

ExecutionResult CommandIP::execute(char* buffer) {
    sprintf(buffer, "%s\n", IPAddress(state->network.ip).toString().c_str());
    return EXECUTED;
}

CommandExecutor::CommandExecutor(Modem* modem) {
    ModemConfig* config = modem->persister->getConfig();
    ModemState* state = modem->state;
    commands[0] = new CommandAT();
    commands[1] = new CommandReset(modem->persister);
    commands[2] = new CommandConfig(config);
    commands[3] = new CommandAddr(config);
    commands[4] = new CommandFreq(config);
    commands[5] = new CommandBandwidth(config);
    commands[6] = new CommandSfactor(config);
    commands[7] = new CommandCodeRate(config);
    commands[8] = new CommandPower(config);
    commands[9] = new CommandScan(state);
    commands[10] = new CommandWIFI(config);
    commands[11] = new CommandRestart();
    commands[12] = new CommandStat(state);
    commands[13] = new CommandIP(state);
}

CommandExecutor::~CommandExecutor() {
    for (int i = 0; i < COMMANDS_COUNT; i++) {
        delete commands[i];
    }
}

ExecutionResult CommandExecutor::execute(char* input, size_t len, char* buffer) {
    if ((input[0] == 'A') && (input[1] == 'T')) {
        for (int i = 0; i < COMMANDS_COUNT; i++) {
            if (commands[i]->parse(input, len)) {
                return commands[i]->execute(buffer);
            }
        }
    }
    return NOT_EXECUTED;
}