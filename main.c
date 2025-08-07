#include <dlfcn.h>
#include <pthread.h>

// main application implemintation
// entry point for constructing and running the pipeline system
/** responsible for:
    *  parsing input
    * loading plugins
    * wiring the pipeline together
    * managing execution
    * shutting everything down cleanly
 */

 /** plugins list(6):
  * logger: Logs all strings that pass through to standard output.
  * typewriter: Simulates a typewriter effect by printing each character with a 100ms
  *             delay (you can use the usleep function). Notice, this can cause a “traffic jam”.
  * uppercaser: Converts all alphabetic characters in the string to uppercase.
  * rotator: Moves every character in the string one position to the right. The last character wraps around to the front.
  * flipper: Reverses the order of characters in the string.
  * expander: Inserts a single white space between each character in the string
  */

  int main(int argc, char *argv[]){

  }