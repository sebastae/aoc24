package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strconv"
	"strings"
)

type Equation struct {
	Target     int
	Components []int
}

type Operation func(a, b int) int

// Line format: <target>: <component> ... <component>
func ParseLine(line string) (Equation, error) {
	target_str, components_str, found := strings.Cut(line, ":")
	if !found {
		return Equation{}, fmt.Errorf("invalid line format: missing ':'")
	}

	target, err := strconv.ParseInt(target_str, 10, 64)
	if err != nil {
		return Equation{}, fmt.Errorf("invalid line format: invalid target string '%s'", target_str)
	}

	components_strs := strings.Split(components_str, " ")
	components := make([]int, 0)
	for _, c := range components_strs {

		if strings.Trim(c, " ") == "" {
			continue
		}

		val, err := strconv.ParseInt(c, 10, 64)
		if err != nil {
			return Equation{}, fmt.Errorf("invalid line format: invalid component format '%s'", c)
		}

		components = append(components, int(val))
	}

	return Equation{
		Target:     int(target),
		Components: components,
	}, nil

}

func ParseFile(file *os.File) ([]Equation, error) {
	lines := make([]Equation, 0)
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line, err := ParseLine(scanner.Text())
		if err != nil {
			return lines, fmt.Errorf("error parsing file: %s", err)
		}

		lines = append(lines, line)
	}

	if err := scanner.Err(); err != nil {
		return lines, fmt.Errorf("error scanning file: %s", err)
	}

	return lines, nil
}

func CalculateResultsLeft(numbers []int, operations []Operation) []int {
	results := make([]int, 0)
	if len(numbers) < 2 {
		results = append(results, numbers[0])
		return results
	}

	lhs, rhs := numbers[0], numbers[1]
	for _, op := range operations {
		results = append(results, op(lhs, rhs))
	}

	if len(numbers) == 2 {
		return results
	}

	rec_results := make([]int, 0)
	for _, result := range results {
		next_nums := make([]int, len(numbers)-1)
		copy(next_nums, numbers[1:])
		next_nums[0] = result
		rec_results = append(rec_results, CalculateResultsLeft(next_nums, operations)...)
	}

	return rec_results
}

func (eq *Equation) IsValid() bool {
	operations := []Operation{
		func(a, b int) int { return a + b },
		func(a, b int) int { return a * b },
	}
	results := CalculateResultsLeft(eq.Components, operations)
	for _, result := range results {
		if result == eq.Target {
			return true
		}
	}
	return false
}

func main() {

	if len(os.Args) < 2 {
		fmt.Printf("Error: missing filename, usage: %s <filename>\n", os.Args[0])
		os.Exit(1)
	}

	file, err := os.Open(os.Args[1])
	if err != nil {
		log.Fatal(fmt.Errorf("error opening file: %s", err))
	}
	defer file.Close()

	equations, err := ParseFile(file)
	if err != nil {
		log.Fatal(err)
	}

  calibration_result := 0
	for _, eq := range equations {
    if (eq.IsValid()) {
      calibration_result += eq.Target
    }
	}

  fmt.Printf("Total calibration result: %d\n", calibration_result)

}
