/* ID Header:
 Student Name:
 Student ID:
 Submission Date:
 File:
*/

#include "vmsim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

void print_sim_opts_t(sim_opts_t *obj){
    printf("--- Simulation Options ---\n");
    printf("Mode:       %s\n", (obj->mode == MODE_BB) ? "MODE_BB" : "MODE_SEG");
    printf("Base:       %ld\n", obj->base);
    printf("Limit:      %ld\n", obj->limit);
    printf("Trace Path: %s\n", obj->trace_path);
    printf("--------------------------\n");
    }

// usage
static void usage(const char *prog) {
    fprintf(stderr,
        "Usage:\n"
        "  %s --mode=bb  --base=N --limit=N --trace=FILE \n"
        "  %s --mode=seg --config=FILE --trace=FILE \n",
        prog, prog);
}

// CLI
bool parse_args(int argc, char **argv, sim_opts_t *obj) {

    (void)argc;
    (void)argv;

    if (obj){
        memset(obj, 0, sizeof(*obj));
    }

    if (strcmp(strchr(argv[1], '=') + 1, "bb") == 0){

        obj->mode = MODE_BB;
        obj->base = atol(strchr(argv[2], '=') + 1);
        obj->limit = atol(strchr(argv[3], '=') + 1);
        obj->trace_path = (strchr(argv[4], '=') + 1);

        return true;

    }
    else if(strcmp(argv[1], "--mode=seg") == 0){

    }

    return false;
}

bool bb_validate_input(const sim_opts_t *obj, const char *line, int l_num){

    char op;
    char address[20]; // Buffer to hold the extracted string

    int matched = sscanf(line, " %c %s", &op, address);
    
    // input is malformed
    if (matched != 2){
        printf("trace: %s:%d: malformed: expected \"OP ADDR\"\n", obj->trace_path, l_num);
        return false;
    }

    // input is of wrong operation
    if (!(op == 'R' || op == 'W')) {
        printf("trace: %s:%d: malformed: op must be R/W, got \"%c\"\n", obj->trace_path, l_num, op);
        return false;
    }

    // input contains non digit in address
    for (int i = 0; address[i] != '\0'; i++) {
            if (!isdigit((unsigned char)address[i])) {
                printf("trace: %s:%d: bad address \"%s\" (not decimal)\n", obj->trace_path, l_num, address);
                return false;
            }
    }

    // all conditions are met return valid
    return true;
}

void bb_translate(const sim_opts_t *obj, stats_t *stat,const char *line){

    char op;
    int virt_address; 

    stat->accesses ++;

    sscanf(line, " %c %d", &op, &virt_address);

    if (0 <= virt_address && virt_address < obj->limit){

        int phys_address = obj->base + virt_address;

        stat->ok ++;

        printf("%c %d -> PA %d ; ok\n", op, virt_address, phys_address);

    }
    else{
        stat->faults_bounds ++;
        printf("%c %d -> fault: BOUNDS\n", op, virt_address);

    }
}

void print_stats(stats_t *stat){
    printf("== stats ==\naccesses=%d, ok=%d, faults.bounds=%d", stat->accesses, stat->ok, stat->faults_bounds);
}


int run_bb(const sim_opts_t *obj, stats_t *stats) {

    (void)obj;
    (void)stats;

    stats->accesses = 0;
    stats->ok = 0;
    stats->faults_bounds = 0;

    FILE *file = fopen(obj->trace_path, "r");
    
    //stops the program if the file doesn't exist
    if (file == NULL) {
        perror("Error opening file"); 
        return EXIT_FAILURE;
    }

    //input buffer
    char buffer[256];
    int line = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL) {


        if (buffer[0] == '#'){ //checks if line starts with #
            continue;
        }
        else if (buffer[0] == '\n' || buffer[0] == '\r'){ //checks if line starts with \n or \r
            continue;
        }
        else{ //else valid line
            
            if(bb_validate_input(obj ,buffer, line) == true){ 
                bb_translate(obj ,stats,buffer);
                // helper func
            }
        }
        line ++;
    }

    print_stats(stats);
        

    fclose(file);
    return 0;
}

//seg
int run_seg(const sim_opts_t *o, stats_t *st) {
    (void)o; (void)st;
    fprintf(stderr, "TODO: run_seg()\n");
    return 0;
}

//main()
int main(int argc, char **argv) {

    sim_opts_t opts;

    if (!parse_args(argc, argv, &opts)){
        //print_sim_opts_t(&opts);
        usage(argv[0]); return 1; 
    }

    stats_t st = (stats_t){0};

    if (opts.mode == MODE_BB){ 
        return run_bb(&opts, &st);
    } 
    else return run_seg(&opts, &st);
}
