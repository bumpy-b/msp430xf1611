#ifndef MSPGCC_COMMANDLINE_H
#define MSPGCC_COMMANDLINE_H

#include <mspgcc/util.h>
#include <stdbool.h>

/**
 * Use the following macro to create separator entries command table.
 *
 * @param text          text that is displayed for the separator
 */
#define COMMANDLINE_SEP(text) {"", command_help, text}  //table entry generation

/**
 * Use the following macro to create entries in the command table.
 * the command function is prefixed with "command_".
 * The help text should be short (<60 characters).
 *
 * @param name          name of the command and it's function
 * @param hlp           short help string
 */
#define COMMANDLINE_ENTRY(name, hlp) {#name, command_##name, hlp}  //table entry generation

/** Definition of a command table entry. */
typedef struct
{
    const char *name;                   // command name
    int (*function)(int, char**);       // function to call
    const char *help;                   // one line of help text
} COMMANDLINE_TABLE;

/**
 * Return value of commandline_eval. This value should be returned by
 * successful commands.
 */
#define COMMANDLINE_SUCCESS 0

/**
 * Return value for commands: parameter(s) invalid of wrong type or unexpected.
 */
#define COMMANDLINE_ERROR_PARAMETER 1

/**
 * Special return value of commandline_eval. Returned if the command is not
 * found in the provided table.
 */
#define COMMANDLINE_NO_SUCH_COMMNAD -1

/**
 * Simple command line processor.
 *
 * The line passed in is splited up in a list of pointers to strings and a
 * count, just like "int argc" and "char *argv[]" known from the "main".
 *
 * The command tables last entry has to be all zeros.
 *
 * A help command is built-in. It prints the list of command and their help
 * texts.
 *
 * Leading whitespace as well as multiple spaces between the arguments is
 * striped away/ignored.
 *
 * Example:
 * @code
 *     static int command_hello(int argc, char *argv[]) {
 *         printf("hello world!\n");
 *     }
 *     
 *     static const COMMANDLINE_TABLE commandline_table[] =
 *     {
 *         COMMANDLINE_ENTRY(hello, "The usual thing"),
 *         {0} //sentinel, marks the end of the table
 *     }
 *     
 *     // A loop whis is reading and processing a line
 *     // could look like this:
 *     while (1) {
 *         int retval;
 *         printf("> ");        //show a prompt
 *         simple_readline(line, sizeof(line)-1);
 *         retval = commandline_eval(line, commandline_table);
 *         if (retval == COMMANDLINE_NO_SUCH_COMMNAD) {
 *             printf("Unknown command. Try \"help\".\n");
 *         } else if (retval) { //if command failed
 *             printf("-- retval: %d\n", retval);
 *         } //no output for successful commands
 *     }
 * @endcode
 *
 * @note The number of parameter that are processed is limited, defined by
 *       the implementation.
 * 
 * @param commandline           [in/out] the command line string.
 *                              Some whitespace characters are overwritten!
 * @param command_table         [in] table containing the supported commands.
 * @return      the return value of the called function, 0 on success. or
 *              ::COMMANDLINE_NO_SUCH_COMMNAD if the command was not found
 *              in the table.
 */
int commandline_eval(char *commandline, const COMMANDLINE_TABLE *command_table);

/**
 * Print the full help sting corresponding to the given command. Typically used
 * for usage info if a command is called the wrong way. For example:
 * @code
 * if (argc < 2) {
 *      commandline_usage(argv[0]);
 * }
 * @endcode
 *
 * @param  s            [in] the string that is converted
 * @return true if the command was found and the help printed, false otherwise
 */
bool commandline_usage(const char *s);

/**
 * Parse string and try to convert it to a number. hex, dec and octal is
 * suported.
 *
 * Internaly uses strtoul, but it saves code to use this function instead of
 * calling strtoul at different places, as the additional arguements and the
 * typecasting of the result uses some code.
 *
 * @param  s            [in] the string that is converted
 * @return the converted number. 0 if the number could not be decoded.
 */
unsigned int commandline_number(const char *s);

// the library provides a number of commands for the user
// they just have to be registered in the command_table.
// note that these functions generate english text outputs on stdout
// some are also dendant on functions from other libraries
// (e.g. flash from libmspgcc)

/**
 * Print a list of supported commands and their short help.
 * the comand line table must have the name "commandline_table"
 * and has to be a global variable.
 */
int command_help(int argc, char *argv[]);

/**
 * Memory write command. Supports writes to flash. There is no memory
 * protection implemented, any address can be written. This means that
 * the program itself can be destroyed, peripherals reprogrammed etc.
 * This function is not suitable for end user usage!
 * 
 * Parameters: address, new memory contents
 */
int command_write(int argc, char *argv[]);

/**
 * Erase a single flash segment. There is no memory protection implemented,
 * any address can be written. This means that the program itself can be
 * destroyed, peripherals reprogrammed etc.
 * This function is not suitable for end user usage!
 * 
 * Parameters: address
 */
int command_erase(int argc, char *argv[]);

/**
 * Dump a memory range. There is no memory protection implemented,
 * any address can be read.
 *
 * This function can only dump memory that supports byte access.
 * (The 16 bit peripherals can not be dumped correctly.)
 *
 * This function is probably not suitable for end user usage!
 * 
 * Parameters: address size(optional)
 */
int command_hexdump(int argc, char *argv[]);

/**
 * Reset the target.
 * This is implemented by generating an access violation on the watdchdog
 * control register (WDTCTL) which causes a POR.
 */
int command_reset(int argc, char *argv[]);

/**
 * Calculate an XOR checksum over the given memory.
 *
 * Parameters: address size(optional)
 */
int command_checksum(int argc, char *argv[]);

/**
 * Simple Tab completion. This function can be called when a command should be
 * completed. This function writes to the display using printf/putchar.
 * Call thins function when a tab was received, it will automaticaly scan
 * the commandline_table for matches and writes the result to the line buffer.
 * 
 * @param commandline_table     table with commands that can be completed
 * @param lineeditor            the command line, that is edited
 * @param prompt                the prompt is used after a list of command was printed
 */
void commandline_tabcomplete(
    const COMMANDLINE_TABLE *commandline_table,
    LINEEDITOR_STATE *lineeditor,
    const char *prompt
);
#endif //MSPGCC_COMMANDLINE_H
