typedef struct Tower Tower;

#define TOWER_COUNT 3

void
create_towers(struct Tower** towers, unsigned char);

void
solve_tower_of_hanoi(struct Tower*);
