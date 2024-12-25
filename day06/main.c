#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_SIZE 256
#define OBSTACLE_FLAG (1 << 0)
#define VISITED_FLAG (1 << 1)
#define OBSTACLE_CHAR '#'
#define OPEN_CHAR '.'
#define GUARD_CHAR '^'

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
  char *data;
} grid_t;

int pos_within_grid(vec_t pos, grid_t *grid) {
  return pos.x >= 0 && pos.x <= (grid->dimensions.x - 1) && pos.y >= 0 &&
         pos.y <= (grid->dimensions.y - 1);
}

size_t get_grid_index(grid_t *grid, vec_t pos) {
  return pos.x + grid->dimensions.y * pos.y;
}

char get_grid_data(grid_t *grid, vec_t pos) {
  if (!pos_within_grid(pos, grid)) {
    return 0;
  }

  size_t index = get_grid_index(grid, pos);
  return grid->data[index];
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
  return grid_cell_is_open(
      grid, add_vec(grid->guard.pos, DIRECTIONS[grid->guard.dir]));
}

uint8_t proceed_guard(grid_t *grid) {
  uint8_t val = 0;

  if (next_is_open(grid)) {
    grid->guard.pos = add_vec(grid->guard.pos, DIRECTIONS[grid->guard.dir]);
    val = (get_grid_data(grid, grid->guard.pos) & VISITED_FLAG) == 0 &&
          (pos_within_grid(grid->guard.pos, grid));
    grid->data[get_grid_index(grid, grid->guard.pos)] |= VISITED_FLAG;

  } else {
    grid->guard.dir =
        (grid->guard.dir + 1) % (sizeof(DIRECTIONS) / sizeof(DIRECTIONS[0]));
  }
  return val;
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

  // Part 1 - Count number of unique visited grid cells
  int unique_visited = 1;
  while (pos_within_grid(grid.guard.pos, &grid)) {
    unique_visited += proceed_guard(&grid);
  }

  printf("Visited cells: %i\n", unique_visited);

  free(grid.data);
  return 0;
}
