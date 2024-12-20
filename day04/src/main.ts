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

  // I'm lazy so we don't generalize for any length strings
  // Though rather trivial to step (readLength - 1) / 2 steps in each direction from the center
  // to get the orthogonal string for any length
  function getOrthogonal({ x, y }: Position, [dx, dy]: Direction): number[] {
    const { x: cx, y: cy }: Position = { x: x + dx, y: y + dy };
    const chars = [
      at(cx + dx * -1, cy + dy),
      at(cx, cy),
      at(cx + dx, cy + dy * -1),
    ];

    return chars.filter((c) => c != undefined);
  }

  return { at, charAt, forEach, readDirection, getOrthogonal, height, width };
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

  // Part 2

  const crossTarget = TARGET_WORD.substring(1);
  const foundCrosses: Position[] = [];

  grid.forEach((char, pos) => {
    if (crossTarget.charCodeAt(0) == char) {
    SEARCH_DIRECTIONS.filter(([dx, dy]) => dx*dy != 0).forEach((dir) => {
      const diagonalWord = grid.readDirection(pos, dir, crossTarget.length)
      if (String.fromCharCode(...diagonalWord) == crossTarget) {
        const orthoWord = String.fromCharCode(...grid.getOrthogonal(pos, dir))
        if (orthoWord == crossTarget || orthoWord.split('').reverse().join('') == crossTarget) {
          foundCrosses.push(pos)
        }
      }
    })
    }
  });

  // Any valid cross will be found twice
  console.log(`Number of crosses: ${foundCrosses.length / 2}`)
  

})();
