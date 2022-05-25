#ifndef COMMANDS_H
#define COMMANDS_H
#define COMMANDS_COUNT 13
#include "config.h"
#include "state.h"

enum ExecutionResult { NOT_EXECUTED, EXECUTED, EXECUTED_UPDATED };

class Command {
    public:
        Command(const char* cmd);
        virtual ~Command() {}
        virtual bool parse(char* buffer, size_t len);
        virtual ExecutionResult execute(char* buffer) = 0;
    protected:
        const char* command;
        void ok(char* buffer);
        void error(const char* name, const char* valid, char* buffer);
        void reset();
};

template <typename T>
class AssignableCommand: public Command {
    public:
        AssignableCommand(const char* cmd);
        ~AssignableCommand();
        bool parse(char* buffer, size_t len);
        ExecutionResult execute(char* buffer) final;
    protected:
        bool is_assign = false;
        T argument;
        uint8_t argument_size;
        virtual ExecutionResult executeAssign(char* buffer) = 0;
        virtual ExecutionResult executeQuery(char* buffer) = 0;
        void returnValue(T value, char* buffer);
    private:
        void parseArgument(char* arg_cstr);
        void reset();
};

class CommandAT: public Command {
    public:
        CommandAT();
        ExecutionResult execute(char* buffer);
};

class CommandReset: public Command {
    public:
        CommandReset(ConfigPersister* persister);
        ExecutionResult execute(char* buffer);
    private:
        ConfigPersister* persister;
};

class CommandRestart: public Command {
    public:
        CommandRestart();
        ExecutionResult execute(char* buffer);
};

class CommandConfig: public Command {
    public:
        CommandConfig(ConfigPersister* persister);
        ExecutionResult execute(char* buffer);
    private:
        ConfigPersister* persister;
};

class CommandAddr: public Command {
    public:
        CommandAddr(ConfigPersister* persister);
        ExecutionResult execute(char* buffer);
    private:
        ConfigPersister* persister;
};

class CommandFreq: public AssignableCommand<float> {
    public:
        CommandFreq(ConfigPersister* persister);
        ExecutionResult executeAssign(char* buffer);
        ExecutionResult executeQuery(char* buffer);
    private:
        ConfigPersister* persister;
};

class CommandBandwidth: public AssignableCommand<float> {
    public:
        CommandBandwidth(ConfigPersister* persister);
        ExecutionResult executeAssign(char* buffer);
        ExecutionResult executeQuery(char* buffer);
    private:
        ConfigPersister* persister;
};

class CommandSfactor: public AssignableCommand<int> {
    public:
        CommandSfactor(ConfigPersister* persister);
        ExecutionResult executeAssign(char* buffer);
        ExecutionResult executeQuery(char* buffer);
    private:
        ConfigPersister* persister;
};

class CommandCodeRate: public AssignableCommand<int> {
    public:
        CommandCodeRate(ConfigPersister* persister);
        ExecutionResult executeAssign(char* buffer);
        ExecutionResult executeQuery(char* buffer);
    private:
        ConfigPersister* persister;
};

class CommandPower: public AssignableCommand<int> {
    public:
        CommandPower(ConfigPersister* persister);
        ExecutionResult executeAssign(char* buffer);
        ExecutionResult executeQuery(char* buffer);
    private:
        ConfigPersister* persister;
};

class CommandScan: public Command {
    public:
        CommandScan(ModemState* state);
        ExecutionResult execute(char* buffer);
    private:
        ModemState* state;
};

class CommandStat: public Command {
    public:
        CommandStat(ModemState* state);
        ExecutionResult execute(char* buffer);
    private:
        ModemState* state;
};

class CommandWIFI: public AssignableCommand<char**> {
    public:
        CommandWIFI(ConfigPersister* persister);
        ExecutionResult executeAssign(char* buffer);
        ExecutionResult executeQuery(char* buffer);
    private:
        ConfigPersister* persister;
};

class CommandCIFSR: public Command { // get local IP-address
    public:
        CommandCIFSR(ModemState* state);
        ExecutionResult execute(char* buffer);
    private:
        ModemState* state;
};

class CommandExecutor {
    public:
        CommandExecutor(ModemState* state, ConfigPersister* persister);
        ~CommandExecutor();
        ExecutionResult execute(char* input, size_t len, char* buffer);
    private:
        Command *commands[COMMANDS_COUNT];
};

#endif
