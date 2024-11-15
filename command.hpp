#ifndef COMMAND_H
#define COMMAND_H

#include <vector>
#include <sstream>

/// <b>readline</b> string parsed as command to change program/transmission state
struct Command
{
    std::string name; ///< Actual command, such as <b>send</b> or <b>quit</b>
    std::vector<std::string> args; ///< Flags send by user with command. Flag must be predefined and have value string after
};

/// Splitting command got from <b>readline</b> to command <b>name</b>, <b>flags</b> and <b>value</b> string to each <b>flag</b>
/// \param line - <b>string</b> or <b>char*</b> which contains all user input to parse
/// \return Command object with splitted flags
Command parseCommand(const std::string &line)
{
    Command command; ///< Command object to store all parsed information. Will be <b>returned</b>
    std::istringstream inputStream(line); ///< Get stream from <b>line</b>
    std::string arg; ///< String for each flag

    inputStream >> command.name; ///< Parsing stream from <b>line</b> to <b>Command object</b>

    while (inputStream >> arg)
    {
        command.args.push_back(arg);
    }

    return command;
}

/// Check are there exact flag at parsed <b>Command</b>
/// \param args - All flag data from exact <b>Command</b>, i.e.: <b>command.args</b>
/// \param flag - Flag to check presence
/// \param value - String to store <b>Value string</b> after exact flag
/// \return <b>True</b> - If there are provided <b>flag</b> in provided <b>command</b>
/// \return <b>False</b> - There not such a flag
bool hasFlag(const std::vector<std::string> &args, const std::string &flag, std::string &value)
{
    for (std::size_t i = 0; i < args.size(); ++i)
    {
        if (args[i] == flag && i + 1 < args.size())
        {
            value = args[i + 1];
            return true;
        }
    }

    return false;
}

/// Check are there exact flag at parsed <b>Command</b>
/// \param args - All flag data from exact <b>Command</b>, i.e.: <b>command.args</b>
/// \param flag - Flag to check presence
/// \return <b>True</b> - If there are provided <b>flag</b> in provided <b>command</b>
/// \return <b>False</b> - There not such a flag
bool hasFlag(const std::vector<std::string> &args, const std::string &flag)
{
    for (std::size_t currentArg = 0; currentArg < args.size(); ++currentArg)
    {
        if (args[currentArg] == flag)
            return true;
    }

    return false;
}

#endif //COMMAND_H
