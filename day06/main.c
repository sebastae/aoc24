#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_SIZE 256
#define OBSTACLE_FLAG (1 << 0)
#define VISITED_FLAG (1 << 1)
#define ROTATION_OFFSET 2
#define LOOPING_FLAG (1 << 6)
#define OBSTACLE_CHAR '#'
#define OPEN_CHAR '.'
#define GUARD_CHAR '^'

/*
 * Grid cell structure
 * +-----------+------------+------------+------------+-------------+
 * | unused  1 | looping  1 | rotated  4 | visited  1 | obstacle  1 |
 * +-----------+------------+------------+------------+-------------+
 */

typedef struct {
  int x;
  int y;
} vec_t;

const vec_t DIRECTIONS[] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

typedef struct {
  vec_t pos;
  uint32_t dir;
} guard_t;

typedef struct {
  vec_t dimensions;
  size_t size;
  guard_t guard;
  vec_t initial_guard_pos;
  char *data;
} grid_t;

typedef struct {
  size_t cap;
  size_t len;
  vec_t *data;
} vecarr_t;

int pos_within_grid(vec_t pos, grid_t *grid) {
  return pos.x >= 0 && pos.x <= (grid->dimensions.x - 1) && pos.y >= 0 &&
         pos.y <= (grid->dimensions.y - 1);
}

int guard_is_within_grid(grid_t *grid) { return pos_within_grid(grid->guard.pos, grid); }

size_t get_grid_index(grid_t *grid, vec_t pos) { return pos.x + grid->dimensions.y * pos.y; }

char get_grid_data(grid_t *grid, vec_t pos) {
  if (!pos_within_grid(pos, grid)) {
    return 0;
  }

  size_t index = get_grid_index(grid, pos);
  return grid->data[index];
}

char set_grid_data(grid_t *grid, vec_t pos, char data) {
  if (!pos_within_grid(pos, grid)) {
    return 0;
  }

  char prev = get_grid_data(grid, pos);
  grid->data[get_grid_index(grid, pos)] = data;

  return prev;
}

void update_grid_data(grid_t *grid, vec_t pos, char data) {
  if (!pos_within_grid(pos, grid)) {
    return;
  }

  grid->data[get_grid_index(grid, pos)] |= data;
}

int grid_cell_is_open(grid_t *grid, vec_t pos) {
  if (!pos_within_grid(pos, grid)) {
    return 1;
  }

  char cell = get_grid_data(grid, pos);

  return (cell & OBSTACLE_FLAG) == 0;
}

vec_t add_vec(vec_t a, vec_t b) {
  vec_t new_vec = {a.x + b.x, a.y + b.y};
  return new_vec;
}

int next_is_open(grid_t *grid) {
  return grid_cell_is_open(grid, add_vec(grid->guard.pos, DIRECTIONS[grid->guard.dir]));
}

char proceed_guard(grid_t *grid) {
  char val = 0;

  if (next_is_open(grid)) {
    grid->guard.pos = add_vec(grid->guard.pos, DIRECTIONS[grid->guard.dir]);
    val = get_grid_data(grid, grid->guard.pos);

    char rotation_flag = 1 << (grid->guard.dir + ROTATION_OFFSET);
    update_grid_data(grid, grid->guard.pos, VISITED_FLAG | rotation_flag);

    char has_visited_in_same_direction = VISITED_FLAG | rotation_flag;
    if ((val & has_visited_in_same_direction) == has_visited_in_same_direction &&
        guard_is_within_grid(grid)) {
      val |= LOOPING_FLAG;
    }
  } else {
    val = get_grid_data(grid, grid->guard.pos);
    grid->guard.dir = (grid->guard.dir + 1) % (sizeof(DIRECTIONS) / sizeof(DIRECTIONS[0]));
  }
  return val;
}

