/* 
 * LC-3 Virtual machine
 * For now, should build on Windows only
*/
#include "lc3_vm.h"
#include "win_tools.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

int main(int argc, char *argv[])
{
    // load arguments
    if (argc < 2)
    {
        // show usage
        printf("lc3 [image-file]\n");
        exit(2);
    }

    printf("loading image...");
    t_lc3_vm *vm = load_vm(argv[1]);

    // Windows setup
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    printf("starting!");
    run_vm(vm);
    
    // shutdown
    restore_input_buffering();
}