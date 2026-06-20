typedef struct Tower Tower;

#define TOWER_COUNT 3

unsigned char*
get_tower_stacks(struct Tower*);

unsigned char
get_current_tower_height(struct Tower*);

struct Tower**
create_towers(unsigned char);

void
solve_tower_of_hanoi(struct Tower**);
