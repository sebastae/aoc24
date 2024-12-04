---@class BinHeap
---@field heap any[]
---@field compare fun(parent: any, child: any): number
---@field insert fun(self: BinHeap, element: any)
---@field pop fun(self: BinHeap): any
---@field new fun(self: BinHeap, compare: fun(parent: any, child: any): number): BinHeap

---@class BinHeap
local BinHeap = {}
---@param compare fun(parent: any, child: any): number
function BinHeap:new(compare)
  local heap = {
    heap = {},
    compare = compare
  }

  setmetatable(heap, self)
  self.__index = self
  return heap
end

function BinHeap:is_correct_order(parent, child)
  local p = self.heap[parent]
  local c = self.heap[child]


  if p ~= nil and c == nil then return true end
  local correct = self.compare(self.heap[parent], self.heap[child]) <= 0
  return correct
end

function BinHeap.ileft(index)
  return 2 * index
end

function BinHeap.iright(index)
  return 2 * index + 1
end

function BinHeap.iparent(index)
  return math.floor(index / 2)
end

function BinHeap:swap(a, b)
  local tmp = self.heap[a]
  self.heap[a] = self.heap[b]
  self.heap[b] = tmp
end

function BinHeap:heapify_up(index)
  -- We cannot heapify up from the root
  if index == 1 then return end
  local parent = self.iparent(index)
  if not self:is_correct_order(parent, index) then
    self:swap(parent, index)
    self:heapify_up(parent)
  else
    return
  end
end

function BinHeap:heapify_down(index)
  -- We're at the last element
  if index >= #self.heap then
    return
  end

  local left = self.ileft(index)
  local right = self.iright(index)

  if self:is_correct_order(index, left) and self:is_correct_order(index, right) then
    return
  else
    local swap_index = left
    if not self:is_correct_order(left, right) then
      swap_index = right
    end

    self:swap(index, swap_index)
    self:heapify_down(swap_index)
  end
end

function BinHeap:insert(element)
  table.insert(self.heap, element)
  self:heapify_up(#self.heap)
end

function BinHeap:pop()

  if #self.heap == 0 then
    return nil
  end

  self:swap(1, #self.heap)
  local element = table.remove(self.heap)
  self:heapify_down(1)
  return element
end


---@param list any[]
---@param compare fun(a: any, b: any): number
function BinHeap:from(list, compare)
  local heap = BinHeap:new(compare)
  for _,item in ipairs(list) do
    heap:insert(item)
  end
  return heap
end

return BinHeap
