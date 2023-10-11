from sys import argv
from os import path
import csv
import matplotlib.pyplot as plt


def plot_csv(csvfile: str, fitness_plot: str, objective_plot: str, fitness_title: str, objective_title: str) -> None:
    with open(csvfile) as f:
        reader = csv.reader(f)
        next(reader)

        minFitness = []
        maxFitness = []
        avgFitness = []
        minObjective = []
        maxObjective = []
        avgObjective = []

        for row in reader:
            vals = [float(x) for x in row]
            minFitness.append(vals[0])
            maxFitness.append(vals[1])
            avgFitness.append(vals[2])
            minObjective.append(vals[3])
            maxObjective.append(vals[4])
            avgObjective.append(vals[5])

        fitnessFigure = plt.figure()
        fitnessFigure.suptitle(fitness_title)
        fitnessAxes = fitnessFigure.subplots()
        fitnessAxes.plot(minFitness, label="Min fitness", color="orangered")
        fitnessAxes.plot(maxFitness, label="Max fitness", color="green")
        fitnessAxes.plot(avgFitness, label="Average fitness", color="steelblue")
        fitnessAxes.legend()
        fitnessAxes.set_ylim([0, 100])
        fitnessAxes.set_xlabel("Generation")
        fitnessAxes.set_ylabel("Fitness (higher is better)")
        fitnessFigure.savefig(fitness_plot)
        plt.close()

        objectiveFigure = plt.figure()
        objectiveFigure.suptitle(objective_title)
        objectiveAxes = objectiveFigure.subplots()
        objectiveAxes.plot(minObjective, label="Min objective", color="green")
        objectiveAxes.plot(maxObjective, label="Max objective", color="orangered")
        objectiveAxes.plot(avgObjective, label="Average objective", color="steelblue")
        objectiveAxes.legend()
        objectiveAxes.set_xlabel("Generation")
        objectiveAxes.set_ylabel("Objective function value (lower is better)")
        objectiveFigure.savefig(objective_plot)
        plt.close()


if __name__ == "__main__":
    if len(argv) != 3:
        print("Usage: python3 plot.py <data directory> <output directory>")
        quit()

    data_dir = argv[1]
    out_dir = argv[2]

    summary_csv = path.join("./", data_dir, "stats-average.csv")
    summary_fitness = path.join("./", out_dir, "fitness-average.png")
    summary_objective = path.join("./", out_dir, "objective-average.png")
    plot_csv(summary_csv, summary_fitness, summary_objective,
             "Average Fitnesses (over 30 Trials)", "Average Objective Function (over 30 Trials)")

    for i in range(30):
        input_data = path.join("./", data_dir, f"stats-trial-{i}.csv")
        output_fitness = path.join("./", out_dir, f"fitness-trial-{i}.png")
        output_objective = path.join("./", out_dir, f"objective-trial-{i}.png")
        plot_csv(input_data, output_fitness, output_objective,
                 f"Fitnesses for Trial {i}", f"Objective Function for Trial {i}")
