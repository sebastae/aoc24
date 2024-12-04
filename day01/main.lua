local BinHeap = require('heap')

local function file_exists(file)
  local f = io.open(file, "rb")
  if f then f:close() end
  return f ~= nil
end


local function parse_file(file)
  if not file_exists(file) then
    os.exit(1)
  end

  local left_list = {}
  local right_list = {}

  for line in io.lines(file) do
    local next = string.gmatch(line, "(%d+)")

    table.insert(left_list, tonumber(next()))
    table.insert(right_list, tonumber(next()))
  end

  return left_list, right_list
end

local function solve_file(file)
  local left_list, right_list = parse_file(file)

  print('\nSolution for "' .. file .. '"')
  -- Part 1

  local left_heap = BinHeap:from(left_list, function(a, b) return a - b end)
  local right_heap = BinHeap:from(right_list, function(a, b) return a - b end)

  local dist_sum = 0
  for left in function() return left_heap:pop() end do
    local right = right_heap:pop()
    dist_sum = dist_sum + math.abs(left - right)
  end

  print("Part 1 dist: " .. dist_sum)

  -- Part 2
  local dict = {}
  for _, v in ipairs(right_list) do
    dict[v] = (dict[v] or 0) + 1
  end

  local sim_sum = 0
  for _, v in ipairs(left_list) do
    sim_sum = sim_sum + v * (dict[v] or 0)
  end

  print("Part 2 similarity: " .. sim_sum)
end

solve_file('input.example.txt')
solve_file('input.txt')
