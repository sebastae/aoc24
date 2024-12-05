use std::{cmp::Ordering, str::FromStr, usize};

const MIN_DIFF: u64 = 1;
const MAX_DIFF: u64 = 3;

type ReportErr = String;

type Level = i64;

struct Report(Vec<Level>);

impl FromStr for Report {
    type Err = ReportErr;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let levels = s
            .split_whitespace()
            .map(|level_str| {
                level_str
                    .parse::<Level>()
                    .map_err(|e| format!("parse level error for \"{level_str}\": {e}"))
            })
            .collect::<Result<Vec<Level>, Self::Err>>()?;

        Ok(Report(levels))
    }
}

#[derive(PartialEq)]
enum Direction {
    Increasing,
    Decreasing,
    None,
    Both,
}

impl Report {
    fn get_dir(&self) -> Direction {
        self.0
            .windows(2)
            .map(|window| match window[0].cmp(&window[1]) {
                Ordering::Equal => Direction::None,
                Ordering::Less => Direction::Increasing,
                Ordering::Greater => Direction::Decreasing,
            })
            .reduce(|prev, dir| if prev == dir { dir } else { Direction::Both })
            .unwrap()
    }

    fn is_safe(&self) -> bool {
        let safe_diff = !self
            .0
            .windows(2)
            .any(|window| match window[0].abs_diff(window[1]) {
                MIN_DIFF..=MAX_DIFF => false,
                _ => true,
            });

        let same_dir = match self.get_dir() {
            Direction::Increasing | Direction::Decreasing => true,
            _ => false,
        };

        safe_diff && same_dir
    }

    fn except(&self, index: usize) -> Report {
        Report(
            self.0
                .iter()
                .enumerate()
                .filter_map(|(i, val)| if i != index { Some(val.clone()) } else { None })
                .collect(),
        )
    }
}

fn count_safe_reports(input: &str) -> usize {
    input
        .lines()
        .map_while(|line| line.parse::<Report>().ok())
        .filter(|report| report.is_safe())
        .count()
}

fn count_dampened_safe_reports(input: &str) -> usize {
    let (safe, not_safe): (Vec<Report>, Vec<Report>) = input
        .lines()
        .map_while(|line| line.parse::<Report>().ok())
        .partition(|report| report.is_safe());

    let num_safe_with_dampening = not_safe.iter().filter(|report| {
        for i in 0..report.0.len() {
            if report.except(i).is_safe() {
                return true;
            }
        }

        false
    });

    safe.len() + num_safe_with_dampening.count()
}

fn main() {
    let input = std::fs::read_to_string("./src/input.txt").expect("read file");
    let safe_reports = count_safe_reports(input.as_str());

    println!("Part 1 - Safe reports: {}", safe_reports);

    let safe_with_dampening = count_dampened_safe_reports(input.as_str());
    println!("Part 2 - Safe with dampening: {}", safe_with_dampening);
}

#[cfg(test)]
mod test {
    use crate::*;

    const EXAMPLE_INPUT: &str = "7 6 4 2 1\n1 2 7 8 9\n9 7 6 2 1\n1 3 2 4 5\n8 6 4 4 1\n1 3 6 7 9";

    #[test]
    fn it_solves_example_part_1() {
        let safe_reports = count_safe_reports(EXAMPLE_INPUT);
        assert_eq!(safe_reports, 2);
    }

    #[test]
    fn it_solves_example_part_2(){
        let safe_reports_with_dampening = count_dampened_safe_reports(EXAMPLE_INPUT);
        assert_eq!(safe_reports_with_dampening, 4);
    }
}
