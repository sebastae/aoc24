import { promises as fs } from "fs";
import { argv } from "process";

type Direction = [number, number];
type Position = { x: number; y: number };

const SEARCH_DIRECTIONS: Direction[] = [];
for (let dx = -1; dx <= 1; dx++) {
  for (let dy = -1; dy <= 1; dy++) {
    if (!(dx == 0 && dy == 0)) {
      SEARCH_DIRECTIONS.push([dx, dy]);
    }
  }
}

const TARGET_WORD = "XMAS";
const NEWLINE = "\n".charCodeAt(0);

function makeCharacterGrid(buffer: Buffer) {
  const width = buffer.indexOf("\n");
  const height = Math.floor(buffer.length / (width + 1));

  const data = buffer.filter((char) => char != NEWLINE);

  function at(x: number, y: number) {
    if (x >= width || x < 0 || y >= height || y < 0) {
      return undefined;
    }

    return data.at(x + y * width);
  }

  function charAt(x: number, y: number) {
    const code = at(x, y);
    return code != undefined ? String.fromCharCode(code) : code;
  }

  function readDirection(
    position: Position,
    [dx, dy]: Direction,
    maxLength: number,
  ) {
    let seq: number[] = [];

    let x = position.x,
      y = position.y;

    while (seq.length < maxLength) {
      const char = at(x, y);
      if (char == undefined) {
        break;
      }

      seq.push(char);
      x += dx;
      y += dy;
    }

    return seq;
  }

  function forEach(
    callback: (value: number, position: Position, buf: Uint8Array) => void,
  ) {
    for (let x = 0; x < width; x++) {
      for (let y = 0; y < height; y++) {
        callback(at(x, y)!, { x, y }, data);
      }
    }
  }

  return { at, charAt, forEach, readDirection, height, width };
}

(async function () {
  const filename = argv[2];
  if (!filename) {
    console.error("No filename provided");
    return;
  }

  const rawData = await fs.readFile(filename);
  const grid = makeCharacterGrid(rawData);

  const words: [Position, Direction][] = [];

  // Search for the target word in each direction from the starting character
  grid.forEach((char, pos) => {
    if (String.fromCharCode(char) == TARGET_WORD[0]) {
      SEARCH_DIRECTIONS.forEach((direction) => {
        const found = String.fromCharCode(
          ...grid.readDirection(pos, direction, TARGET_WORD.length),
        );
        if (found == TARGET_WORD) {
          words.push([pos, direction]);
        }
      });
    }
  });

  console.log(`Number of "${TARGET_WORD}" occurrences: ${words.length}`);
})();
