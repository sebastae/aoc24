import sys
import math

RuleType = tuple[int, int]
UpdateType = list[int]
DisallowMap = dict[int, set[int]]

def parse_input(filename: str) -> (list[RuleType], list[UpdateType]):
    rules: list[RuleType] = []
    updates: list[UpdateType] = []

    reading_rules = True

    with open(filename, 'r') as f:
        for line in f.readlines():
            line = line.strip()
            if line == '':
                reading_rules = False
                continue
            
            if reading_rules:
                l,r = map(lambda x: int(x), line.split('|'))
                rules.append((l,r))
            else:
                updates.append(list(map(lambda x: int(x), line.split(','))))
    return (rules, updates)

"""
Given a list of rules a -> b where a must precede b, build a map of sets b -> A such that A contains
every x where x -> b
"""
def build_disallow_sets(rules: list[RuleType]) -> DisallowMap:
    disallowed = dict[int, set[int]]()
    for rule in rules:
        pre, val = rule
        if not val in disallowed:
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

if __name__ == '__main__':
    rules, updates = parse_input(sys.argv[1])
    rulesets = build_disallow_sets(rules)
    is_update_valid = make_update_validator(rulesets)
    valid_updates = filter(is_update_valid, updates)

    # Part 1 - find sum of center values
    sum_of_centers = sum(map(lambda update: update[math.floor(len(update)/2)], valid_updates))
    print(f"Sum of valid centers: {sum_of_centers}")