void reset_grid(grid_t *grid, vec_t initial_obstacle_pos) {
  for (size_t i = 0; i < grid->size; i++) {
    grid->data[i] &= OBSTACLE_FLAG;
  }

  if (pos_within_grid(initial_obstacle_pos, grid)) {
    set_grid_data(grid, initial_obstacle_pos, 0);
  }

  set_grid_data(grid, grid->initial_guard_pos, VISITED_FLAG);
  grid->guard.pos = grid->initial_guard_pos;
  grid->guard.dir = 0;
}

int obstacle_will_create_loop(grid_t *grid, vec_t obstacle) {
  update_grid_data(grid, obstacle, OBSTACLE_FLAG);
  while (guard_is_within_grid(grid)) {
    char cell_data = proceed_guard(grid);
    if ((cell_data & LOOPING_FLAG) == LOOPING_FLAG) {
      return 1;
    }
  }

  return 0;
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Too few arguments, expected 1: %s <filename>\n", argv[0]);
    return 1;
  }

  FILE *input_file = fopen(argv[1], "r");
  if (input_file == NULL) {
    fprintf(stderr, "Could not open file \"%s\"\n", argv[1]);
    return 1;
  }

  grid_t grid;

  fseek(input_file, 0L, SEEK_END);
  size_t file_size = ftell(input_file);
  rewind(input_file);

  char *data = malloc(file_size);
  if (data == NULL) {
    fprintf(stderr, "Could not allocate memory\n");
    fclose(input_file);
  }

  grid.data = data;
  grid.size = 0;
  grid.dimensions = (vec_t){0, 0};

  char buf[256];
  char w_buf[256];
  size_t read_bytes;
  size_t w_bytes = 0;
  vec_t cur_pos = {0, 0};
  int width = 0;

  while (!feof(input_file)) {
    read_bytes = fread(&buf, sizeof buf[0], BUF_SIZE, input_file);
    for (int i = 0; i < read_bytes; i++) {
      if (buf[i] == '\n') {
        cur_pos.y++;
        width = cur_pos.x;
        cur_pos.x = 0;
        continue;
      }

      switch (buf[i]) {
      case OPEN_CHAR:
        w_buf[w_bytes++] = 0;
        break;
      case OBSTACLE_CHAR:
        w_buf[w_bytes++] = OBSTACLE_FLAG;
        break;
      case GUARD_CHAR:
        grid.guard = (guard_t){cur_pos, 0};
        grid.initial_guard_pos = cur_pos;
        w_buf[w_bytes++] = VISITED_FLAG;
      }

      cur_pos.x++;
    }

    // Copy cleaned buffer into grid data
    memcpy(grid.data + grid.size, &w_buf, w_bytes);
    grid.size += w_bytes;
    w_bytes = 0;
  }

  grid.dimensions = (vec_t){width, cur_pos.y};

  // Cleanup read
  fclose(input_file);

  vecarr_t visited_positions = {
      .cap = grid.size, .len = 0, .data = malloc(grid.size * sizeof(vec_t))};

  if (visited_positions.data == NULL) {
    fprintf(stderr, "Error while allocating %zu bytes of memory", grid.size * sizeof(vec_t));
    return 1;
  }

  visited_positions.data[visited_positions.len++] = grid.initial_guard_pos;

  // Part 1 - Count number of unique visited grid cells
  while (guard_is_within_grid(&grid)) {
    char cell = proceed_guard(&grid);
    if ((cell & VISITED_FLAG) == 0 && guard_is_within_grid(&grid)) {

      visited_positions.data[visited_positions.len++] = grid.guard.pos;
    }

    if (cell & LOOPING_FLAG) {
      break;
    }
  }

  printf("Visited cells: %zu\n", visited_positions.len);

  // Part 2 - find how many positions will loop
  // Since the guard only will collide with positions in the original path, we
  // try to add an obstacle to each of the visited positions and see if they
  // will loop
  size_t looping_positions = 0;
  for (size_t i = 0; i < visited_positions.len; i++) {
    vec_t pos = visited_positions.data[i];
    if (obstacle_will_create_loop(&grid, pos)) {
      looping_positions++;
    }

    reset_grid(&grid, pos);
  }

  printf("Viable obstacle positions: %zu\n", looping_positions);

  free(grid.data);
  return 0;
}
