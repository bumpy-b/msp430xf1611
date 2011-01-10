#ifndef MSPGCC_EVENTHANDLER_H
#define MSPGCC_EVENTHANDLER_H

// Bit definitions and function prototypes for the eventhandler
// http://mspgcc.sf.net
// chris <cliechti@gmx.net>


#ifndef _GNU_ASSEMBLER_

/**
 * Simple event handler.
 * 
 * Each event is a function and has a corresponding bit in the variable
 * eventreg. EVENT00 has bit 0x8000 which is the highest priority,
 * EVENT01~0x4000 etc. Up to 16 events are supported.
 * Each event function has to exit by itself (there is no task switching).
 * This causes a risk of priority inversion when a low priority event
 * handler consumes too much time.
 * For devices with human interaction, a maximum time limit for each event of
 * less than 30ms is recomended.
 * 
 * Each time am eventhandler exits, the eventhandler_bits are scanned from
 * left to right (starting at the higest prio). If no eventhandler_bits is
 * set, "eventhandler_idle" is called. Scanning for events is restarted when
 * the control returns from the idle function.
 * 
 * An event function does not need to save any registers as on each run
 * the eventhandler is reinitializing all its registers.
 *
 * If no events are pending, then "eventhandler_idle" is called. The library
 * provides a default implementation that enters the LPM0 powersaving mode.
 * A different function may be provided that e.g. toggles a LED to show the
 * CPU activity or enters a different lowpower mode etc. Together with a
 * timer/getTime() function it is possible to implement a load monitor that
 * records the CPU load.
 *
 * @note
 *      It is usual for the "eventhandler_idle" function to enter a lowpower
 *      mode of the CPU, thus starting events from interrupts also requires
 *      to wake it up. See below for more info and examples.
 *
 * @param eventreginit  Initial value of eventhandler_bits. These events are
 *                      handled right after the start of the eventhandler.
 */
void __attribute__((noreturn)) eventhandler(unsigned short eventreginit);

/**
 * The bits in this variable define which event functions are launched.
 */
extern volatile unsigned short eventhandler_bits;  //priority/activation bits of event

/**
 * This macro starts an eventhandler.
 *
 * If used in an interrupt, do not forget to use a interrupt handler with
 * "wakeup" attribute or call "_BIC_SR_IRQ(LPM4_bits);"
 *
 * @param eventbit      one of the EVENTxx_bits defines or an alias.
 */
#define EVENTHANDLER_LAUNCH(eventbit) eventhandler_bits |= (eventbit)

/**
 * Table with event functions. Events are set up using a table with
 * predefined name: "eventhandler_table" is has to be supplied by the user.
 * 
 * Event functions are "void f(void)". Any registers can be used and have not
 * to be saved.
 * (actualy the functions can be void f(unsigned short eventbit) with eventbit
 * beeing the bit that is associated with that event. This can be used if a
 * event wants to restart itself: "eventhandler_bits |= eventbit;")
 * 
 *     const EVENTHANDLER_TABLE eventhandler_table[] = {
 *         event_timer,
 *         event_serial,
 *         {0} //sentinel, marks end of list
 *     };
 * 
 * It is recomended to make defines for each of these events, so that events
 * are lauched by their name. This makes it easier to refactor code later and
 * keep them in sync with the "eventhandler_table".
 *
 *     #define EVENT_timer      EVENT00_bits
 *     #define EVENT_serial     EVENT01_bits
 *
 * Then to launch an event from an interrupt:
 *
 *     wakeup interrupt(WDT_VECTOR) intervallTimer(void) {
 *         EVENTHANDLER_LAUNCH(EVENT_timer);
 *     }
 */
typedef void (*EVENTHANDLER_TABLE)(void);



/**
 * This is a special eventhandler: it starts additional events, based on the
 * "event_timetable".
 * 
 * It has an internal clock variable. If the internal clock advanced more than
 * one tick since the last run of scheduler, then each step in between is
 * calculated. This is to ensure that no step is left out, even when the
 * scheduler is called late (because of other events that consume too much
 * CPU time).
 * 
 * The schduler table has to be provided by the user. It has to be a global
 * named scheduler_table.
 * Example:
 *
 *     const SCHEDULER_TABLE scheduler_table[] = {
 *         { modulo: 1, shift: 0, eventbits: EVENT_periodic},
 *         {0} //sentinel, marks the end of the table
 *     };
 */
void event_scheduler(void);

/**
 * Increment the internal clock of the scheduler.
 */
void scheduler_increment(void);

/**
 * Reset the internal clock of the scheduler.
 */
void scheduler_reset(void);

/**
 * Event table entries. See ::event_scheduler for more information.
 */
typedef struct {
    unsigned short modulo;              ///< divisor
    unsigned short shift;               ///< remainder
    unsigned short eventbits;           ///< these bits are passed to EVENTHANDLER_LAUNCH
} SCHEDULER_TABLE;

#else //_GNU_ASSEMBLER_
// assembler interface

/**
 * This macro starts an eventhandler, assembler version.
 *
 * If used in an interrupt, do not forget to use a interrupt handler that
 * disables LPM4_bits in SR."
 *
 * @param eventbit      One of the EVENTxx_bits defines or an alias.
 *                      Must be a constant.
 */
#define EVENTHANDLER_LAUNCH(eventbit)  bis.w #eventbit, eventhandler_bits

#endif //_GNU_ASSEMBLER_

// Bit masks for taskbit register
// It is recomended to define aliases and not using these directly
#define EVENT00_bits 0x8000
#define EVENT01_bits 0x4000
#define EVENT02_bits 0x2000
#define EVENT03_bits 0x1000
#define EVENT04_bits 0x0800
#define EVENT05_bits 0x0400
#define EVENT06_bits 0x0200
#define EVENT07_bits 0x0100
#define EVENT08_bits 0x0080
#define EVENT09_bits 0x0040
#define EVENT10_bits 0x0020
#define EVENT11_bits 0x0010
#define EVENT12_bits 0x0008
#define EVENT13_bits 0x0004
#define EVENT14_bits 0x0002
#define EVENT15_bits 0x0001


#endif //MSPGCC_EVENTHANDLER_H
