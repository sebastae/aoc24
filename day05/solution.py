import sys
import math
from typing import Iterable

RuleType = tuple[int, int]
UpdateType = list[int]
DisallowMap = dict[int, set[int]]


def parse_input(filename: str) -> tuple[list[RuleType], list[UpdateType]]:
    rules: list[RuleType] = []
    updates: list[UpdateType] = []

    reading_rules = True

    with open(filename, "r") as f:
        for line in f.readlines():
            line = line.strip()
            if line == "":
                reading_rules = False
                continue

            if reading_rules:
                lhs, rhs = map(lambda x: int(x), line.split("|"))
                rules.append((lhs, rhs))
            else:
                updates.append(list(map(lambda x: int(x), line.split(","))))
    return (rules, updates)


"""
Given a list of rules a -> b where a must precede b, build a map of sets b -> A such that A contains
every x where x -> b
"""


def build_disallow_sets(rules: list[RuleType]) -> DisallowMap:
    disallowed = dict[int, set[int]]()
    for rule in rules:
        pre, val = rule
        if val not in disallowed:
            disallowed[val] = set()
        disallowed[val].add(pre)
    return disallowed


"""
When reading a value from an update, check if it is in the set of values that invalidate the update.
If the update is still valid, add the values that must precede it to the set.
"""


def make_update_validator(rulemap: DisallowMap):
    def validator(update: UpdateType) -> bool:
        disallowed_values = set()
        for value in update:
            if value in disallowed_values:
                return False
            if value in rulemap:
                disallowed_values = disallowed_values.union(rulemap[value])
        return True

    return validator


class Node:
    def __init__(self, value: int):
        self.value = value
        self.incoming = set[Node]()
        self.outgoing = set[Node]()

    @staticmethod
    def from_rules(rules: Iterable[RuleType]):
        nodes = dict[int, Node]()
        for rule in rules:
            if rule[0] not in nodes:
                nodes[rule[0]] = Node(rule[0])
            if rule[1] not in nodes:
                nodes[rule[1]] = Node(rule[1])

            rhs_node = nodes[rule[0]]
            lhs_node = nodes[rule[1]]

            rhs_node.outgoing.add(lhs_node)
            lhs_node.incoming.add(rhs_node)

        return list(nodes.values())


"""
Sort values topologically using Kahn's algorithm
"""


def topological_sort(rules: Iterable[RuleType]) -> list[int]:
    nodes = Node.from_rules(rules)
    start_nodes = list(filter(lambda node: len(node.incoming) == 0, nodes))
    out_list = list[int]()

    while len(start_nodes) > 0:
        node = start_nodes.pop()
        out_list.append(node.value)

        for outgoing in node.outgoing:
            outgoing.incoming.remove(node)

            if len(outgoing.incoming) == 0:
                start_nodes.append(outgoing)

    return out_list


def make_update_fixer(rules: Iterable[RuleType]):
    def fix_update(update: UpdateType) -> UpdateType:
        update_set = set(update)
        relevant_rules = list(
            filter(lambda rule: rule[0] in update_set and rule[1] in update_set, rules)
        )

        sorted_values = topological_sort(relevant_rules)
        return sorted_values

    return fix_update


def sum_centers(updates: Iterable[UpdateType]) -> int:
    return sum(map(lambda update: update[math.floor(len(update) / 2)], updates))


if __name__ == "__main__":
    rules, updates = parse_input(sys.argv[1])
    rulesets = build_disallow_sets(rules)

    # Part 1 - find sum of center values in valid updates
    is_update_valid = make_update_validator(rulesets)
    valid_updates = filter(is_update_valid, updates)
    print(f"Sum of valid centers: {sum_centers(valid_updates)}")

    # Part 2 - fix incorrect updates
    invalid_updates = filter(lambda update: not is_update_valid(update), updates)
    reorder_update = make_update_fixer(rules)
    fixed_updates = map(lambda update: reorder_update(update), invalid_updates)
    print(f"Sum of fixed centers: {sum_centers(fixed_updates)}")
